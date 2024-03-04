#pragma once

#include <memory>
#include "Shader/Shader.h"
#include "Graphics/Model.h"
#include "Character.h"
#include "State/Player/PlayerStateMachine.h"
#include "Enemy.h"
#include "Effect.h"
#include "Input/GamePad.h"

//プレイヤー
class Player : public Character
{
public:
	// 攻撃タイプ
	enum class AttackType
	{
		Hammer,
		Spear,
		Sword,

		None,
	};
	// 敵探知ステート
	enum class EnemySearch
	{
		None = -1,
		Find,
		Attack,

		Max,
	};
	// 入力
	enum class InputState : unsigned int
	{
		None = 0,
		Run = 1,
		Jump = GamePad::BTN_A,
		Hammer = GamePad::BTN_B,
		Sword = GamePad::BTN_X,
		Spear = GamePad::BTN_Y,
	};
	// ジャンプステート
	enum class JumpState
	{
		CanJump,
		CanDoubleJump,
		CannotJump
	};

	// 武器
	struct Arms
	{
		const char*		nodeName;	// 武器ノードの名前
		XMFLOAT3		position;	// 位置
		const XMVECTOR	rootOffset;	// 根本のオフセット
		const XMVECTOR	tipOffset;	// 先のオフセット
		float			radius;		// 半径
		bool			flag1;		// 攻撃タイプ1が攻撃中か
		bool			flag2;		// 攻撃タイプ2が攻撃中か
		bool			flag3;		// 攻撃タイプ3が攻撃中か
		bool			flagJump;	// ジャンプ攻撃が攻撃中か
	};

private:
	// キャラクターモデル
	std::unique_ptr<Model> model;
	// ステートマシン
	PlayerStateMachine* stateMachine = nullptr;

	// 敵毎の距離
	std::unordered_map<Enemy*, float> enemyDist;
	// 敵毎の敵探索ステート
	std::unordered_map<Enemy*, EnemySearch> enemySearch;

	// 現在の攻撃タイプ
	AttackType currentAttackType = AttackType::None;
	// 現在の連続攻撃回数
	int attackCount = 0;
	// 攻撃中の敵ナンバー
	int attackingEnemyNumber = -1;
	// 攻撃判定 同一の敵の連続判定
	bool isAttackJudge = true;

	// ジャンプ遷移状態
	JumpState jumpTrg = JumpState::CanJump;
	// ジャンプ攻撃が動くか
	bool isMoveAttack = false;

	// SwordAttack
	static const int MAX_POLYGON = 32;
	XMFLOAT3 trailPositions[2][MAX_POLYGON];
	XMFLOAT4 color = { 1, 1, 1, 1 };
	bool isAddVertex = true;

	// Effect
	std::unique_ptr<Effect> hitEffect;

	// ***************** const *****************

	// プレイヤーと敵の判定距離
	static inline const float playerVSenemyJudgeDist[(int)EnemySearch::Max] =
	{
		6.5f,	// Find
		2.5f,	// Attack
	};

protected:
	// 現在の敵探索ステート
	EnemySearch currentEnemySearch = EnemySearch::None;
	// 最も近い敵
	Enemy* nearestEnemy = nullptr;
	// 最も近い敵との距離
	float nearestDist = FLT_MAX;
	// 最も近い敵とのベクトル
	XMFLOAT3 nearestVec = {};

	// ***************** const *****************

	const float playerModelSize = 0.02f;
	const int playerMaxHealth = 100;

	const float moveSpeed = 5.0f;
	const float turnSpeed = XMConvertToRadians(720);

protected:
	// 更新
	void UpdateUtils(float elapsedTime);
	// 敵との距離更新
	void UpdateEnemyDistance(float elapsedTime);
	// ジャンプステート更新
	void UpdateJumpState(float elapsedTime);

	// 垂直速力更新
	void UpdateVerticalVelocity(float elapsedFrame) override;

	// プレイヤーとエネミーとの衝突処理
	void CollisionPlayerVsEnemies();

	// ダメージ受けた時に呼ばれる
	void OnDamaged() override;
	// 死亡した時に呼ばれる
	void OnDead() override;

	// 軌跡配列ズラし
	void ShiftTrailPositions();
	// 軌跡描画
	void RenderTrail();

	// HPバー描画
	void HPBarRender(const RenderContext& rc, Sprite* gauge, bool is1P);

	// デバッグプリミティブ描画
	void DrawDebugPrimitive();
	// デバッグ
	void DebugMenu();

	// 移動ベクトル
	virtual XMFLOAT3 GetMoveVec() const = 0;

public:
	Player();
	~Player() override;

	// 初期化
	void Init();

	// 更新
	virtual void Update(float elapsedTime) = 0;

	// 着地した時に呼ばれる
	void OnLanding(float elapsedTime) override;

	// 武器当たり判定位置の更新
	void UpdateArmPositions(Model* model, Arms& arm);
	// 攻撃中の水平加速度更新
	void HorizontalVelocityByAttack(bool plus, int velo, float elapsedTime);

	// 武器とエネミーの衝突処理
	void CollisionArmsVsEnemies(Arms arm);

	// 回復遷移確認処理
	bool IsRecoverTransition();

	// シャドウマップ用描画
	void ShadowRender(const RenderContext& rc, ShadowMap* shadowMap);
	// 描画
	void Render(const RenderContext& rc, ModelShader* shader);
	// 攻撃の軌跡描画
	void PrimitiveRender(const RenderContext& rc);
	// HPバー描画
	virtual void HPBarRender(const RenderContext& rc, Sprite* gauge) = 0;

	// 移動入力処理
	bool InputMove(float elapsedTime);
	// 攻撃入力処理
	bool InputAttackFromNoneAttack(float elapsedTime);
	// ジャンプ中の攻撃入力処理
	bool InputAttackFromJump(float elapsedTime);

	// ボタン判定(押下時)
	virtual bool InputButtonDown(InputState button) = 0;
	// ボタン判定(入力時)
	virtual bool InputButton(InputState button) = 0;
	// ボタン判定(押上時)
	virtual bool InputButtonUp(InputState button) = 0;

	// ***************** GETTER & SETTER *****************

	PlayerStateMachine* GetStateMachine() const { return stateMachine; }

	Model* GetModel() const { return model.get(); }

	Enemy* GetNearestEnemy() { return nearestEnemy; }
	EnemySearch GetEachEnemySearch(Enemy* enemy) { return enemySearch[enemy]; }
	float GetEachEnemyDist(Enemy* enemy) { return enemyDist[enemy]; }

	int GetAttackCount() const { return attackCount; }
	void SetAttackCount(int count) { attackCount = count; }
	void AddAttackCount() { attackCount++; }

	AttackType GetAttackType() const { return currentAttackType; }
	void SetAttackType(AttackType at) { currentAttackType = at; }

	EnemySearch GetEnemySearch() const { return currentEnemySearch; }

	bool GetAttackJudge() const { return isAttackJudge; }
	void SetAttackJadge(bool aj) { isAttackJudge = aj; }

	bool GetMoveAttack() const { return isMoveAttack; }
	void SetMoveAttack(bool ma) { isMoveAttack = ma; }
	
	XMFLOAT3 GetVelocity() const { return velocity; }
	void SetVelocity(float x = FLT_MAX, float y = FLT_MAX, float z = FLT_MAX)
	{
		if(x != FLT_MAX) velocity.x = x;
		if(y != FLT_MAX) velocity.y = y;
		if(z != FLT_MAX) velocity.z = z;
	}
	void AddVelocity(XMFLOAT3 velo)
	{
		velocity.x += velo.x;
		velocity.y += velo.y;
		velocity.z += velo.z;
	}

	Arms GetHammer() const { return Hammer; }
	Arms GetSpear() const { return Spear; }
	Arms GetSword() const { return Sword; }

protected:
	Arms Hammer
	{
		"Hammer",
		{0,0,0},
		{0,0,0},
		{0,35,0},
		0.5f,
		false,
		false,
		false,
		false,
	};
	Arms Spear
	{
		"Spear",
		{0,0,0},
		{0,0,0},
		{0,90,0},
		0.4f,
		false,
		false,
		false,
		false,
	};
	Arms Sword
	{
		"Sword",
		{0,0,0},
		{0,0,0},
		{0,30,0},
		0.4f,
		false,
		false,
		false,
		false,
	};

public:
	// ステート
	enum class State
	{
		Idle,
		IdleToRun,
		Run,
		RunToIdle,
		JumpStart,
		JumpLoop,
		JumpAir,
		JumpEnd,
		Damage,
		Dead,
		Recover,
		AttackHammer1,
		AttackHammer2,
		AttackHammerJump,
		AttackSpear1,
		AttackSpear2,
		AttackSpear3,
		AttackSpearJump,
		AttackSword1,
		AttackSword2,
		AttackSword3,
		AttackSwordJump,
		CliffGrab,

		None,
	};

	// アニメーション
	enum class Animation
	{
		None,
		CliffGrab,
		Damage,
		Death,
		AttackHammer1,
		AttackHammer2,
		AttackHammerJump,
		Idle,
		IdleToRun,
		JumpAir,
		JumpEnd,
		JumpLoop,
		JumpStart,
		Running,
		RunToIdle,
		AttackSpear1,
		AttackSpear2,
		AttackSpear3,
		AttackSpearJump,
		AttackSword1,
		AttackSword2,
		AttackSword3,
		AttackSwordJump,
	};
};
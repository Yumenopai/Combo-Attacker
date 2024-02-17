#pragma once

#include <memory>
#include "Shader/Shader.h"
#include "Graphics/Model.h"
#include "Character.h"
#include "State/Player/PlayerStateMachine.h"
#include "Enemy.h"
#include "Effect.h"


//プレイヤー
class Player : public Character
{
public:
	//ステート
	enum class AttackType
	{
		Hammer,
		Spear,
		Sword,

		None,
	};
	//ステート
	enum class EnemySearch
	{
		None = -1,
		Find,
		Attack,

		Max,
	};

	struct Arms
	{
		const char* nodeName;
		XMFLOAT3	position;
		const XMVECTOR	rootOffset;
		const XMVECTOR	tipOffset;
		float		radius;
		bool flag1;
		bool flag2;
		bool flag3;
		bool flagJump;
	};

protected:
	std::unique_ptr<Model> model;
	AttackType	Atype = AttackType::None;
	EnemySearch ESState = EnemySearch::None;

	PlayerStateMachine* stateMachine = nullptr;

	// VS-Enemy-Search
	std::map<Enemy*, EnemySearch> enemySearch;
	std::map<Enemy*, float> enemyDist;

	Enemy* nearestEnemy = nullptr;
	float nearestDist = FLT_MAX;
	XMFLOAT3 nearestVec = {};

	bool isAttackjudge = true;
	int attackingEnemyNumber = -1;

	//ジャンプ攻撃時に使用
	bool isMoveAttack = false;
	int attackCount = 0;
	const float playerVSenemyJudgeDist[(int)EnemySearch::Max] = { 6.5f, 2.5f, };

	// SwordAttack
	static const int MAX_POLYGON = 32;
	XMFLOAT3 trailPositions[2][MAX_POLYGON];
	XMFLOAT4 color = { 1, 1, 1, 1 };
	bool isAddVertex = true;

	float moveSpeed = 5.0f;
	float turnSpeed = XMConvertToRadians(720);

	bool isDamaged = false;
	bool isDead = false;
	float stateTimer = 0.0f;
	float DamageFlash = 0.1f;
	int flashCount = 0;

	float jumpSpeed = 17.5f;

	DirectX::XMFLOAT3 offset;

	std::unique_ptr<Effect> hitEffect;

public:
	Player();
	~Player() override;

	// 初期化
	void Init();

	//更新
	virtual void Update(float elapsedTime) = 0;
	virtual void UpdateJump(float elapsedTime) = 0;

	//描画
	virtual void ShadowRender(const RenderContext& rc, ShadowMap* shadowMap) = 0;
	//描画
	virtual void Render(const RenderContext& rc, ModelShader* shader) = 0;
	void PrimitiveRender(const RenderContext& rc);
	virtual void HPBarRender(const RenderContext& rc, Sprite* gauge) = 0;

	// ステートマシン取得 
	PlayerStateMachine* GetStateMachine() const { return stateMachine; }

	int GetAttackCount() const { return attackCount; }
	void SetAttackCount(int count) { attackCount = count; }
	void AddAttackCount() { attackCount++; }

	AttackType GetAttackType() const { return Atype; }
	void SetAttackType(AttackType at) { Atype = at; }

	EnemySearch GetESState() const { return ESState; }

	bool GetAttackJudge() const { return isAttackjudge; }
	void SetAttackJadge(bool aj) { isAttackjudge = aj; }

	bool GetMoveAttack() const { return isMoveAttack; }
	void SetMoveAttack(bool ma) { isMoveAttack = ma; }
	
	XMFLOAT3 GetVelocity() { return velocity; }
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

	Arms GetHammer() { return Hammer; }
	Arms GetSpear() { return Spear; }
	Arms GetSword() { return Sword; }

	// 攻撃処理
	bool InputAttackFromNoneAttack(float elapsedTime);
	bool InputAttackFromJump(float elapsedTime);

	//回復遷移確認処理
	bool IsRecoverTransition();
	
	// 入力処理
	virtual bool InputJumpButtonDown() = 0;
	virtual bool InputJumpButton() = 0;
	virtual bool InputJumpButtonUp() = 0;
	virtual bool InputHammerButton() = 0;
	virtual bool InputSwordButton() = 0;
	virtual bool InputSpearButton() = 0;

	//デバッグプリミティブ描画
	void DrawDebugPrimitive();


	Model* GetModel() const { return model.get(); }
	
	Enemy* GetNearestEnemy() { return nearestEnemy; }
	EnemySearch GetEachEnemySearchState(Enemy* enemy) { return enemySearch[enemy]; }
	float GetEachEnemyDist(Enemy* enemy) { return enemyDist[enemy]; }

	// 武器当たり判定位置の更新
	void UpdateArmPositions(Model* model, Arms& arm);
	void HorizontalVelocityByAttack(bool plus, int velo, float elapsedTime);

	// プレイヤーとエネミーとの衝突処理
	void CollisionPlayerVsEnemies();
	// 武器ととエネミーの衝突処理
	void CollisionArmsVsEnemies(Arms arm);

	// 移動入力処理
	virtual bool InputMove(float elapsedTime);

	//デバッグ
	void DebugMenu();

	//着地した時に呼ばれる
	void OnLanding(float elapsedTime) override;

	//死亡した時に呼ばれる
	void OnDead();

protected:
	//ダメージ受けた時に呼ばれる
	void OnDamaged();

	//垂直速力更新 オーバーライド
	void UpdateVerticalVelocity(float elapsedFrame);

	//軌跡
	void ShiftTrailPositions();
	void RenderTrail();

protected:
	enum class JumpState
	{
		CanJump,
		CanDoubleJump,
		CannotJump
	};
	JumpState jumpTrg = JumpState::CanJump;

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
	//ステート
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

	//アニメーション
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
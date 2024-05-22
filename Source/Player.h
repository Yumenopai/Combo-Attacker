#pragma once

#include <memory>
#include "Shader/Shader.h"
#include "Graphics/Model.h"
#include "Graphics/Graphics.h"
#include "Graphics/FontSprite.h"
#include "Character.h"
#include "State/Player/PlayerStateMachine.h"
#include "Effect.h"
#include "Input/GamePad.h"

#define SC_INT static_cast<int>
#define SC_AT static_cast<Player::AttackType>

class Enemy;

class Player : public Character
{
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

	// 攻撃タイプ
	enum class AttackType
	{
		None = -1,

		Sword = 0,
		Hammer,
		Spear,

		MaxCount
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
		Attack = GamePad::BTN_B,
		Player = GamePad::BTN_X,
		Buddy = GamePad::BTN_Y,
	};
	// ジャンプステート
	enum class JumpState
	{
		CanJump,
		CanDoubleJump,
		CannotJump
	};
	// エフェクト番号
	enum class EffectNumber
	{
		Hit,
		BlueVortex,
		GreenVortex,
		RedVortex,
		Recovery,

		MaxCount
	};
	// 通知メッセージ
	enum class MessageNotification
	{
		WeaponGet = 0,
		LevelUp,
		Attack,
		RanAway,
		Indifference,

		MaxCount
	};

	// 武器
	struct Arms
	{
		const char*		nodeName;	// 武器ノードの名前
		XMFLOAT3		position;	// 位置
		const XMVECTOR	rootOffset;	// 根本のオフセット
		const XMVECTOR	tipOffset;	// 先のオフセット
		const float		radius;		// 半径
		const int		damage;		// ダメージ量
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

	/// <summary>
	/// 所持武器の登録
	///		first:所持できる武器
	///		second : 所持しているか
	/// </summary>
	std::unordered_map<AttackType, bool> HaveArms;

	int currentLevel = 1;

	// 現在攻撃しているか
	bool isAttacking = false;
	// 現在の連続攻撃回数
	int attackCount = 0;
	// 攻撃中の敵ナンバー
	int attackingEnemyNumber = -1;
	// 攻撃判定 同一の敵の連続判定
	bool isAttackJudge = true;
	// スペシャル技
	bool enableSpecialAttack = false;

	// ジャンプ遷移状態
	JumpState jumpTrg = JumpState::CanJump;
	// ジャンプ攻撃が動くか
	bool isMoveAttack = false;

	// UI
	int messageNumber = -1; // -1は非表示
	float messageYTimer = 0.0f;

	// SwordAttack
	static const int MAX_POLYGON = 32;
	XMFLOAT3 trailPositions[2][MAX_POLYGON];
	XMFLOAT4 color = { 1, 1, 1, 1 };
	bool isAddVertex = true;

	// Effect
	Effect EffectArray[(int)EffectNumber::MaxCount] = 
	{
		 "Data/Effect/hit.efk",
		 "Data/Effect/blueVortex.efk",
		 "Data/Effect/greenVortex.efk",
		 "Data/Effect/redVortex.efk",
		 "Data/Effect/recovery.efk"
	};

	// ***************** const *****************

	// プレイヤーと敵の判定距離
	static inline const float playerVSenemyJudgeDist[(int)EnemySearch::Max] =
	{
		6.5f,	// Find
		2.5f,	// Attack
	};

	const float playerModelSize = 0.02f;
	const int playerMaxHealth = 100;

	const float moveSpeed = 8.0f;
	
	// UI
	const float hpGuageWidth = 700.0f;
	const float hpGuageHeight = 15.0f;
	const XMFLOAT4 hpGuageBack = { 0.3f, 0.3f, 0.3f, 0.8f };

protected:
	// 初期装備
	AttackType InitialArm;
	// 使用中の武器
	AttackType CurrentUseArm = AttackType::None;

	// 現在の敵探索ステート
	EnemySearch currentEnemySearch = EnemySearch::None;
	// 最も近い敵
	Enemy* nearestEnemy = nullptr;
	// 最も近い敵との距離
	float nearestDist = FLT_MAX;
	// 最も近い敵とのベクトル
	XMFLOAT3 nearestVec = {};
	// 二番目に近い敵との距離
	float secondDist = FLT_MAX;
	// 二番目に近い敵とのベクトル
	XMFLOAT3 secondDistEnemyVec = {};
	// 攻撃中の敵
	Enemy* currentAttackEnemy;
	// 与えた総ダメージ
	int allDamage = 0;

	// 回復遷移可能か
	bool enableRecoverTransition = false;

	// メッセージ
	bool enableShowMessage[(int)MessageNotification::MaxCount] = {};

	// ***************** const *****************
	
	Player* targetPlayer;
	std::string characterName;
	XMFLOAT4 nameColor;

	float turnSpeed;

	// UI //1P & AI
	float hpGuage_Y;
	XMFLOAT4 hpColorNormal;
	XMFLOAT4 hpColorWorning;

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

	// 回復遷移確認処理
	bool EnableRecoverTransition();

	// ダメージ受けた時に呼ばれる
	void OnDamaged() override;
	// 死亡した時に呼ばれる
	void OnDead() override;

	// 軌跡配列ズラし
	void ShiftTrailPositions();
	// 軌跡描画
	void RenderTrail() const;

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

	// 次の選択武器取得
	Player::AttackType GetNextArm();

	// 近距離攻撃時の角度矯正
	void ForceTurnByAttack(float elapsedTime);
	// 武器当たり判定位置の更新
	void UpdateArmPositions(Model* model, Arms& arm);
	// 攻撃中の水平加速度更新
	void HorizontalVelocityByAttack(bool plus, int velo, float elapsedTime);

	// 武器とエネミーの衝突処理
	void CollisionArmsVsEnemies(Arms arm);

	// シャドウマップ用描画
	void ShadowRender(const RenderContext& rc, ShadowMap* shadowMap);
	// 描画
	void Render(const RenderContext& rc, ModelShader* shader);
	// 攻撃の軌跡描画
	void PrimitiveRender(const RenderContext& rc);
	// HPバー描画
	void RenderHPBar(ID3D11DeviceContext* dc, Sprite* gauge, FontSprite* font) const;
	//キャラクター名前描画
	void RenderCharacterOverHead(const RenderContext& rc, FontSprite* font, Sprite* message);
	// 所持武器描画
	void RenderHaveArms(ID3D11DeviceContext* dc, Sprite* frame, Sprite* ArmSprite);

	// 移動入力処理
	bool InputMove(float elapsedTime);
	// 攻撃入力処理
	bool InputAttackFromNoneAttack();
	// ジャンプ中の攻撃入力処理
	bool InputAttackFromJump(float elapsedTime);
	// 武器変更処理
	virtual void InputChangeArm(AttackType arm = AttackType::None) = 0;
	// ターゲット回復処理
	virtual void InputRecover() = 0;
	// 武器を使用可能にする
	void AddHaveArm(AttackType arm = AttackType::None);

	// ボタン判定(押下時)
	virtual bool InputButtonDown(InputState button) = 0;
	// ボタン判定(入力時)
	virtual bool InputButton(InputState button) = 0;
	// ボタン判定(押上時)
	virtual bool InputButtonUp(InputState button) = 0;

	// 簡略化関数
	void ChangeState(State newState) {
		stateMachine->ChangeState(SC_INT(newState));
	}
	void PlayAnimation(Animation anim, bool loop) {
		model->PlayAnimation(SC_INT(anim), loop);
	}
	void PlayEffect(EffectNumber num, const XMFLOAT3& position, float scale = 1.0f) {
		EffectArray[SC_INT(num)].Play(position, scale);
	}

	// デバッグプリミティブ描画
	void DrawDebugPrimitive() const;
	// デバッグ
	void DebugMenu();

#pragma region GETTER & SETTER
	// ***************** GETTER & SETTER *****************

	PlayerStateMachine* GetStateMachine() const { return stateMachine; }

	Model* GetModel() const { return model.get(); }

	Player* GetTargetPlayer() const { return targetPlayer; }
	Enemy* GetCurrentAttackEnemy() const { return currentAttackEnemy; }
	void SetCurrentAttackEnemy(Enemy* enemy) { currentAttackEnemy = enemy; }

	Enemy* GetNearestEnemy() { return nearestEnemy; }
	EnemySearch GetEachEnemySearch(Enemy* enemy) { return enemySearch[enemy]; }
	float GetEachEnemyDist(Enemy* enemy) { return enemyDist[enemy]; }

	int GetAttackCount() const { return attackCount; }
	void SetAttackCount(int count) { attackCount = count; }
	void AddAttackCount() { attackCount++; }
	
	int GetAllDamage() const { return allDamage; }

	bool GetAttacking() const { return isAttacking; }
	void SetAttacking(bool _isAttacking) { isAttacking = _isAttacking; }

	int GetLevel() const { return currentLevel; }
	void SetLevel(int lv) { currentLevel = lv; }
	void AddLevel(int lv) { currentLevel += lv; SetEnableShowMessage(Player::MessageNotification::LevelUp, true); }

	bool GetHaveEachArm(AttackType arm) { return HaveArms[arm]; }
	int GetHaveArmCount() {
		int haveCount = 0;
		for (int i = 0; i < HaveArms.size(); i++)
		{
			if (HaveArms[SC_AT(i)]) haveCount++;
		}
		return haveCount;
	}

	void SetEnableShowMessage(MessageNotification number, bool isShow) { enableShowMessage[static_cast<int>(number)] = isShow; }

	bool GetEnableRecoverTransition() const { return enableRecoverTransition; }

	AttackType GetCurrentUseArm() const { return CurrentUseArm; }
	bool GetHpWorning() const { return GetHealthRate() <= 20; }

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
#pragma endregion

protected:
	Arms Hammer
	{
		"Hammer",
		{0,0,0},
		{0,0,0},
		{0,35,0},
		0.5f,
		2,
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
		1,
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
		0.6f,
		1,
		false,
		false,
		false,
		false,
	};
};
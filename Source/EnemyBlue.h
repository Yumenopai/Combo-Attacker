#pragma once

#include <memory>
#include "Graphics/Model.h"
#include "Enemy.h"
#include "Player.h"

class EnemyBlue :public Enemy
{
public:
	EnemyBlue();
	~EnemyBlue() override;

	//更新処理
	void Update(float elapsedTime) override;

	//描画処理
	void ShadowRender(const RenderContext& rc, ShadowMap* shadowMap) override;
	void Render(const RenderContext& rc, ModelShader* shader) override;

	//縄張り設定
	void SetTerritory(const DirectX::XMFLOAT3& origin, float range);

	//デバッグ
	void DebugMenu();

protected:
	//ダメージ時に呼ばれる
	void OnDamaged() override;
	//死亡した時に呼ばれる
	void OnDead() override;

private:
	//ターゲット位置をランダム設定
	void SetRandomTargetPosition();

	//目標地点へ移動
	void MoveToTarget(float elapsedTime, float speedRate);

	//プレイヤー索敵
	bool SearchPlayer();

	//最近プレイヤーの登録
	void UpdateTargetPosition();
	
	//ノードとプレイヤーの衝突処理
	void CollisionNodeVsPlayer(const char* nodeName, float nodeRadius);

	// 最近プレイヤーのステート取得
	Player::EnemySearch GetNearestPlayerES();

	//徘徊ステートへ遷移
	void TransitionWanderState();
	//徘徊ステート更新処理
	void UpdateWanderState(float elapsedTime);
	//待機ステートへ遷移
	void TransitionIdleState();
	//待機ステート更新処理
	void UpdateIdleState(float elapsedTime);
	//追跡ステートへ遷移
	void TransitionPursuitState();
	//追跡ステート更新処理
	void UpdatePursuitState(float elapsedTime);
	//攻撃ステートへ遷移
	void TransitionAttackState();
	//攻撃ステート更新処理
	void UpdateAttackState(float elapsedTime);
	//戦闘待機ステートへ遷移
	void TransitionIdleBattleState();
	//戦闘待機ステート更新処理
	void UpdateIdleBattleState(float elapsedTime);
	//ダメージステートへ遷移
	void TransitionGetHitState();
	//ダメージステート更新処理
	void UpdateGetHitState(float elapsedTime);
	//咆哮ステートへ遷移
	void TransitionScreamState();
	//咆哮ステート更新処理
	void UpdateScreamState(float elapsedTime);
	//翼攻撃ステートへ遷移
	void TransitionAttackClawState();
	//翼攻撃ステート更新処理
	void UpdateAttackClawState(float elapsedTime);
	//死亡ステートへ遷移
	void TransitionDieState();
	//死亡ステート更新処理
	void UpdateDieState(float elapsedTime);

private:
	//ステート
	enum class State
	{
		Wander,
		Idle,
		Pursuit,
		Attack,
		IdleBattle,
		GetHit,
		Scream,
		AttackClaw,
		Die,
	};

	//アニメーション
	enum Animation
	{
		Anim_Idle1,
		Anim_FlyForward,
		Anim_Attack01,
		Anim_TakeOff,
		Anim_Land,
		Anim_Scream,
		Anim_AttackClaw,
		Anim_AttackFlame,
		Anim_Defend,
		Anim_GetDamage,
		Anim_Sleep,
		Anim_Walk,
		Anim_Run,
		Anim_FlyAttackFlame,
		Anim_FlyGlide,
		Anim_Idle02,
		Anim_Die,
		Anim_FlyFloat,
	};

private:
	std::unique_ptr<Model> model;
	State	state = State::Wander;

	bool isHalfHP = false;
	bool isDead = false;

	DirectX::XMFLOAT3 territoryOrigin = { 0,0,0 };
	float	territoryRange = 10.0f;
	float	moveSpeed = 3.0f;
	float	turnSpeed = DirectX::XMConvertToRadians(360);
	float	searchRange = 10.0f;
	float	attackRange = 5.0f;

	std::string nearestPlayerStr = "";
	float minLen = 0.0f;
	float nowLen = 0.0f;
};
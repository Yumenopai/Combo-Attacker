#pragma once

#include <memory>
#include "Graphics/Model.h"
#include "Enemy.h"
#include "Player.h"

class EnemySlime :public Enemy
{
private:
	//アニメーション
	enum class Animation
	{
		IdleNormal,
		IdleBattle,
		Attack1,
		Attack2,
		WalkFWD,
		WalkBWD,
		WalkLeft,
		WalkRight,
		RunFWD,
		SenseSomthingST,
		SenseSomthingPRT,
		Taunt,
		Victory,
		GetHit,
		Dizzy,
		Die
	};

public:
	//ステート
	enum class State
	{
		Idle,
		Wander,
		Pursuit,
		Attack,
		IdleBattle,
		HitDamage
	};


protected:
	std::unique_ptr<Model> model;
	State	state = State::Wander;
	
	float effectOffset_Y = 0.8f;

	DirectX::XMFLOAT3 territoryOrigin = { 0,0,0 };
	float	territoryRange = 10.0f;
	float	moveSpeed = 3.0f;
	float	turnSpeed = DirectX::XMConvertToRadians(360);
	float	searchRange = 5.0f;
	float	attackRange = 3.0f;

	const char* eyeBallNodeName = "";

public:
	EnemySlime();
	~EnemySlime() override;

	//更新処理
	void Update(float elapsedTime) override;

	//描画処理
	void ShadowRender(const RenderContext& rc, ShadowMap* shadowMap) override;
	void Render(const RenderContext& rc, ModelShader* shader) override;

	//縄張り設定
	void SetTerritory(const DirectX::XMFLOAT3& origin, float range);
	//Getter
	const float GetEffectOffset_Y() override { return effectOffset_Y; }

protected:
	//ダメージ時に呼ばれる
	void OnDamaged() override;
	//死亡した時に呼ばれる
	void OnDead() override;

	//ターゲット位置をランダム設定
	void SetRandomTargetPosition();

	//ターゲット位置を設定
	void UpdateTargetPosition();

	Player::EnemySearch GetNearestPlayerES();

	//目標地点へ移動
	void MoveToTarget(float elapsedTime, float speedRate);
	
	//最近Playerへの回転
	void TurnToTarget(float elapsedTime, float speedRate);

	//プレイヤー索敵
	virtual bool SearchPlayer();

	//ノードとプレイヤーの衝突処理
	void CollisionNodeVsPlayer(const char* nodeName, float nodeRadius);

	//徘徊ステート更新処理
	void UpdateWanderState(float elapsedTime);
	//待機ステート更新処理
	void UpdateIdleState(float elapsedTime);
	//追跡ステート更新処理
	void UpdatePursuitState(float elapsedTime);
	//攻撃ステート更新処理
	void UpdateAttackState(float elapsedTime);
	//戦闘待機ステート更新処理
	void UpdateIdleBattleState(float elapsedTime);
	//ダメージステート更新処理
	void UpdateHitDamageState(float elapsedTime);

	// 各ステージごとの更新処理
	virtual void UpdateEachState(float elapsedTime);

	//ステート遷移
	void TransitionState(State nowState);
	virtual void TransitionPlayAnimation(State nowState);
};
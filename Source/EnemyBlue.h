#pragma once

#include <memory>
#include "Graphics/Model.h"
#include "Enemy.h"
#include "Player.h"

class EnemyBlue :public Enemy
{
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
	// エフェクト番号
	enum class EffectNumber
	{
		dead,

		MaxCount
	};

private:
	// Effect
	Effect EffectArray[(int)EffectNumber::MaxCount] =
	{
		 "Data/Effect/cyanBroken.efk",
	};


public:
	EnemyBlue();
	~EnemyBlue() override;

	//ステート遷移
	void TransitionState(State nowState);
	virtual void TransitionPlayAnimation(State nowState);
	//更新処理
	void Update(float elapsedTime) override;

	//描画処理
	void ShadowRender(const RenderContext& rc, ShadowMap* shadowMap) override;
	void Render(const RenderContext& rc, ModelShader* shader) override;

	//縄張り設定
	void SetTerritory(const DirectX::XMFLOAT3& origin, float range);
	//Getter
	const float GetEffectOffset_Y() override { return effectOffset_Y; }

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
	void UpdateGetHitState(float elapsedTime);
	//咆哮ステート更新処理
	void UpdateScreamState(float elapsedTime);
	//翼攻撃ステート更新処理
	void UpdateAttackClawState(float elapsedTime);
	//死亡ステート更新処理
	void UpdateDieState(float elapsedTime);

	// エフェクト再生
	void PlayEffect(EffectNumber num, const XMFLOAT3& position, float scale = 1.0f) {
		EffectArray[static_cast<int>(num)].Play(position, scale);
	}


private:
	//アニメーション
	enum class Animation
	{
		Idle1,
		FlyForward,
		Attack01,
		TakeOff,
		Land,
		Scream,
		AttackClaw,
		AttackFlame,
		Defend,
		GetDamage,
		Sleep,
		Walk,
		Run,
		FlyAttackFlame,
		FlyGlide,
		Idle02,
		Die,
		FlyFloat,
	};

private:
	std::unique_ptr<Model> model;
	State	state = State::Wander;

	float effectOffset_Y = 0.8f;

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
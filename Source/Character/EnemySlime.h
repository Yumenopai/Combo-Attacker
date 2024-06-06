#pragma once

#include <memory>
#include "Graphics/Model.h"
#include "Enemy.h"
#include "Constant/EnemyConst.h"
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
	// モデル
	std::unique_ptr<Model> model;
	// ステート
	State state = State::Wander;

	// 目玉ノードの名前
	const char* eye_ball_node_name = "";

public:
	EnemySlime();
	~EnemySlime() override;

	// 更新処理
	void Update(float elapsedTime) override;

	// シャドウマップ描画
	void ShadowRender(const RenderContext& rc, ShadowMap* shadowMap) override;
	// 描画
	void Render(const RenderContext& rc, ModelShader* shader) override;

	// Getter
	const float GetEffectOffset_Y() override { return slime_effect_offset_y; }

protected:
	// ダメージ時に呼ばれる
	void OnDamaged() override;
	// 死亡した時に呼ばれる
	void OnDead() override;

	// ターゲット位置を設定
	void UpdateTargetPosition();

	// Getter_最近プレイヤーの敵探査ステート
	Player::EnemySearch GetNearestPlayer_EnemySearch();

	// 目標地点へ移動
	void MoveToTarget(float elapsedTime, float speedRate = 1);
	
	// 最近Playerへの回転
	void TurnToTarget(float elapsedTime, float speedRate = 1);

	// プレイヤー索敵
	virtual bool SearchPlayer();

	// ノードとプレイヤーの衝突処理
	void CollisionNodeVsPlayer(const char* nodeName, float nodeRadius);

	// 徘徊ステート更新処理
	void UpdateWanderState(float elapsedTime);
	// 待機ステート更新処理
	void UpdateIdleState(float elapsedTime);
	// 追跡ステート更新処理
	void UpdatePursuitState(float elapsedTime);
	// 攻撃ステート更新処理
	void UpdateAttackState(float elapsedTime);
	// 戦闘待機ステート更新処理
	void UpdateIdleBattleState(float elapsedTime);
	// ダメージステート更新処理
	void UpdateHitDamageState(float elapsedTime);

	// 各ステージごとの更新処理
	virtual void UpdateEachState(float elapsedTime);

	// ステート遷移
	void TransitionState(State nowState);
	// アニメーション遷移
	virtual void TransitionPlayAnimation(State nowState);
};
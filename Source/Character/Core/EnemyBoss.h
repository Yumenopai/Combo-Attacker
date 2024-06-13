#pragma once

#include <memory>
#include "Constant/EnemyConst.h"
#include "Graphics/Model.h"
#include "Character/Core/Enemy.h"
#include "Character/Core/Player.h"

class EnemyBoss :public Enemy
{
private:
	// エフェクト番号
	enum class EffectNumber
	{
		dead,

		MaxCount
	};

protected:
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
protected:
	// モデル
	std::unique_ptr<Model> model;

private:
	// ステート
	State state = State::Wander;

	// HPが半分以下か
	bool isHalfHP = false;
	// 死亡判定か
	bool isDead = false;
	// 死亡からクリア表示までのタイマー
	int clearTimer = 0;

	// 縄張り対象位置
	DirectX::XMFLOAT3 territoryOrigin = { 0,0,0 };

	// ImGui用/最近プレイヤーの名前ストリング
	std::string nearestPlayerStr = "";
	// プレイヤーの最近距離
	float minLen = 0.0f;
	// 現在のターゲットの距離
	float nowLen = 0.0f;

	// Effect
	Effect EffectArray[(int)EffectNumber::MaxCount] =
	{
		 "Data/Effect/cyanBroken.efk",
	};

public:
	EnemyBoss();
	~EnemyBoss() override;

	//ステート遷移
	void TransitionState(State nowState);
	//更新処理
	void Update(float elapsedTime) override;

	//描画処理
	void ShadowRender(const RenderContext& rc, ShadowMap* shadowMap) override;
	void Render(const RenderContext& rc, ModelShader* shader) override;

	//Getter
	const float GetEffectOffset_Y() override { return dragon_effect_offset_y; }

	//デバッグ
	void DebugMenu();

protected:
	// 初期処理
	void Init();

	// アニメーション遷移
	virtual void TransitionPlayAnimation(State nowState) = 0;

	//ダメージ時に呼ばれる
	void OnDamaged() override;
	//死亡した時に呼ばれる
	void OnDead() override;

	//死亡ステート更新処理
	virtual void UpdateDieState(float elapsedTime);

private:
	//ターゲット位置をランダム設定
	void SetRandomTargetPosition();

	//目標地点へ移動
	void MoveToTarget(float elapsedTime, float speedRate = 1);

	//プレイヤー索敵
	bool SearchPlayer();

	//最近プレイヤーの登録
	void UpdateTargetPosition();
	
	//ノードとプレイヤーの衝突処理
	void CollisionNodeVsPlayer(const char* nodeName, float nodeRadius);

	// 最近プレイヤーのステート取得
	Player::EnemySearch GetNearestPlayer_EnemySearch();

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

	// エフェクト再生
	void PlayEffect(EffectNumber num, const XMFLOAT3& position, float scale = 1.0f) {
		EffectArray[static_cast<int>(num)].Play(position, scale);
	}
};
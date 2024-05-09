#pragma once

#include <memory>
#include "Graphics/Model.h"
#include "EnemySlime.h"
#include "Effect.h"

class EnemyTurtleShell :public EnemySlime
{
private:
	//アニメーション
	enum Animation
	{
		Anim_IdleNormal,
		Anim_IdleBattle,
		Anim_Attack1,
		Anim_Attack2,
		Anim_WalkFWD,
		Anim_Defend,
		Anim_DefendGetHit,
		Anim_WalkBWD,
		Anim_WalkLeft,
		Anim_WalkRight,
		Anim_RunFWD,
		Anim_SenseSomthingST,
		Anim_SenseSomthingPRT,
		Anim_Taunt,
		Anim_Victory,
		Anim_GetHit,
		Anim_Dizzy,
		Anim_Die
	};
	// エフェクト番号
	enum class EffectNumber
	{
		dead,

		MaxCount
	};

public:
	// Effect
	Effect EffectArray[(int)EffectNumber::MaxCount] =
	{
		 "Data/Effect/cyanBroken.efk",
	};

public:
	EnemyTurtleShell();

	//ダメージ時に呼ばれる
	void OnDamaged() override;
	//死亡した時に呼ばれる
	void OnDead() override;

	//プレイヤー索敵
	bool SearchPlayer();

	// 簡略化関数
	void PlayEffect(EffectNumber num, const XMFLOAT3& position, float scale = 1.0f) {
		EffectArray[static_cast<int>(num)].Play(position, scale);
	}

private:
	// 各ステージごとの更新処理
	void UpdateEachState(float elapsedTime);

	//追跡ステート更新処理
	void UpdatePursuitState(float elapsedTime);

	//遷移時のアニメーション再生
	void TransitionPlayAnimation(State nowState);
};
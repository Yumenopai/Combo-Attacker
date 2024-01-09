#pragma once

#include <memory>
#include "Graphics/Model.h"
#include "EnemySlime.h"

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

public:
	EnemyTurtleShell();

	void OnDamaged();

	//プレイヤー索敵
	bool SearchPlayer();

private:
	// 各ステージごとの更新処理
	void UpdateEachState(float elapsedTime);

	//遷移時のアニメーション再生
	void TransitionPlayAnimation(State nowState);
};
#pragma once

#include <memory>
#include "Core/EnemyBoss.h"
#include "Core/Player.h"
#include "Graphics/Model.h"

class EnemyDragon :public EnemyBoss
{
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

public:
	EnemyDragon();
	~EnemyDragon() override;

	//アニメーション遷移
	void TransitionPlayAnimation(State nowState) override;

protected:
	//死亡ステート更新処理
	void UpdateDieState(float elapsedTime) override;
};
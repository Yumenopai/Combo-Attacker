#pragma once
#include "Core/EnemyNormal.h"

class EnemySlime :public EnemyNormal
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
	EnemySlime();
	~EnemySlime() override;

protected:
	// アニメーション遷移
	void TransitionPlayAnimation(State nowState) override;
};
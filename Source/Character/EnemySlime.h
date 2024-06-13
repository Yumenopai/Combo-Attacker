#pragma once
#include "Core/EnemyNormal.h"

class EnemySlime :public EnemyNormal
{
private:
	//�A�j���[�V����
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
	// �A�j���[�V�����J��
	void TransitionPlayAnimation(State nowState) override;
};
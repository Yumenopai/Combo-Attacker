#pragma once

#include <memory>
#include "Core/EnemyBoss.h"
#include "Core/Player.h"
#include "Graphics/Model.h"

class EnemyDragon :public EnemyBoss
{
private:
	//�A�j���[�V����
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

	//�A�j���[�V�����J��
	void TransitionPlayAnimation(State nowState) override;

protected:
	//���S�X�e�[�g�X�V����
	void UpdateDieState(float elapsedTime) override;
};
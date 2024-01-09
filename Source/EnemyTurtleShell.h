#pragma once

#include <memory>
#include "Graphics/Model.h"
#include "EnemySlime.h"

class EnemyTurtleShell :public EnemySlime
{
private:
	//�A�j���[�V����
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

	//�v���C���[���G
	bool SearchPlayer();

private:
	// �e�X�e�[�W���Ƃ̍X�V����
	void UpdateEachState(float elapsedTime);

	//�J�ڎ��̃A�j���[�V�����Đ�
	void TransitionPlayAnimation(State nowState);
};
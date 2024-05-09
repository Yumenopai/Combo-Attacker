#pragma once

#include <memory>
#include "Graphics/Model.h"
#include "EnemySlime.h"
#include "Effect.h"

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
	// �G�t�F�N�g�ԍ�
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

	//�_���[�W���ɌĂ΂��
	void OnDamaged() override;
	//���S�������ɌĂ΂��
	void OnDead() override;

	//�v���C���[���G
	bool SearchPlayer();

	// �ȗ����֐�
	void PlayEffect(EffectNumber num, const XMFLOAT3& position, float scale = 1.0f) {
		EffectArray[static_cast<int>(num)].Play(position, scale);
	}

private:
	// �e�X�e�[�W���Ƃ̍X�V����
	void UpdateEachState(float elapsedTime);

	//�ǐՃX�e�[�g�X�V����
	void UpdatePursuitState(float elapsedTime);

	//�J�ڎ��̃A�j���[�V�����Đ�
	void TransitionPlayAnimation(State nowState);
};
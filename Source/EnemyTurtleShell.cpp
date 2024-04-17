#include "EnemyTurtleShell.h"
#include "Graphics/Graphics.h"
#include "PlayerManager.h"
#include "Collision.h"
#include "MathScript.h"

//�R���X�g���N�^
EnemyTurtleShell::EnemyTurtleShell()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	model = std::make_unique<Model>(device, "Data/Model/RPG_TurtleShell/TurtleShellPBR.fbx", 0.01f);

	angle.y = Math::RandomRange(-360, 360);
	//radius = 0.5f;
	height = 1.0f;
	//�ҋ@�X�e�[�g�֑J��
	TransitionState(State::Idle);
}

//�v���C���[���G
bool EnemyTurtleShell::SearchPlayer()
{
	XMVECTOR pPos = XMLoadFloat3(&targetPosition);
	XMVECTOR ePos = XMLoadFloat3(&position);
	float dist = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(pPos, ePos)));

	if (dist < searchRange * searchRange)
	{
		return true;
	}
	return false;
}

//�J�ڎ��A�j���[�V�����Đ�
void EnemyTurtleShell::TransitionPlayAnimation(State nowState)
{
	Animation anime = Anim_IdleNormal; //�A�j���[�V�����ݒ�
	bool animeLoop = true;

	switch (nowState)
	{
	case State::Idle:
		anime = Anim_IdleNormal;
		break;
	case State::Wander:
		anime = Anim_WalkFWD;
		break;
	case State::Pursuit:
		anime = Anim_RunFWD;
		break;
	case State::Attack:
		anime = Anim_Attack1;
		animeLoop = false;
		break;
	case State::IdleBattle:
		anime = Anim_IdleBattle;
		break;
	case State::HitDamage:
		anime = Anim_GetHit;
		animeLoop = false;
		break;
	}

	model->PlayAnimation(static_cast<int>(anime), animeLoop); //�A�j���[�V�����Đ�
}

// �e�X�e�[�W���Ƃ̍X�V����
void EnemyTurtleShell::UpdateEachState(float elapsedTime)
{
	switch (state)
	{
	case State::Wander:
		UpdateWanderState(elapsedTime);
		break;
	case State::Idle:
		UpdateIdleState(elapsedTime);
		break;
	case State::Pursuit:
		UpdatePursuitState(elapsedTime);
		break;
	case State::Attack:
		UpdateAttackState(elapsedTime);
		break;
	case State::IdleBattle:
		UpdateIdleBattleState(elapsedTime);
		break;
	case State::HitDamage:
		UpdateHitDamageState(elapsedTime);
		break;
	}
}
//�_���[�W���ɌĂ΂��
void EnemyTurtleShell::OnDamaged()
{
	//�_���[�W�X�e�[�g�֑J��
	TransitionState(State::HitDamage);
}
//���S�������ɌĂ΂��
void EnemyTurtleShell::OnDead()
{
	// ���S���G�t�F�N�g�Đ�
	PlayEffect(EffectNumber::dead, position, 0.6f);

	//���g��j��
	Destroy();

	// �A�C�e���Q�b�g
	// ���ݎ����Ă��Ȃ����탊�X�g�擾
	auto remainList = Player1P::Instance().GetHaventArm();
	// �c��o����
	int remainCount = static_cast<int>(remainList.size());

	if (remainCount == 0) {
		return;
	}
	else {
		// ����
		int num = rand() % remainCount;

		int i = 0;
		// �l�����镐����m�肷��
		for (const auto& arm : remainList)
		{
			if (i == num)
			{
				Player1P::Instance().AddHaveArm(arm);
				return;
			}
			i++;
		}
	}
}
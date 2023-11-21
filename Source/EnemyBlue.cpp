#include "EnemyBlue.h"
#include "Graphics/Graphics.h"
#include "Player1P.h"
#include "Collision.h"

//�R���X�g���N�^
EnemyBlue::EnemyBlue()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	model = std::make_unique<Model>(device, "Data/Model/Enemy/Blue.fbx", 0.001f);

	height = 1.0f;
	//�ҋ@�X�e�[�g�֑J��
	TransitionWanderState();
}

//�f�X�g���N�^
EnemyBlue::~EnemyBlue()
{
}

//�X�V����
void EnemyBlue::Update(float elapsedTime)
{
	//�X�e�[�g���̍X�V����
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
	}

	//���͏����X�V
	UpdateVelocity(elapsedTime);

	//���G���ԍX�V
	UpdateInvincibleTimer(elapsedTime);

	//�I�u�W�F�N�g�s����X�V
	UpdateTransform();

	//���f���A�j���[�V�����X�V
	model->UpdateAnimation(elapsedTime);

	//���f���s��X�V
	model->UpdateTransform(transform);
}

//�`�揈��
void EnemyBlue::ShadowRender(const RenderContext& rc, ShadowMap* shadowMap)
{
	shadowMap->Draw(rc, model.get());
}
void EnemyBlue::Render(const RenderContext& rc, ModelShader* shader)
{
	shader->Draw(rc, model.get());
}

//�꒣��ݒ�
void EnemyBlue::SetTerritory(const DirectX::XMFLOAT3& origin, float range)
{
	territoryOrigin = origin;
	territoryRange = range;
}

//���S�������ɌĂ΂��
void EnemyBlue::OnDead()
{
	//���g��j��
	Destroy();
}

//�^�[�Q�b�g�ʒu�������_���ݒ�
void EnemyBlue::SetRandomTargetPosition()
{
	float theta = 0;
	float range = 0;

	targetPosition.x = territoryOrigin.x + range * sinf(theta);
	targetPosition.y = territoryOrigin.y;
	targetPosition.z = territoryOrigin.z + range * cosf(theta);
}

//�ڕW�n�_�ֈړ�
void EnemyBlue::MoveToTarget(float elapsedTime, float speedRate)
{
	//�^�[�Q�b�g�����ւ̐i�s�x�N�g�����Z�o
	float vx = targetPosition.x - position.x;
	float vz = targetPosition.z - position.z;
	float dist = sqrtf(vx * vx + vz * vz);
	vx /= dist;
	vz /= dist;

	//�ړ�����
	Move(vx, vz, moveSpeed * speedRate);
	Turn(elapsedTime, vx, vz, turnSpeed * speedRate);
}

//�v���C���[���G
bool EnemyBlue::SearchPlayer()
{
	//�v���C���[�Ƃ̍��፷���l������3D�ł̋������������
	const DirectX::XMFLOAT3& playerPosition = Player1P::Instance().GetPosition();
	float vx = playerPosition.x - position.x;
	float vy = playerPosition.y - position.y;
	float vz = playerPosition.z - position.z;
	float dist = sqrtf(vx * vx + vy * vy + vz * vz);

	if (dist < searchRange)
	{
		float distXZ = sqrtf(vx * vx + vz * vz);
		//�P�ʃx�N�g����
		vx /= distXZ;
		vz /= distXZ;
		//�O���x�N�g��
		float frontX = sinf(angle.y);
		float frontZ = cosf(angle.y);
		//2�̃x�N�g�����ϒl�őO�㔻��
		float dot = (frontX * vx) + (frontZ * vz);
		if (dot > 0.0f)
		{
			return true;
		}
	}
	return false;
}

//�p�j�X�e�[�g�֑J��
void EnemyBlue::TransitionWanderState()
{
	state = State::Wander;

	//�ڕW�n�_�ݒ�
	SetRandomTargetPosition();

	//�����A�j���[�V�����Đ�
	//model->PlayAnimation(Anim_WalkFWD, true);
}

//�p�j�X�e�[�g�X�V����
void EnemyBlue::UpdateWanderState(float elapsedTime)
{
	//�ڕW�n�_�ֈړ�
	MoveToTarget(elapsedTime, 0.5f);

	//�v���C���[���G
	if (SearchPlayer())
	{
		//����������ǐՃX�e�[�g�֑J��
		TransitionPursuitState();
	}
}

//�ҋ@�X�e�[�g�֑J��
void EnemyBlue::TransitionIdleState()
{
	state = State::Idle;

	//�ҋ@�A�j���[�V�����Đ�
	//model->PlayAnimation(Anim_IdleNormal, true);
}

//�ҋ@�X�e�[�g�X�V����
void EnemyBlue::UpdateIdleState(float elapsedTime)
{
	//�v���C���[���G
	if (SearchPlayer())
	{
		//����������ǐՃX�e�[�g�֑J��
		TransitionPursuitState();
	}
}

//�ǐՃX�e�[�g�֑J��
void EnemyBlue::TransitionPursuitState()
{
	state = State::Pursuit;

	//�����A�j���[�V�����Đ�
	//model->PlayAnimation(Anim_RunFWD, true);
}

//�ǐՃX�e�[�g�X�V����
void EnemyBlue::UpdatePursuitState(float elapsedTime)
{
	//�ڕW�n�_���v���C���[�ʒu�ɐݒ�
	targetPosition = Player1P::Instance().GetPosition();

	//�ڕW�n�_�ֈړ�
	MoveToTarget(elapsedTime, 1.0f);

	//�v���C���[�̋߂Â��ƍU���X�e�[�g�֑J��
	float vx = targetPosition.x - position.x;
	float vy = targetPosition.y - position.y;
	float vz = targetPosition.z - position.z;
	float dist = sqrtf(vx * vx + vy * vy + vz * vz);
	if (dist < attackRange)
	{
		//�U���X�e�[�g�֑J��
		TransitionAttackState();
	}
	else if (!SearchPlayer())
	{
		TransitionWanderState();
	}
}

//�U���X�e�[�g�֑J��
void EnemyBlue::TransitionAttackState()
{
	state = State::Attack;

	//�U���A�j���[�V�����Đ�
	//model->PlayAnimation(Anim_Attack1, false);
}

//�U���X�e�[�g�X�V����
void EnemyBlue::UpdateAttackState(float elapsedTime)
{
	//�C�ӂ̃A�j���[�V�����Đ���Ԃł̂ݏՓ˔��菈��������
	float animationTime = model->GetCurrentAnimationSeconds();
	if (animationTime >= 0.1f && animationTime <= 0.35f)
	{
	}

	//�U���A�j���[�V�������I�������퓬�ҋ@�X�e�[�g�֑J��
	if (!model->IsPlayAnimation())
	{
		TransitionIdleBattleState();
	}
}

//�퓬�ҋ@�X�e�[�g�֑J��
void EnemyBlue::TransitionIdleBattleState()
{
	state = State::IdleBattle;

	//�퓬�ҋ@�A�j���[�V�����Đ�
	//model->PlayAnimation(Anim_IdleBattle, true);
}

//�퓬�ҋ@�X�e�[�g�X�V����
void EnemyBlue::UpdateIdleBattleState(float elapsedTime)
{
	//�ڕW�n�_���v���C���[�ʒu�ɐݒ�
	targetPosition = Player1P::Instance().GetPosition();

	//�v���C���[���U���͈͂ɂ����ꍇ�͍U���X�e�[�g�֑J��
	float vx = targetPosition.x - position.x;
	float vy = targetPosition.y - position.y;
	float vz = targetPosition.z - position.z;
	float dist = sqrtf(vx * vx + vy * vy + vz * vz);
	if (dist < attackRange)
	{
		//�U���X�e�[�g�֑J��
		TransitionAttackState();
	}
	else
	{
		//�p�j�X�e�[�g�֑J��
		TransitionWanderState();
	}

	MoveToTarget(elapsedTime, 0.0f);
}

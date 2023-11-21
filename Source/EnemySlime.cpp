#include "EnemySlime.h"
#include "Graphics/Graphics.h"
#include "Player1P.h"
#include "Collision.h"

//�R���X�g���N�^
EnemySlime::EnemySlime()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	model = std::make_unique<Model>(device, "Data/Model/RPG_Slime/SlimePBR.fbx", 0.01f);

	//radius = 0.5f;
	height = 1.0f;
	//�ҋ@�X�e�[�g�֑J��
	TransitionWanderState();
}

//�f�X�g���N�^
EnemySlime::~EnemySlime()
{
}

//�X�V����
void EnemySlime::Update(float elapsedTime) 
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
void EnemySlime::ShadowRender(const RenderContext& rc, ShadowMap* shadowMap)
{
	shadowMap->Draw(rc, model.get());
}
void EnemySlime::Render(const RenderContext& rc, ModelShader* shader)
{
	shader->Draw(rc, model.get());
}

//�꒣��ݒ�
void EnemySlime::SetTerritory(const DirectX::XMFLOAT3& origin, float range)
{
	territoryOrigin = origin;
	territoryRange = range;
}

//���S�������ɌĂ΂��
void EnemySlime::OnDead()
{
	//���g��j��
	Destroy();
}

//�w��͈͂̃����_���l���v�Z����
float RandomRange(float min, float max)
{
	return min + (max - min) * (rand() / static_cast<float>(RAND_MAX));
}

//�^�[�Q�b�g�ʒu�������_���ݒ�
void EnemySlime::SetRandomTargetPosition()
{
	float theta = RandomRange(0, DirectX::XM_2PI);
	float range = RandomRange(0, territoryRange);

	targetPosition.x = territoryOrigin.x + range * sinf(theta);
	targetPosition.y = territoryOrigin.y;
	targetPosition.z = territoryOrigin.z + range * cosf(theta);
}

//�ڕW�n�_�ֈړ�
void EnemySlime::MoveToTarget(float elapsedTime, float speedRate)
{
	//�^�[�Q�b�g�����ւ̐i�s�x�N�g�����Z�o
	float vx = targetPosition.x - position.x;
	float vz = targetPosition.z - position.z;
	float dist = sqrtf(vx * vx + vz * vz);
	vx /= dist;
	vz /= dist;

	//�ړ�����
	//Move(vx, vz, moveSpeed * speedRate);
	Turn(elapsedTime, vx, vz, turnSpeed * speedRate);
}

//�v���C���[���G
bool EnemySlime::SearchPlayer()
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

//�m�[�h�ƃv���C���[�̏Փˏ���
void EnemySlime::CollisionNodeVsPlayer(const char* nodeName, float nodeRadius)
{
	//�m�[�h�擾
	Model::Node* node = model->FindNode(nodeName);
	if (node != nullptr)
	{
		//�m�[�h�ʒu�擾
		XMFLOAT3 nodePosition;
		nodePosition.x = node->worldTransform._41;
		nodePosition.y = node->worldTransform._42;
		nodePosition.z = node->worldTransform._43;

		//�Փˏ���
		Player& player = Player1P::Instance();
		XMFLOAT3 outPosition;
		if (Collision::IntersectSphereVsCylinder(
			nodePosition, nodeRadius,
			player.GetPosition(), player.GetRadius(), player.GetHeight(),
			outPosition
		))
		{
			//�_���[�W��^����
			if (player.ApplyDamage(1, 0.5f))
			{
				if (player.ApplyDamage(1, 0.5f))
				{
					//�G�𐁂���΂��x�N�g�����Z�o
					XMFLOAT3 vec;
					vec.x = outPosition.x - nodePosition.x;
					vec.z = outPosition.z - nodePosition.z;
					float length = sqrtf(vec.x * vec.x + vec.z * vec.z);
					vec.x /= length;
					vec.z /= length;

					//XZ���ʂɐ�����΂��͂�������
					float power = 10.0f;
					vec.x *= power;
					vec.z *= power;
					//Y�����ɂ��͂�������
					vec.y = 5.0f;

					//������΂�
					player.AddImpulse(vec);
				}
			}
		}
	}
}

//�p�j�X�e�[�g�֑J��
void EnemySlime::TransitionWanderState()
{
	state = State::Wander;

	//�ڕW�n�_�ݒ�
	SetRandomTargetPosition();

	//�����A�j���[�V�����Đ�
	model->PlayAnimation(Anim_WalkFWD, true);
}

//�p�j�X�e�[�g�X�V����
void EnemySlime::UpdateWanderState(float elapsedTime)
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
void EnemySlime::TransitionIdleState()
{
	state = State::Idle;

	//�ҋ@�A�j���[�V�����Đ�
	model->PlayAnimation(Anim_IdleNormal, true);
}

//�ҋ@�X�e�[�g�X�V����
void EnemySlime::UpdateIdleState(float elapsedTime)
{
	//�v���C���[���G
	if (SearchPlayer())
	{
		//����������ǐՃX�e�[�g�֑J��
		TransitionPursuitState();
	}
}

//�ǐՃX�e�[�g�֑J��
void EnemySlime::TransitionPursuitState()
{
	state = State::Pursuit;

	//�����A�j���[�V�����Đ�
	model->PlayAnimation(Anim_RunFWD, true);
}

//�ǐՃX�e�[�g�X�V����
void EnemySlime::UpdatePursuitState(float elapsedTime)
{
	//�ڕW�n�_���v���C���[�ʒu�ɐݒ�
	targetPosition = Player1P::Instance().GetPosition();

	//�ڕW�n�_�ֈړ�
	MoveToTarget(elapsedTime,1.0f);

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
void EnemySlime::TransitionAttackState()
{
	state = State::Attack;

	//�U���A�j���[�V�����Đ�
	model->PlayAnimation(Anim_Attack1, false);
}

//�U���X�e�[�g�X�V����
void EnemySlime::UpdateAttackState(float elapsedTime)
{
	//�C�ӂ̃A�j���[�V�����Đ���Ԃł̂ݏՓ˔��菈��������
	float animationTime = model->GetCurrentAnimationSeconds();
	if (animationTime >= 0.1f && animationTime <= 0.35f)
	{
		//�ڋʃm�[�h�ƃv���C���[�̏Փˏ���
		CollisionNodeVsPlayer("EyeBall", 0.2f);
	}

	//�U���A�j���[�V�������I�������퓬�ҋ@�X�e�[�g�֑J��
	if (!model->IsPlayAnimation())
	{
		TransitionIdleBattleState();
	}
}

//�퓬�ҋ@�X�e�[�g�֑J��
void EnemySlime::TransitionIdleBattleState()
{
	state = State::IdleBattle;

	//�퓬�ҋ@�A�j���[�V�����Đ�
	model->PlayAnimation(Anim_IdleBattle, true);
}

//�퓬�ҋ@�X�e�[�g�X�V����
void EnemySlime::UpdateIdleBattleState(float elapsedTime)
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

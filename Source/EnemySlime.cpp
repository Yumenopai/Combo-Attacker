#include "EnemySlime.h"
#include "Graphics/Graphics.h"
#include "PlayerManager.h"
#include "Collision.h"
#include "MathScript.h"

//�R���X�g���N�^
EnemySlime::EnemySlime()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	model = std::make_unique<Model>(device, "Data/Model/RPG_Slime/SlimePBR.fbx", 0.01f);
	eyeBallNodeName = "EyeBall";

	angle.y = Math::RandomRange(-360, 360);
	//radius = 0.5f;
	height = 1.0f;
	health = maxHealth = 30;

	damage = 2;
	//�ҋ@�X�e�[�g�֑J��
	TransitionState(State::Idle);
}

//�f�X�g���N�^
EnemySlime::~EnemySlime()
{
}

//�X�V����
void EnemySlime::Update(float elapsedTime) 
{
	//�ŋ�Player��ݒ�
	UpdateTargetPosition();

	// �e�X�e�[�W���Ƃ̍X�V����
	UpdateEachState(elapsedTime);

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

// �e�X�e�[�W���Ƃ̍X�V����
void EnemySlime::UpdateEachState(float elapsedTime)
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

//�_���[�W���ɌĂ΂��
void EnemySlime::OnDamaged()
{
	//�_���[�W�X�e�[�g�֑J��
	TransitionState(State::HitDamage);
}
//���S�������ɌĂ΂��
void EnemySlime::OnDead()
{
	//���g��j��
	Destroy();
}

Player::EnemySearch EnemySlime::GetNearestPlayerES()
{
	Player::EnemySearch es = Player::EnemySearch::None;
	//es = Player1P::Instance().GetEachEnemySearch(this);

	for (Player* player : PlayerManager::Instance().players)
	{
		if (static_cast<int>(es) < static_cast<int>(player->GetEachEnemySearch(this)))
			es = player->GetEachEnemySearch(this);
	}
	return es;
}

//�^�[�Q�b�g�ʒu�������_���ݒ�
void EnemySlime::SetRandomTargetPosition()
{
	float theta = Math::RandomRange(0, DirectX::XM_2PI);
	float range = Math::RandomRange(0, territoryRange);

	targetPosition.x = territoryOrigin.x + range * sinf(theta);
	targetPosition.y = territoryOrigin.y;
	targetPosition.z = territoryOrigin.z + range * cosf(theta);
}

//�^�[�Q�b�g�ʒu��ݒ�
void EnemySlime::UpdateTargetPosition()
{
	//�v���C���[�Ƃ̍��፷���l������3D�ł̋������������
	float minLen = FLT_MAX;
	for (Player* player : PlayerManager::Instance().players)
	{
		float nowLen = player->GetEachEnemyDist(this);
		if (nowLen < minLen)
		{
			minLen = nowLen;
			targetPosition = player->GetPosition();
		}
	}
	//targetPosition = Player1P::Instance().GetPosition();
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
	Move(vx, vz, moveSpeed * speedRate);
	Turn(elapsedTime, vx, vz, turnSpeed * speedRate);
}

//�ŋ�Player�ւ̉�]
void EnemySlime::TurnToTarget(float elapsedTime, float speedRate)
{
	//�^�[�Q�b�g�����ւ̐i�s�x�N�g�����Z�o
	float vx = targetPosition.x - position.x;
	float vz = targetPosition.z - position.z;
	float dist = XMVectorGetX(XMVector3Length(XMVectorSubtract(
		XMLoadFloat3(&targetPosition), XMLoadFloat3(&position)
	)));

	vx /= dist;
	vz /= dist;

	//��]����
	Turn(elapsedTime, vx, vz, turnSpeed * speedRate);
}

//�v���C���[���G
bool EnemySlime::SearchPlayer()
{
	float vx = targetPosition.x - position.x;
	float vy = targetPosition.y - position.y;
	float vz = targetPosition.z - position.z;
	float dist = vx * vx + vy * vy + vz * vz;

	if (dist < attackRange * attackRange)//�K��������
	{
		return true;
	}

	if (dist < searchRange * searchRange)
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
	if (node == nullptr) return;

	//�m�[�h�ʒu�擾
	XMFLOAT3 nodePosition;
	nodePosition.x = node->worldTransform._41;
	nodePosition.y = node->worldTransform._42;
	nodePosition.z = node->worldTransform._43;

	//�Փˏ���
	for (Player* player : PlayerManager::Instance().players)
	{
		XMFLOAT3 outPosition;
		if (Collision::IntersectSphereVsCylinder(
			nodePosition, nodeRadius,
			player->GetPosition(), player->GetRadius(), player->GetHeight(),
			outPosition
		))
		{
			//�_���[�W��^����
			if (player->ApplyDamage(damage))
			{
				//�G�𐁂���΂��x�N�g�����Z�o
				XMFLOAT3 vec;
				vec.x = outPosition.x - nodePosition.x;
				vec.z = outPosition.z - nodePosition.z;
				float length = sqrtf(vec.x * vec.x + vec.z * vec.z);
				vec.x /= length;
				vec.z /= length;

				//XZ���ʂɐ�����΂��͂�������
				float power = 3.0f;
				vec.x *= power;
				vec.z *= power;
				//Y�����ɂ��͂�������
				vec.y = 4.0f;

				//������΂�
				player->AddImpulse(vec);
			}
		}
	}
}

/****************�X�V����****************/

//�p�j�X�e�[�g�X�V����
void EnemySlime::UpdateWanderState(float elapsedTime)
{
	//�ڕW�n�_�ֈړ�
	//MoveToTarget(elapsedTime, 0.5f);
	TurnToTarget(elapsedTime, 0.5f);

	//�v���C���[���G
	if (SearchPlayer())
	{
		//����������ǐՃX�e�[�g�֑J��
		TransitionState(State::Pursuit);
	}
}

//�ҋ@�X�e�[�g�X�V����
void EnemySlime::UpdateIdleState(float elapsedTime)
{
	//�v���C���[���G
	if (SearchPlayer())
	{
		//����������ǐՃX�e�[�g�֑J��
		TransitionState(State::Pursuit);
	}
}

//�ǐՃX�e�[�g�X�V����
void EnemySlime::UpdatePursuitState(float elapsedTime)
{
	//�ڕW�n�_�ֈړ�
	//MoveToTarget(elapsedTime,1.0f);
	//�ڕW�։�]
	TurnToTarget(elapsedTime, 1.0f);

	//�v���C���[���U���͈͂ɂ����ꍇ�͍U���X�e�[�g�֑J��
	Player::EnemySearch es = GetNearestPlayerES();

	if (es >= Player::EnemySearch::Attack)
	{
		//�U���X�e�[�g�֑J��
		TransitionState(State::Attack);
	}
	else if (!SearchPlayer())
	{
		TransitionState(State::Idle);
	}
}

//�U���X�e�[�g�X�V����
void EnemySlime::UpdateAttackState(float elapsedTime)
{
	//�ڕW�։�]
	TurnToTarget(elapsedTime, 1.0f);

	//�C�ӂ̃A�j���[�V�����Đ���Ԃł̂ݏՓ˔��菈��������
	float animationTime = model->GetCurrentAnimationSeconds();
	if (animationTime >= 0.1f && animationTime <= 0.35f)
	{
		//�ڋʃm�[�h�ƃv���C���[�̏Փˏ���
		CollisionNodeVsPlayer(eyeBallNodeName, 0.2f);
	}

	//�U���A�j���[�V�������I�������퓬�ҋ@�X�e�[�g�֑J��
	if (!model->IsPlayAnimation())
	{
		TransitionState(State::IdleBattle);
	}
}

//�퓬�ҋ@�X�e�[�g�X�V����
void EnemySlime::UpdateIdleBattleState(float elapsedTime)
{
	//�v���C���[���U���͈͂ɂ����ꍇ�͍U���X�e�[�g�֑J��
	Player::EnemySearch es = GetNearestPlayerES();

	if (es >= Player::EnemySearch::Attack)
	{
		//�U���X�e�[�g�֑J��
		TransitionState(State::Attack);
	}
	else
	{
		//�ҋ@�X�e�[�g�֑J��
		TransitionState(State::Idle);
	}
	
	TurnToTarget(elapsedTime, 0.0f);
}

//�_���[�W�X�e�[�g�X�V����
void EnemySlime::UpdateHitDamageState(float elapsedTime)
{
	if (model->IsPlayAnimation()) return;

	//�v���C���[���U���͈͂ɂ����ꍇ�͍U���X�e�[�g�֑J��
	Player::EnemySearch es = GetNearestPlayerES();

	if (es >= Player::EnemySearch::Attack)
	{
		//�U���X�e�[�g�֑J��
		TransitionState(State::IdleBattle);
	}
	else
	{
		//�ҋ@�X�e�[�g�֑J��
		TransitionState(State::Idle);
	}
}

//�X�e�[�g�J��
void EnemySlime::TransitionState(State nowState)
{
	state = nowState; //�X�e�[�g�ݒ�

	//switch (nowState)
	//{
	//case State::Idle:
	//	break;
	//case State::Wander:
	//	//�ڕW�n�_�ݒ�
	//	SetRandomTargetPosition();
	//	break;
	//case State::Pursuit:
	//	break;
	//case State::Attack:
	//	break;
	//case State::IdleBattle:
	//	break;
	//case State::HitDamage:
	//	break;
	//}

	TransitionPlayAnimation(nowState);
}

//�J�ڎ��A�j���[�V�����Đ�
void EnemySlime::TransitionPlayAnimation(State nowState)
{
	//�A�j���[�V�����ݒ�
	Animation anime = Animation::IdleNormal;
	bool animeLoop = true;

	switch (nowState)
	{
	case State::Idle:
		anime = Animation::IdleNormal;
		break;
	case State::Wander:
		anime = Animation::WalkFWD;
		break;
	case State::Pursuit:
		anime = Animation::RunFWD;
		break;
	case State::Attack:
		anime = Animation::Attack1;
		animeLoop = false;
		break;
	case State::IdleBattle:
		anime = Animation::IdleBattle;
		break;
	case State::HitDamage:
		anime = Animation::GetHit;
		animeLoop = false;
		break;
	}

	//�A�j���[�V�����Đ�
	model->PlayAnimation(static_cast<int>(anime), animeLoop);
}

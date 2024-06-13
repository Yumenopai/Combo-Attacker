#include "EnemyNormal.h"
#include "Constant/EnemyConst.h"
#include "Graphics/Graphics.h"
#include "Character/Manager/PlayerManager.h"
#include "Collision.h"
#include "MathScript.h"
#include "AnimationTimeStruct.h"

//�R���X�g���N�^
EnemyNormal::EnemyNormal()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	model = std::make_unique<Model>(device, slime_model_file, slime_render_size);
}

//�f�X�g���N�^
EnemyNormal::~EnemyNormal()
{
}

void EnemyNormal::Init()
{
	//�ҋ@�X�e�[�g�֑J��
	TransitionState(State::Idle);
}

//�X�V����
void EnemyNormal::Update(float elapsedTime)
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
void EnemyNormal::UpdateEachState(float elapsedTime)
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
void EnemyNormal::ShadowRender(const RenderContext& rc, ShadowMap* shadowMap)
{
	shadowMap->Draw(rc, model.get());
}
void EnemyNormal::Render(const RenderContext& rc, ModelShader* shader)
{
	shader->Draw(rc, model.get());
}

//�_���[�W���ɌĂ΂��
void EnemyNormal::OnDamaged()
{
	//�_���[�W�X�e�[�g�֑J��
	TransitionState(State::HitDamage);
}
//���S�������ɌĂ΂��
void EnemyNormal::OnDead()
{
	int i = 0;
	// �^�����_���[�W�ʂ����Ȃ������Level�������Ȃ�
	for (Player* player : PlayerManager::Instance().players)
	{
		if (static_cast<float>(attackedDamage[i]) / maxHealth > add_level_min_damage_rate) {
			player->AddLevel(slime_add_level_up);
		}
		i++;
	}
	// �ł��_���[�W��^�����v���C���[�͂����LevelUp
	GetMostAttackPlayer()->AddLevel(most_attack_bonus_level_up);

	//���g��j��
	Destroy();
}

Player::EnemySearch EnemyNormal::GetNearestPlayer_EnemySearch()
{
	Player::EnemySearch es = Player::EnemySearch::None;

	for (Player* player : PlayerManager::Instance().players)
	{
		if (SC_INT(es) < SC_INT(player->GetEachEnemySearch(this)))
			es = player->GetEachEnemySearch(this);
	}
	return es;
}

//�^�[�Q�b�g�ʒu��ݒ�
void EnemyNormal::UpdateTargetPosition()
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
void EnemyNormal::MoveToTarget(float elapsedTime, float speedRate)
{
	//�^�[�Q�b�g�����ւ̐i�s�x�N�g�����Z�o
	float vx = targetPosition.x - position.x;
	float vz = targetPosition.z - position.z;
	float dist = sqrtf(vx * vx + vz * vz);
	vx /= dist;
	vz /= dist;

	//�ړ�����
	Move(vx, vz, slime_move_speed * speedRate);
	Turn(elapsedTime, vx, vz, slime_turn_speed * speedRate);
}

//�ŋ�Player�ւ̉�]
void EnemyNormal::TurnToTarget(float elapsedTime, float speedRate)
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
	Turn(elapsedTime, vx, vz, slime_turn_speed * speedRate);
}

//�v���C���[���G
bool EnemyNormal::SearchPlayer()
{
	float vx = targetPosition.x - position.x;
	float vy = targetPosition.y - position.y;
	float vz = targetPosition.z - position.z;
	float dist = vx * vx + vy * vy + vz * vz;

	if (dist < slime_attack_range * slime_attack_range)//�K��������
	{
		return true;
	}

	if (dist < slime_search_range * slime_search_range)
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
void EnemyNormal::CollisionNodeVsPlayer(const char* nodeName, float nodeRadius)
{
	//�m�[�h�擾
	Model::Node* node = model->FindNode(nodeName);
	if (node == nullptr) return;

	//�m�[�h�ʒu�擾
	XMFLOAT3 nodePosition = {};
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
			if (player->ApplyDamage(attackDamage))
			{
				//�G�𐁂���΂��x�N�g�����Z�o
				XMFLOAT3 vec = {};
				vec.x = outPosition.x - nodePosition.x;
				vec.z = outPosition.z - nodePosition.z;
				float length = sqrtf(vec.x * vec.x + vec.z * vec.z);
				vec.x /= length;
				vec.z /= length;

				//XZ���ʂɐ�����΂��͂�������
				vec.x *= slime_impulse_power_rate;
				vec.z *= slime_impulse_power_rate;
				//Y�����͋��������㑤�ւ̗͂�������
				vec.y = slime_impulse_power_y;

				//������΂�
				player->AddImpulse(vec);
			}
		}
	}
}

/****************�X�V����****************/

//�p�j�X�e�[�g�X�V����
void EnemyNormal::UpdateWanderState(float elapsedTime)
{
	//�ڕW�n�_�։�]
	TurnToTarget(elapsedTime, slime_turn_speed);

	//�v���C���[���G
	if (SearchPlayer())
	{
		//����������ǐՃX�e�[�g�֑J��
		TransitionState(State::Pursuit);
	}
}

//�ҋ@�X�e�[�g�X�V����
void EnemyNormal::UpdateIdleState(float elapsedTime)
{
	//�v���C���[���G
	if (SearchPlayer())
	{
		//����������ǐՃX�e�[�g�֑J��
		TransitionState(State::Pursuit);
	}
}

//�ǐՃX�e�[�g�X�V����
void EnemyNormal::UpdatePursuitState(float elapsedTime)
{
	//�ڕW�։�]
	TurnToTarget(elapsedTime, slime_turn_speed);

	//�v���C���[���U���͈͂ɂ����ꍇ�͍U���X�e�[�g�֑J��
	Player::EnemySearch es = GetNearestPlayer_EnemySearch();

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
void EnemyNormal::UpdateAttackState(float elapsedTime)
{
	//�ڕW�։�]
	TurnToTarget(elapsedTime, slime_turn_speed);

	//�C�ӂ̃A�j���[�V�����Đ���Ԃł̂ݏՓ˔��菈��������
	float animationTime = model->GetCurrentAnimationSeconds();
	if (IsDuringTime(animationTime, slime_attack_time))
	{
		//�ڋʃm�[�h�ƃv���C���[�̏Փˏ���
		CollisionNodeVsPlayer(eye_ball_node_name, attack_node_radius);
	}

	//�U���A�j���[�V�������I�������퓬�ҋ@�X�e�[�g�֑J��
	if (!model->IsPlayAnimation())
	{
		TransitionState(State::IdleBattle);
	}
}

//�퓬�ҋ@�X�e�[�g�X�V����
void EnemyNormal::UpdateIdleBattleState(float elapsedTime)
{
	//�v���C���[���U���͈͂ɂ����ꍇ�͍U���X�e�[�g�֑J��
	Player::EnemySearch es = GetNearestPlayer_EnemySearch();

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
	}

//�_���[�W�X�e�[�g�X�V����
void EnemyNormal::UpdateHitDamageState(float elapsedTime)
{
	if (model->IsPlayAnimation()) return;

	//�v���C���[���U���͈͂ɂ����ꍇ�͍U���X�e�[�g�֑J��
	Player::EnemySearch es = GetNearestPlayer_EnemySearch();

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
void EnemyNormal::TransitionState(State nowState)
{
	state = nowState; //�X�e�[�g�ݒ�
	TransitionPlayAnimation(nowState);
}
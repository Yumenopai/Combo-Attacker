#include "EnemySlime.h"
#include "EnemyConst.h"
#include "Graphics/Graphics.h"
#include "PlayerManager.h"
#include "Collision.h"
#include "MathScript.h"
#include "AnimationTimeStruct.h"

//�R���X�g���N�^
EnemySlime::EnemySlime()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	model = std::make_unique<Model>(device, slime_model_file, slime_render_size);
	eye_ball_node_name = slime_eye_ball_node_name;

	angle.y = Math::RandomRange(-360, 360);
	radius = slime_radius;
	height = slime_height;
	health = maxHealth = slime_max_health;

	attackDamage = slime_attack_damage;
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

//�_���[�W���ɌĂ΂��
void EnemySlime::OnDamaged()
{
	//�_���[�W�X�e�[�g�֑J��
	TransitionState(State::HitDamage);
}
//���S�������ɌĂ΂��
void EnemySlime::OnDead()
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

Player::EnemySearch EnemySlime::GetNearestPlayer_EnemySearch()
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
	Move(vx, vz, slime_move_speed * speedRate);
	Turn(elapsedTime, vx, vz, slime_turn_speed * speedRate);
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
	Turn(elapsedTime, vx, vz, slime_turn_speed * speedRate);
}

//�v���C���[���G
bool EnemySlime::SearchPlayer()
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
void EnemySlime::CollisionNodeVsPlayer(const char* nodeName, float nodeRadius)
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
void EnemySlime::UpdateWanderState(float elapsedTime)
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
void EnemySlime::UpdateAttackState(float elapsedTime)
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
void EnemySlime::UpdateIdleBattleState(float elapsedTime)
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
void EnemySlime::UpdateHitDamageState(float elapsedTime)
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
void EnemySlime::TransitionState(State nowState)
{
	state = nowState; //�X�e�[�g�ݒ�
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

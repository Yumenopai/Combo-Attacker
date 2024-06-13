#include "EnemyBoss.h"
#include "Graphics/Graphics.h"
#include "Character/Manager/PlayerManager.h"
#include "Collision.h"
#include "AnimationTimeStruct.h"
#include "imgui.h"

//�R���X�g���N�^
EnemyBoss::EnemyBoss()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	model = std::make_unique<Model>(device, dragon_model_file, dragon_render_size);
}

//�f�X�g���N�^
EnemyBoss::~EnemyBoss()
{
}

// ��������
void EnemyBoss::Init()
{
	//�ҋ@�X�e�[�g�֑J��
	TransitionState(State::Idle);
}

//�X�e�[�g�J��
void EnemyBoss::TransitionState(State nowState)
{
	state = nowState; //�X�e�[�g�ݒ�

	switch (nowState)
	{
	case State::Wander:
		//�ڕW�n�_�ݒ�
		SetRandomTargetPosition();
	}

	TransitionPlayAnimation(nowState);
}

//�X�V����
void EnemyBoss::Update(float elapsedTime)
{
	UpdateTargetPosition();

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
	case State::GetHit:
		UpdateGetHitState(elapsedTime);
		break;
	case State::Scream:
		UpdateScreamState(elapsedTime);
		break;
	case State::AttackClaw:
		UpdateAttackClawState(elapsedTime);
		break;
	case State::Die:
		UpdateDieState(elapsedTime);
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
void EnemyBoss::ShadowRender(const RenderContext& rc, ShadowMap* shadowMap)
{
	shadowMap->Draw(rc, model.get());
}
void EnemyBoss::Render(const RenderContext& rc, ModelShader* shader)
{
	shader->Draw(rc, model.get());
}

//�_���[�W���ɌĂ΂��
void EnemyBoss::OnDamaged()
{
	if (!isHalfHP && health <= maxHealth / 2) // �����܂ł�����
	{
		//�_���[�W�X�e�[�g�֑J��
		TransitionState(State::GetHit);
		isHalfHP = true;
	}
}
//���S�������ɌĂ΂��
void EnemyBoss::OnDead()
{
	if (isDead) return;

	int i = 0;
	// �^�����_���[�W�ʂ����Ȃ������Level�������Ȃ�
	for (Player* player : PlayerManager::Instance().players)
	{
		if (attackedDamage[i] > (maxHealth / add_level_min_damage_rate)) {
			player->AddLevel(dragon_add_level_up);
		}
		i++;
	}
	// �ł��_���[�W��^�����v���C���[�͂����LevelUp
	GetMostAttackPlayer()->AddLevel(most_attack_bonus_level_up);

	//���S�X�e�[�g�֑J��
	TransitionState(State::Die);
	isDead = true;
}

Player::EnemySearch EnemyBoss::GetNearestPlayer_EnemySearch()
{
	Player::EnemySearch es = Player::EnemySearch::None;

	for (Player* player : PlayerManager::Instance().players)
	{
		if (static_cast<int>(es) < static_cast<int>(player->GetEachEnemySearch(this)))
			es = player->GetEachEnemySearch(this);
	}
	return es;
}

//�^�[�Q�b�g�ʒu��ݒ�
void EnemyBoss::UpdateTargetPosition()
{
	//�v���C���[�Ƃ̍��፷���l������3D�ł̋������������
	minLen = FLT_MAX;
	for (Player* player : PlayerManager::Instance().players)
	{
		nowLen = player->GetEachEnemyDist(this);
		if (nowLen < minLen)
		{
			minLen = nowLen;
			targetPosition = player->GetPosition();
		}
	}
}

//�^�[�Q�b�g�ʒu�������_���ݒ�
void EnemyBoss::SetRandomTargetPosition()
{
	float theta = 0;
	float range = 0;

	targetPosition.x = territoryOrigin.x + range * sinf(theta);
	targetPosition.y = territoryOrigin.y;
	targetPosition.z = territoryOrigin.z + range * cosf(theta);
}

//�ڕW�n�_�ֈړ�
void EnemyBoss::MoveToTarget(float elapsedTime, float speedRate)
{
	//�^�[�Q�b�g�����ւ̐i�s�x�N�g�����Z�o
	float vx = targetPosition.x - position.x;
	float vz = targetPosition.z - position.z;
	float dist = sqrtf(vx * vx + vz * vz);
	vx /= dist;
	vz /= dist;

	//�ړ�����
	Move(vx, vz, dragon_move_speed * speedRate);
	Turn(elapsedTime, vx, vz, dragon_turn_speed * speedRate);
}

//�v���C���[���G
bool EnemyBoss::SearchPlayer()
{
	Player::EnemySearch es = GetNearestPlayer_EnemySearch();
	if (es >= Player::EnemySearch::Find)
	{
		return true;
	}
	return false;
}

//�p�j�X�e�[�g�X�V����
void EnemyBoss::UpdateWanderState(float elapsedTime)
{
	//�ڕW�n�_�ֈړ�
	MoveToTarget(elapsedTime, 0.5f);// ���x���[�g�w��F�ʏ��50%���x

	//�v���C���[���G
	if (SearchPlayer())
	{
		//����������ǐՃX�e�[�g�֑J��
		TransitionState(State::Pursuit);
	}
}

//�ҋ@�X�e�[�g�X�V����
void EnemyBoss::UpdateIdleState(float elapsedTime)
{
	//�v���C���[���G
	if (SearchPlayer())
	{
		//����������ǐՃX�e�[�g�֑J��
		TransitionState(State::Pursuit);
	}
}

//�ǐՃX�e�[�g�X�V����
void EnemyBoss::UpdatePursuitState(float elapsedTime)
{
	//�ڕW�n�_�ֈړ�
	MoveToTarget(elapsedTime);

	//�v���C���[���߂Â��ƍU���X�e�[�g�֑J��
	Player::EnemySearch es = GetNearestPlayer_EnemySearch();

	if (es >= Player::EnemySearch::Attack)
	{
		//�U���X�e�[�g�֑J��
		isHalfHP ? TransitionState(State::AttackClaw): TransitionState(State::Attack);
	}
	else if (!SearchPlayer())
	{
		TransitionState(State::Idle);
	}
}

//�U���X�e�[�g�X�V����
void EnemyBoss::UpdateAttackState(float elapsedTime)
{
	//�C�ӂ̃A�j���[�V�����Đ���Ԃł̂ݏՓ˔��菈��������
	float animationTime = model->GetCurrentAnimationSeconds();
	if (IsDuringTime(animationTime, dragon_attack_head_time))
	{
		//���m�[�h�ƃv���C���[�̏Փˏ���
		CollisionNodeVsPlayer(dragon_head_node_name, attack_node_radius);
	}	

	//�U���A�j���[�V�������I�������퓬�ҋ@�X�e�[�g�֑J��
	if (!model->IsPlayAnimation())
	{
		TransitionState(State::IdleBattle);
	}
}

//�퓬�ҋ@�X�e�[�g�X�V����
void EnemyBoss::UpdateIdleBattleState(float elapsedTime)
{
	//�v���C���[���U���͈͂ɂ����ꍇ�͍U���X�e�[�g�֑J��
	Player::EnemySearch es = GetNearestPlayer_EnemySearch();

	if (es >= Player::EnemySearch::Attack)
	{
		//�U���X�e�[�g�֑J��
		isHalfHP ? TransitionState(State::AttackClaw) : TransitionState(State::Attack);
	}
	else
	{
		//�ҋ@�X�e�[�g�֑J��
		TransitionState(State::Idle);
	}
}

//���K�X�e�[�g�X�V����
void EnemyBoss::UpdateScreamState(float elapsedTime)
{
	if (model->IsPlayAnimation()) return;
	
	//�A�j���[�V�����I����
	{
		//�U���X�e�[�g�֑J��
		TransitionState(State::IdleBattle);
	}
}

//���U���X�e�[�g�X�V����
void EnemyBoss::UpdateAttackClawState(float elapsedTime)
{
	//�C�ӂ̃A�j���[�V�����Đ���Ԃł̂ݏՓ˔��菈��������
	float animationTime = model->GetCurrentAnimationSeconds();
	if (IsDuringTime(animationTime, dragon_attack_claw_time))
	{
		//�܃m�[�h�ƃv���C���[�̏Փˏ���
		CollisionNodeVsPlayer(dragon_clawL_node_name, attack_node_radius);
		CollisionNodeVsPlayer(dragon_clawR_node_name, attack_node_radius);
	}

	//�U���A�j���[�V�������I�������퓬�ҋ@�X�e�[�g�֑J��
	if (!model->IsPlayAnimation())
	{
		TransitionState(State::IdleBattle);
	}
}

//�퓬�ҋ@�X�e�[�g�X�V����
void EnemyBoss::UpdateGetHitState(float elapsedTime)
{
	if (model->IsPlayAnimation()) return;
	
	//�A�j���[�V�����I����
	{
		//���K�X�e�[�g�֑J��
		TransitionState(State::Scream);
	}
}

//���S�X�e�[�g�X�V����
void EnemyBoss::UpdateDieState(float elapsedTime)
{
	//�A�j���[�V�����Đ����͉������Ȃ�
	if (model->IsPlayAnimation()) return;
	
	//�A�j���[�V�����I���ɍ��킹�G�t�F�N�g�Đ�
	PlayEffect(EffectNumber::dead, position);

	//�j��
	Destroy();
}

//�m�[�h�ƃv���C���[�̏Փˏ���
void EnemyBoss::CollisionNodeVsPlayer(const char* nodeName, float nodeRadius)
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
		// �������Ă��Ȃ��ꍇ���^�[��
		if (!Collision::IntersectSphereVsCylinder(
			nodePosition, nodeRadius,
			player->GetPosition(), player->GetRadius(), player->GetHeight(),
			outPosition)) return;
		// �_���[�W��^���Ȃ��ꍇ���^�[��
		if (!player->ApplyDamage(attackDamage)) return;

		// �G�𐁂���΂��x�N�g�����Z�o
		XMFLOAT3 vec = {};
		vec.x = outPosition.x - nodePosition.x;
		vec.z = outPosition.z - nodePosition.z;
		float length = sqrtf(vec.x * vec.x + vec.z * vec.z);
		vec.x /= length;
		vec.z /= length;

		// XZ���ʂɐ�����΂��͂�������
		vec.x *= dragon_impulse_power_rate;
		vec.z *= dragon_impulse_power_rate;
		//Y�����͋��������㑤�ւ̗͂�������
		vec.y = dragon_impulse_power_y;

		// ������΂�
		player->AddImpulse(vec);
	}
}

#pragma region DEBUG_DRAW
void EnemyBoss::DebugMenu()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	// �f�o�b�O������\���̕ύX
	std::string str = "";
	// ���݂̃X�e�[�g�ԍ��ɍ��킹�ăf�o�b�O�������str�Ɋi�[
	switch (state)
	{
	case State::Wander:
		str = "Wander";
		break;
	case State::Idle:
		str = "Idle";
		break;
	case State::Pursuit:
		str = "Pursuit";
		break;
	case State::Attack:
		str = "Attack";
		break;
	case State::IdleBattle:
		str = "IdleBattle";
		break;
	case State::GetHit:
		str = "GetHit";
		break;
	default:
		break;
	}

	if (ImGui::Begin("Dragon", nullptr, ImGuiWindowFlags_None))
	{
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//�ʒu
			ImGui::DragFloat3("Position", &position.x, 0.1f);

			//��]
			XMFLOAT3 a = {};
			a.x = XMConvertToDegrees(angle.x);
			a.y = XMConvertToDegrees(angle.y);
			a.z = XMConvertToDegrees(angle.z);
			ImGui::DragFloat3("Angle", &a.x, 1.0f);
			if (a.y > 360) a.y = 0;
			if (a.y < 0) a.y = 360;
			angle.x = XMConvertToRadians(a.x);
			angle.y = XMConvertToRadians(a.y);
			angle.z = XMConvertToRadians(a.z);

			//�X�P�[��
			ImGui::DragFloat3("Scale", &scale.x, 0.01f);

			ImGui::Text(u8"State�@%s", str.c_str());
			ImGui::Text(u8"nearestPlayer�@%s", nearestPlayerStr.c_str());
			ImGui::Text(u8"minLen�@%f", minLen);
			ImGui::Text(u8"nowLen�@%f", nowLen);
		}
		ImGui::End();
	}
}
#pragma endregion
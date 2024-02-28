#include "EnemyBlue.h"
#include "Graphics/Graphics.h"
#include "PlayerManager.h"
#include "Collision.h"
#include "imgui.h"

//�R���X�g���N�^
EnemyBlue::EnemyBlue()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	model = std::make_unique<Model>(device, "Data/Model/Enemy/Blue.fbx", 0.008f);

	height = 3.6f;
	radius = 1.8f;
	health = maxHealth = 100;
	isHalfHP = false;

	//�ҋ@�X�e�[�g�֑J��
	TransitionIdleState();
}

//�f�X�g���N�^
EnemyBlue::~EnemyBlue()
{
}

//�X�V����
void EnemyBlue::Update(float elapsedTime)
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

//�_���[�W���ɌĂ΂��
void EnemyBlue::OnDamaged()
{
	if (health <= 50 && !isHalfHP)
	{
		//�_���[�W�X�e�[�g�֑J��
		TransitionGetHitState();
		isHalfHP = true;
	}
}
//���S�������ɌĂ΂��
void EnemyBlue::OnDead()
{
	if (!isDead)
	{
		//���S�X�e�[�g�֑J��
		TransitionDieState();
		isDead = true;
	}
}

Player::EnemySearch EnemyBlue::GetNearestPlayerES()
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

//�^�[�Q�b�g�ʒu��ݒ�
void EnemyBlue::UpdateTargetPosition()
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

	//targetPosition = Player1P::Instance().GetPosition();
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
	Player::EnemySearch es = GetNearestPlayerES();
	if (es >= Player::EnemySearch::Find)
	{
		return true;
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
	model->PlayAnimation(Anim_Walk, true);
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
	model->PlayAnimation(Anim_Idle1, true);
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
	model->PlayAnimation(Anim_Run, true);
}

//�ǐՃX�e�[�g�X�V����
void EnemyBlue::UpdatePursuitState(float elapsedTime)
{
	//�ڕW�n�_�ֈړ�
	MoveToTarget(elapsedTime, 1.0f);

	//�v���C���[���߂Â��ƍU���X�e�[�g�֑J��
	Player::EnemySearch es = GetNearestPlayerES();

	if (es >= Player::EnemySearch::Attack)
	{
		//�U���X�e�[�g�֑J��
		isHalfHP ? TransitionAttackClawState() : TransitionAttackState();
	}
	else if (!SearchPlayer())
	{
		TransitionIdleState();
	}
}

//�U���X�e�[�g�֑J��
void EnemyBlue::TransitionAttackState()
{
	state = State::Attack;

	//�U���A�j���[�V�����Đ�
	model->PlayAnimation(Anim_Attack01, false);
}

//�U���X�e�[�g�X�V����
void EnemyBlue::UpdateAttackState(float elapsedTime)
{
	//�C�ӂ̃A�j���[�V�����Đ���Ԃł̂ݏՓ˔��菈��������
	float animationTime = model->GetCurrentAnimationSeconds();
	if (animationTime >= 0.35f && animationTime <= 0.6f)
	{
		//���m�[�h�ƃv���C���[�̏Փˏ���
		CollisionNodeVsPlayer("Jaw3", 0.2f);

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
	model->PlayAnimation(Anim_Idle02, true);
}

//�퓬�ҋ@�X�e�[�g�X�V����
void EnemyBlue::UpdateIdleBattleState(float elapsedTime)
{
	//�v���C���[���U���͈͂ɂ����ꍇ�͍U���X�e�[�g�֑J��
	Player::EnemySearch es = GetNearestPlayerES();

	if (es >= Player::EnemySearch::Attack)
	{
		//�U���X�e�[�g�֑J��
		isHalfHP ? TransitionAttackClawState() : TransitionAttackState();
	}
	else
	{
		//�ҋ@�X�e�[�g�֑J��
		TransitionIdleState();
	}

	MoveToTarget(elapsedTime, 0.0f);
}

//���K�X�e�[�g�֑J��
void EnemyBlue::TransitionScreamState()
{
	state = State::Scream;

	//���K�A�j���[�V�����Đ�
	model->PlayAnimation(Anim_Scream, false);
}

//���K�X�e�[�g�X�V����
void EnemyBlue::UpdateScreamState(float elapsedTime)
{
	if (model->IsPlayAnimation()) return;
	
	//�A�j���[�V�����I����
	{
		//�U���X�e�[�g�֑J��
		TransitionIdleBattleState();
	}
}

//���U���X�e�[�g�֑J��
void EnemyBlue::TransitionAttackClawState()
{
	state = State::AttackClaw;

	//���U���A�j���[�V�����Đ�
	model->PlayAnimation(Anim_AttackClaw, false);
}

//���U���X�e�[�g�X�V����
void EnemyBlue::UpdateAttackClawState(float elapsedTime)
{
	//�C�ӂ̃A�j���[�V�����Đ���Ԃł̂ݏՓ˔��菈��������
	float animationTime = model->GetCurrentAnimationSeconds();
	if (animationTime >= 0.4f && animationTime <= 1.05f)
	{
		//�܃m�[�h�ƃv���C���[�̏Փˏ���
		CollisionNodeVsPlayer("WingClaw2_L", 1);
		CollisionNodeVsPlayer("WingClaw2_L_1", 1);
	}

	//�U���A�j���[�V�������I�������퓬�ҋ@�X�e�[�g�֑J��
	if (!model->IsPlayAnimation())
	{
		TransitionIdleBattleState();
	}
}

//�퓬�ҋ@�X�e�[�g�֑J��
void EnemyBlue::TransitionGetHitState()
{
	state = State::GetHit;

	//�퓬�ҋ@�A�j���[�V�����Đ�
	model->PlayAnimation(Anim_GetDamage, false);
}

//�퓬�ҋ@�X�e�[�g�X�V����
void EnemyBlue::UpdateGetHitState(float elapsedTime)
{
	if (model->IsPlayAnimation()) return;
	
	//�A�j���[�V�����I����
	{
		//���K�X�e�[�g�֑J��
		TransitionScreamState();
	}
}
//���S�X�e�[�g�֑J��
void EnemyBlue::TransitionDieState()
{
	state = State::Die;

	//�퓬�ҋ@�A�j���[�V�����Đ�
	model->PlayAnimation(Anim_Die, false);
}
//���S�X�e�[�g�X�V����
void EnemyBlue::UpdateDieState(float elapsedTime)
{
	if (model->IsPlayAnimation()) return;
	
	//�A�j���[�V�����I����
	{
		Destroy();
	}
}

//�m�[�h�ƃv���C���[�̏Փˏ���
void EnemyBlue::CollisionNodeVsPlayer(const char* nodeName, float nodeRadius)
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
			if (player->ApplyDamage(2, 0))
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
				player->AddImpulse(vec);
			}
		}
	}
}

void EnemyBlue::DebugMenu()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);


	// �f�o�b�O������\���̕ύX
	std::string str = "";
	// ���݂̃X�e�[�g�ԍ��ɍ��킹�ăf�o�b�O�������str�Ɋi�[
	switch (state)
	{
	case EnemyBlue::State::Wander:
		str = "Wander";
		break;
	case EnemyBlue::State::Idle:
		str = "Idle";
		break;
	case EnemyBlue::State::Pursuit:
		str = "Pursuit";
		break;
	case EnemyBlue::State::Attack:
		str = "Attack";
		break;
	case EnemyBlue::State::IdleBattle:
		str = "IdleBattle";
		break;
	case EnemyBlue::State::GetHit:
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
			XMFLOAT3 a;
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
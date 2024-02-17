#include <map>

#include "Player.h"
#include "Graphics/Graphics.h"
#include "Graphics/PrimitiveRenderer.h"
#include "Input/Input.h"
#include "SceneManager.h"
#include "SceneLoading.h"
#include "SceneGame.h"
#include "SceneClear.h"
#include "PlayerManager.h"
#include "State/Player/PlayerEachState.h"
#include "EnemyManager.h"

#include "Stage.h"
#include "imgui.h"

//�R���X�g���N�^
Player::Player()
{
}

Player::~Player()
{
}

// ������
void Player::Init()
{
	//�q�b�g�G�t�F�N�g�ǂݍ���
	hitEffect = std::make_unique<Effect>("Data/Effect/Hit.efk");

	//������
	enemySearch.clear();
	enemyDist.clear();
	EnemyManager& enemyManager = EnemyManager::Instance();
	int enemyCount = enemyManager.GetEnemyCount();//�S�Ă̓G�Ƒ�������ŏՓˏ���
	for (int i = 0; i < enemyCount; i++)
	{
		enemySearch[enemyManager.GetEnemy(i)] = EnemySearch::None;
		enemyDist[enemyManager.GetEnemy(i)] = FLT_MAX;
	}

	health = 100;
	maxHealth = 100;

	// StateMachine�𐶐� 
	stateMachine = new PlayerStateMachine();
	// �X�e�[�g�}�V���ɃX�e�[�g�o�^ 
	stateMachine->RegisterState(new StateIdle(this));
	stateMachine->RegisterState(new StateIdleToRun(this));
	stateMachine->RegisterState(new StateRun(this));
	stateMachine->RegisterState(new StateRunToIdle(this));
	stateMachine->RegisterState(new StateJumpStart(this));
	stateMachine->RegisterState(new StateJumpLoop(this));
	stateMachine->RegisterState(new StateJumpAir(this));
	stateMachine->RegisterState(new StateJumpEnd(this));
	stateMachine->RegisterState(new StateDamage(this));
	stateMachine->RegisterState(new StateDead(this));
	stateMachine->RegisterState(new StateRecover(this));
	stateMachine->RegisterState(new StateAttackHammer1(this));
	stateMachine->RegisterState(new StateAttackHammer2(this));
	stateMachine->RegisterState(new StateAttackHammerJump(this));
	stateMachine->RegisterState(new StateAttackSpear1(this));
	stateMachine->RegisterState(new StateAttackSpear2(this));
	stateMachine->RegisterState(new StateAttackSpear3(this));
	stateMachine->RegisterState(new StateAttackSpearJump(this));
	stateMachine->RegisterState(new StateAttackSword1(this));
	stateMachine->RegisterState(new StateAttackSword2(this));
	stateMachine->RegisterState(new StateAttackSword3(this));
	stateMachine->RegisterState(new StateAttackSwordJump(this));

	// �X�e�[�g���Z�b�g 
	stateMachine->SetState(static_cast<int>(State::Idle));
}

// �U���̋O�Օ`��
void Player::PrimitiveRender(const RenderContext& rc)
{
	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();

	// �|���S���`��
	primitiveRenderer->Render(rc.deviceContext, rc.camera->GetView(), rc.camera->GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void Player::DebugMenu()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Player", nullptr, ImGuiWindowFlags_None))
	{
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//�ʒu
			ImGui::DragFloat3("Position", &position.x, 0.1f);
			ImGui::DragFloat3("Offset", &offset.x, 0.1f);

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
		}
			
		ImGui::Checkbox("attacking", &isAttackjudge);

		ImGui::End();
	}
}

//���n�������ɌĂ΂��
void Player::OnLanding(float elapsedTime)
{
	if (Atype != AttackType::None) //�U����(��ɃW�����v�U����)
	{
		// ���n���Ă����͉��������Ȃ����߂����ŏ����������Ȃ�
		// �eStateUpdate�ɂăA�j���[�V�����I�����IdleState�֑J�ڂ���
	}
	else if (InputMove(elapsedTime))
	{
		stateMachine->ChangeState(static_cast<int>(State::Run));
	}
	else {
		stateMachine->ChangeState(static_cast<int>(State::JumpEnd));
	}
}

void Player::OnDamaged()
{
	isDamaged = true;
	stateMachine->ChangeState(static_cast<int>(State::Damage));
}

void Player::OnDead()
{
	if(!isDead)
	{
		stateMachine->ChangeState(static_cast<int>(State::Dead));
		isDead = true;
	}
}

void Player::UpdateArmPositions(Model* model, Arms& arm)
{
	Model::Node* armBone = model->FindNode(arm.nodeName);
	XMMATRIX W = XMLoadFloat4x4(&armBone->worldTransform);
	XMVECTOR V = arm.tipOffset;
	XMVECTOR P = XMVector3Transform(V, W);
	XMStoreFloat3(&arm.position, P);

	// ���̍��{�Ɛ�[�̍��W���擾���A���_�o�b�t�@�ɕۑ�
	// ���̌��_���獪�{�Ɛ�[�܂ł̃I�t�Z�b�g�l
	DirectX::XMVECTOR RootOffset = arm.rootOffset;
	DirectX::XMVECTOR TipOffset = arm.tipOffset;

	XMVECTOR RootP = XMVector3Transform(RootOffset, W);
	XMVECTOR TipP = XMVector3Transform(TipOffset, W);
	DirectX::XMStoreFloat3(&trailPositions[0][0], RootP);
	DirectX::XMStoreFloat3(&trailPositions[1][0], TipP);
}

void Player::HorizontalVelocityByAttack(bool plus, int velo, float elapsedTime)
{
	if (enemySearch[nearestEnemy] == EnemySearch::Attack) return;

	if (plus)
	{
		velocity.x += sinf(angle.y) * velo * elapsedTime;
		velocity.z += cosf(angle.y) * velo * elapsedTime;
	}
	else
	{
		velocity.x = sinf(angle.y) * velo * elapsedTime;
		velocity.z = cosf(angle.y) * velo * elapsedTime;
	}
}

void Player::ShiftTrailPositions()
{
	for (int i = MAX_POLYGON - 1; i > 0; i--)
	{
		// ���ւ��炵�Ă���
		trailPositions[0][i] = trailPositions[0][i - 1];
		trailPositions[1][i] = trailPositions[1][i - 1];
	}
}
void Player::RenderTrail()
{
	// �|���S���쐬
	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();
	for (int i = 0; i < MAX_POLYGON - 3; ++i)
	{
		const int division = 10;

		XMVECTOR RPos1 = XMLoadFloat3(&trailPositions[0][i + 0]);
		XMVECTOR RPos2 = XMLoadFloat3(&trailPositions[0][i + 1]);
		XMVECTOR RPos3 = XMLoadFloat3(&trailPositions[0][i + 2]);
		XMVECTOR RPos4 = XMLoadFloat3(&trailPositions[0][i + 3]);
		XMVECTOR TPos1 = XMLoadFloat3(&trailPositions[1][i + 0]);
		XMVECTOR TPos2 = XMLoadFloat3(&trailPositions[1][i + 1]);
		XMVECTOR TPos3 = XMLoadFloat3(&trailPositions[1][i + 2]);
		XMVECTOR TPos4 = XMLoadFloat3(&trailPositions[1][i + 3]);
		for (int j = 1; j < division; ++j)
		{
			float t = j / static_cast<float>(division);

			XMFLOAT3 Position[2];
			DirectX::XMStoreFloat3(&Position[0], XMVectorCatmullRom(RPos1, RPos2, RPos3, RPos4, t));
			DirectX::XMStoreFloat3(&Position[1], XMVectorCatmullRom(TPos1, TPos2, TPos3, TPos4, t));

			if (isAddVertex)
			{
				primitiveRenderer->AddVertex(Position[0], color);
				primitiveRenderer->AddVertex(Position[1], color);
			}
		}
	}
}

//�������͍X�V�I�[�o�[���C�h
void Player::UpdateVerticalVelocity(float elapsedFrame)
{
	auto state = static_cast<Player::State>(stateMachine->GetStateNumber());

	// �W�����v�X�s�A�[�U���̂ݕʂ̉������������g�p����
	if (state == State::AttackSpearJump)
	{
		if (velocity.y > 0)	velocity.y = 0;
		velocity.y += gravity * 0.25f * elapsedFrame;
	}
	else
	{
		velocity.y += gravity * elapsedFrame;
	}
}

// ===========���͏���===========
//�ړ����͏���
bool Player::InputMove(float elapsedTime)
{
	XMFLOAT3 moveVec = {}; //GetMoveVec();
	XMVECTOR MoveVec = XMLoadFloat3(&moveVec); //�i�s�x�N�g�����擾

	// �����Ă��Ċ��ŋߋ������o�^����Ă���
	if (XMVectorGetX(XMVector3LengthSq(MoveVec)) != 0 && nearestDist < FLT_MAX)
	{
		// �������Ă���̂��G�l�~�[�x�N�g���Ɖs�p�֌W�Ȃ�
		float dot = XMVectorGetX(XMVector3Dot(MoveVec, XMLoadFloat3(&nearestVec)));
		// �ŋ߃G�l�~�[�Ɍ�����
		if (dot > 0) moveVec = nearestVec;
	}						

	//�ړ�����
	Move(moveVec.x, moveVec.z, moveSpeed);
	//���񏈗�
	Turn(elapsedTime, moveVec.x, moveVec.z, turnSpeed);

	//�i�s�x�N�g�����[���x�N�g���łȂ��ꍇ�͓��͂��ꂽ
	return moveVec.x != 0 || moveVec.y != 0 || moveVec.z != 0;
}

// �U�����͏���
bool Player::InputAttackFromNoneAttack(float elapsedTime)
{
	if (InputHammerButton()) {
		stateMachine->ChangeState(static_cast<int>(State::AttackHammer1));
	}
	else if (InputSwordButton()) {
		stateMachine->ChangeState(static_cast<int>(State::AttackSword1));
	}
	else if (InputSpearButton()) {
		stateMachine->ChangeState(static_cast<int>(State::AttackSpear1));
	}
	else {
		return false;
	}

	if (enemySearch[nearestEnemy] >= EnemySearch::Find)
	{
		//���񏈗�
		Turn(elapsedTime, nearestVec.x, nearestVec.z, 1000);
	}

	return true;
}
bool Player::InputAttackFromJump(float elapsedTime)
{
	if (InputHammerButton()) {
		stateMachine->ChangeState(static_cast<int>(State::AttackHammerJump));
	}
	else if (InputSwordButton()) {
		stateMachine->ChangeState(static_cast<int>(State::AttackSwordJump));
	}
	else if (InputSpearButton()) {
		stateMachine->ChangeState(static_cast<int>(State::AttackSpearJump));
	}
	else {
		return false;
	}

	return true;
}

//�񕜑J�ڊm�F����
bool Player::IsRecoverTransition()
{
	Player* targetplayer = this;
	PlayerManager& playerManager = PlayerManager::Instance();
	int playerCount = playerManager.GetPlayerCount();
	for (int i = 0; i < playerCount; i++)
	{
		if (playerManager.GetPlayer(i) == this) continue;
		targetplayer = playerManager.GetPlayer(i);
	}
	// 20%�ȏ��false
	if (targetplayer->GetHealthRate() > 20) return false;

	XMVECTOR posPlayerthis = XMLoadFloat3(&GetPosition());
	XMVECTOR posPlayertarget = XMLoadFloat3(&targetplayer->GetPosition());
	float distSq = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(posPlayerthis, posPlayertarget)));
	// �߂��������false
	if (distSq > 2.0f * 2.0f) return false;

	// true�ŉ񕜑J��
	return true;
}

//�v���C���[�ƃG�l�~�[�Ƃ̏Փˏ���
void Player::CollisionPlayerVsEnemies()
{
	EnemyManager& enemyManager = EnemyManager::Instance();

	//�S�Ă̓G�Ƒ�������ŏՓˏ���
	int enemyCount = enemyManager.GetEnemyCount();
	for (int i = 0; i < enemyCount; i++)
	{
		Enemy* enemy = enemyManager.GetEnemy(i);

		//�Փˏ���
		DirectX::XMFLOAT3 outPosition;
		if (Collision::IntersectCylinderVsCylinder(
			enemy->GetPosition(), enemy->GetRadius(), enemy->GetHeight(),
			position, radius, height,
			XMFLOAT3{}, outPosition
		))
		{
			SetPosition(outPosition);
		}
	}
}

// ����ƃG�l�~�[�̏Փˏ���
void Player::CollisionArmsVsEnemies(Arms arm)
{
	//�w��̃m�[�h�ƑS�Ă̓G�𑍓�����ŏՓˏ���
	EnemyManager& enemyManager = EnemyManager::Instance();
	int enemyCount = enemyManager.GetEnemyCount();

	for (int i = 0; i < enemyCount; ++i)
	{
		Enemy* enemy = enemyManager.GetEnemy(i);

		//�Փˏ���
		XMFLOAT3 outPosition;
		if (Collision::IntersectSphereVsCylinder(
			arm.position, arm.radius,
			enemy->GetPosition(), enemy->GetRadius(), enemy->GetHeight(),
			outPosition
		))
		{
			if (attackingEnemyNumber == i && !isAttackjudge) return; //�U�����肵�Ȃ��ꍇ�͏������Ȃ�

			//�_���[�W��^����
			if (enemy->ApplyDamage(1, 0))
			{
				//������΂�
				if (attackCount >= 4)
				{
					const float power = 13.0f; //���̐����̗�
					const XMFLOAT3& ep = enemy->GetPosition();

					//�m�b�N�o�b�N�����̎Z�o
					float vx = ep.x - Player1P::Instance().position.x;
					float vz = ep.z - Player1P::Instance().position.z;
					float lengthXZ = sqrtf(vx * vx + vz * vz);
					vx /= lengthXZ;
					vz /= lengthXZ;

					//�m�b�N�o�b�N�͂̒�`
					XMFLOAT3 impluse;
					impluse.x = vx * power;
					impluse.z = vz * power;

					impluse.y = power * 0.8f;	//������ɂ��ł��グ��

					enemy->AddImpulse(impluse);
				}
				//�q�b�g�G�t�F�N�g�Đ�
				{
					DirectX::XMFLOAT3 e = enemy->GetPosition();
					e.y += enemy->GetHeight() * 0.5f;
					hitEffect->Play(e);
				}
				attackingEnemyNumber = i;
				isAttackjudge = false;
			}
		}
		else if(attackingEnemyNumber == i)//�U�����̃G�l�~�[�ƈ�U�U�����O�ꂽ���A���񓖂��������ɔ�����s��
		{
			isAttackjudge = true;
		}
	}
}

//�f�o�b�O�v���~�e�B�u�`��
void Player::DrawDebugPrimitive()
{
	Gizmos* gizmos = Graphics::Instance().GetGizmos();
	//�Փ˔���p�̃f�o�b�O�~����`��
	gizmos->DrawCylinder(
		position,		//�ʒu
		radius,			//���a
		height,			//����
		{ 0,0,0 },		//�p�x
		{ 1,0,0,1 });	//�F

	if (Hammer.flag1 || Hammer.flag2 || Hammer.flagJump)
	{
		gizmos->DrawSphere(
			Hammer.position,
			Hammer.radius,
			XMFLOAT4(0, 0, 1, 1)
		);
	}
	if (Spear.flag1 || Spear.flag2 || Spear.flag3 || Spear.flagJump)
	{
		gizmos->DrawSphere(
			Spear.position,
			Spear.radius,
			XMFLOAT4(0, 0, 1, 1)
		);
	}
	if (Sword.flag1 || Sword.flag2 || Sword.flag3 || Sword.flagJump)
	{
		gizmos->DrawSphere(
			Sword.position,
			Sword.radius,
			XMFLOAT4(0, 0, 1, 1)
		);
	}

	Camera& camera = Camera::Instance();
	//�`��R���e�L�X�g�ݒ�
	RenderContext rc;
	rc.camera = &camera;
	rc.deviceContext = Graphics::Instance().GetDeviceContext();
	rc.renderState = Graphics::Instance().GetRenderState();

	//�`����s
	gizmos->Render(rc);
}
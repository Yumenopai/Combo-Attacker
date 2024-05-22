#include <map>

#include "Player.h"
#include "PlayerManager.h"
#include "State/Player/PlayerEachState.h"
#include "Graphics/Graphics.h"
#include "Graphics/PrimitiveRenderer.h"
#include "Input/Input.h"
#include "EnemyManager.h"
#include "Enemy.h"
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
	ID3D11Device* device = Graphics::Instance().GetDevice();
	//�v���C���[���f���ǂݍ���
	model = std::make_unique<Model>(device, "Data/Model/SD-UnityChan/UnityChan.fbx", playerModelSize);

	// ����o�^
	HaveArms.clear();
	for (int i = 0; i < SC_INT(Player::AttackType::MaxCount); i++)
	{
		HaveArms.insert(std::pair<Player::AttackType, bool>(SC_AT(i), false));
	}
	// ��������
	CurrentUseArm = InitialArm;
	HaveArms[InitialArm] = true;

	enemySearch.clear();
	enemyDist.clear();
	EnemyManager& enemyManager = EnemyManager::Instance();
	int enemyCount = enemyManager.GetEnemyCount();//�S�Ă̓G�Ƒ�������ŏՓˏ���
	for (int i = 0; i < enemyCount; i++)
	{
		enemySearch[enemyManager.GetEnemy(i)] = EnemySearch::None;
		enemyDist[enemyManager.GetEnemy(i)] = FLT_MAX;
	}

	// HP�ݒ�
	health = maxHealth = playerMaxHealth;

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
	stateMachine->SetState(SC_INT(State::Idle));
}

void Player::UpdateUtils(float elapsedTime)
{
	// �z��Y����
	//ShiftTrailPositions();

	// �񕜑J�ډ\��
	enableRecoverTransition = EnableRecoverTransition();

	// �X�e�[�g���ɒ��ŏ�������
	stateMachine->Update(elapsedTime);

	// ���̋O�Օ`��X�V����
	//RenderTrail();

	// �W�����v����
	UpdateJumpState(elapsedTime);

	// �v���C���[�ƃG�l�~�[�Ƃ̏Փˏ���
	CollisionPlayerVsEnemies();

	// ���͏����X�V
	UpdateVelocity(elapsedTime);

	// �I�u�W�F�N�g�s��X�V
	UpdateTransform();

	// ���f���A�j���[�V�����X�V����
	model->UpdateAnimation(elapsedTime);

	// ���f���s��X�V
	model->UpdateTransform(transform);
}

// �G�Ƃ̋����X�V
void Player::UpdateEnemyDistance(float elapsedTime)
{
	EnemyManager& enemyManager = EnemyManager::Instance();
	int enemyCount = enemyManager.GetEnemyCount();

	nearestEnemy = nullptr;
	nearestDist = FLT_MAX;
	nearestVec = {};
	int noneEnemy = 0;
	for (int i = 0; i < enemyCount; i++)
	{
		Enemy* enemy = enemyManager.GetEnemy(i);
		// ���ꂼ��̃G�l�~�[�̋�������
		XMVECTOR PosPlayer = XMLoadFloat3(&GetPosition());
		XMVECTOR PosEnemy = XMLoadFloat3(&enemy->GetPosition());

		XMVECTOR DistVec = XMVectorSubtract(PosEnemy, PosPlayer);
		float dist = XMVectorGetX(XMVector3Length(DistVec));

		enemyDist[enemy] = dist; //�e�G�l�~�[�Ƃ̋��������X�V���ɋL�^����
		if (dist < playerVSenemyJudgeDist[(int)EnemySearch::Attack])
		{
			enemySearch[enemy] = EnemySearch::Attack;
		}
		else if (dist < playerVSenemyJudgeDist[(int)EnemySearch::Find])
		{
			enemySearch[enemy] = EnemySearch::Find;
		}
		else
		{
			enemySearch[enemy] = EnemySearch::None;
			noneEnemy++;
			if (i == (enemyCount - 1) && enemyCount == noneEnemy)
			{
				currentEnemySearch = EnemySearch::None;
			}
			continue;
		}

		/***********************/
		if (dist < nearestDist) 
		{
			// secondEnemy�̓o�^
			secondDist = nearestDist;
			secondDistEnemyVec = nearestVec;

			// �ŋ߃G�l�~�[�̓o�^
			nearestEnemy = enemy;
			nearestDist = dist;
			currentEnemySearch = enemySearch[enemy];
			XMStoreFloat3(&nearestVec, DistVec);
		}
	}
}

//�W�����v����
void Player::UpdateJumpState(float elapsedTime)
{
	// �U�����̏ꍇ�̓W�����v�����Ȃ�
	if (isAttacking) return;

	const float firstJumpSpeed = 150.0f;
	const float secondJumpSpeed = 15.0f;
	const float MaxJumpSpeed = 17.5f;

	switch (jumpTrg)
	{
	case JumpState::CanJump:
		// �����Ă���Ԃ̏���
		if (InputButton(Player::InputState::Jump))
		{
			velocity.y += firstJumpSpeed * elapsedTime;
			// �w������x�܂ł���������
			if (velocity.y > MaxJumpSpeed)
			{
				jumpTrg = JumpState::CanDoubleJump;
			}
		}
		// ��񗣂�����
		else if (InputButtonUp(Player::InputState::Jump))
		{
			jumpTrg = JumpState::CanDoubleJump;
		}
		break;

	case JumpState::CanDoubleJump:
		// 2�i�ڃW�����v�͍������ߕs��
		if (InputButtonDown(Player::InputState::Jump))
		{
			if (velocity.y > 0) velocity.y += secondJumpSpeed;
			else				velocity.y = secondJumpSpeed;

			jumpTrg = JumpState::CannotJump;
		}
		// ��i�ڃW�����v���̍U���{�^��
		else if (InputAttackFromJump(elapsedTime))
		{
			jumpTrg = JumpState::CannotJump;
		}

		//break;
		[[fallthrough]];
	case JumpState::CannotJump:

		// �W�����v�\��Ԃ̎��̂ݒʂ�Ȃ�
		// ���n��(�n�ʂɗ����Ă��鎞�͏펞����)
		if (isGround)
		{
			// ���n���ɉ������ς̏ꍇ�͏�������Ȃ��悤�ɂ���
			if (InputButton(Player::InputState::Jump))
			{
				jumpTrg = JumpState::CannotJump;
			}
			// ������Ă��Ȃ����͒n�ʂɂ���̂ŃW�����v�\��Ԃɂ���
			else
			{
				jumpTrg = JumpState::CanJump;
			}
		}
		break;
	}
}

//�`��
void Player::ShadowRender(const RenderContext& rc, ShadowMap* shadowMap)
{
	shadowMap->Draw(rc, model.get());
}
//�`��
void Player::Render(const RenderContext& rc, ModelShader* shader)
{
	shader->Draw(rc, model.get());
}

// �U���̋O�Օ`��
void Player::PrimitiveRender(const RenderContext& rc)
{
	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();

	// �|���S���`��
	primitiveRenderer->Render(rc.deviceContext, rc.camera->GetView(), rc.camera->GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

// HP�`��
void Player::RenderHPBar(ID3D11DeviceContext* dc, Sprite* gauge, FontSprite* font) const
{
	const float frameExpansion = 6;
	Graphics& graphics = Graphics::Instance();
	const float screenWidth = static_cast<float>(graphics.GetScreenWidth());

	// ���O�\��
	font->Textout(dc, characterName, 200, hpGuage_Y - 7.0f, 1.0f, { -10, 10, 0 }, 12, 16, 32, 32, 16, 16, 0, nameColor);
	// Lv�\��
	std::string levelStr = "Lv:" + std::to_string(currentLevel);
	font->Textout(dc, levelStr, 800, hpGuage_Y + 12.0f, 1.0f, { -10, 10, 0 }, 12, 16, 32, 32, 16, 16, 0, nameColor);
	// HP�\��
	std::string hpStr = std::to_string(GetHealth()) + '/' + std::to_string(GetMaxHealth());
	font->Textout(dc, hpStr, 900, hpGuage_Y + 12.0f, 1.0f, { -10, 10, 0 }, 12, 16, 32, 32, 16, 16, 0, nameColor);

	//�Q�[�W�`��(���n)
	gauge->Render(dc,
		(screenWidth / 2) - (hpGuageWidth / 2),
		hpGuage_Y,
		0,
		hpGuageWidth + frameExpansion,
		hpGuageHeight + frameExpansion,
		0, 0,
		static_cast<float>(gauge->GetTextureWidth()),
		static_cast<float>(gauge->GetTextureHeight()),
		0.0f,
		{ 0.3f, 0.3f, 0.3f, 0.8f }
	);
	//�Q�[�W�`��
	gauge->Render(dc,
		(screenWidth / 2) - (hpGuageWidth / 2) + frameExpansion / 2,
		hpGuage_Y + frameExpansion / 2,
		0,
		hpGuageWidth * (GetHealthRate() / 100.0f),
		hpGuageHeight,
		0, 0,
		static_cast<float>(gauge->GetTextureWidth()),
		static_cast<float>(gauge->GetTextureHeight()),
		0.0f,
		{ GetHpWorning() ? 0.8f : 0.2f, GetHpWorning() ? 0.2f : 0.8f , 0.2f, 1.0f}
	);
}

// �L�����N�^�[���O�`��
void Player::RenderCharacterOverHead(const RenderContext& rc, FontSprite* font, Sprite* message)
{
	const DirectX::XMFLOAT4X4& view = rc.view;
	const DirectX::XMFLOAT4X4& projection = rc.projection;

	//�r���[�|�[�g
	D3D11_VIEWPORT viewport;
	UINT numViewports = 1;
	rc.deviceContext->RSGetViewports(&numViewports, &viewport);

	//�ϊ��s��
	XMMATRIX View = XMLoadFloat4x4(&view);
	XMMATRIX Projection = XMLoadFloat4x4(&projection);
	XMMATRIX World = XMMatrixIdentity();

	//Player����̃��[���h���W
	XMFLOAT3 worldPosition = GetPosition();
	worldPosition.y += GetHeight() + 0.4f;
	XMVECTOR WorldPosition = XMLoadFloat3(&worldPosition);

	//���[���h����X�N���[���ւ̕ϊ�
	XMVECTOR ScreenPosition = XMVector3Project(
		WorldPosition,
		viewport.TopLeftX,
		viewport.TopLeftY,
		viewport.Width,
		viewport.Height,
		viewport.MinDepth,
		viewport.MaxDepth,
		Projection,
		View,
		World
	);

	XMFLOAT3 screenPosition;
	XMStoreFloat3(&screenPosition, ScreenPosition);
	//�J�����̔w��ɂ��邩�A���炩�ɗ���Ă���Ȃ�`�悵�Ȃ�
	if (screenPosition.z > 0.0f && screenPosition.z < 1.0f)
	{
		// ���O
		font->Textout(rc.deviceContext, characterName,
			0,
			screenPosition.y,
			0,
			{ screenPosition.x - 12 * 5, 0, 0 },
			12, 16,
			32, 32, 16, 16, 0, nameColor);

		// ���b�Z�[�W
		if (messageNumber == 0 || messageNumber == 1)
		{
			const DirectX::XMFLOAT2 spriteSize = { 600.0f,100.f };
			messageYTimer += 0.2f;
			message->Render(rc.deviceContext, { screenPosition.x - 60, screenPosition.y - 5.0f - messageYTimer, 0 }, { 180, 30 }, { spriteSize.x, spriteSize.y * messageNumber }, spriteSize, 0, { 1, 1, 1, 1 });
			
			if (messageYTimer > 10.0f)
			{
				enableShowMessage[messageNumber] = false;
				messageNumber = -1;
			}
		}
		else if (enableShowMessage[0])
		{
			messageNumber = 0;
			messageYTimer = 0.0;
		}
		else if (enableShowMessage[1])
		{
			messageNumber = 1;
			messageYTimer = 0.0;
		}
		else
		{
			for (int i = SC_INT(MessageNotification::Attack); i < SC_INT(MessageNotification::MaxCount); i++)
			{
				if (enableShowMessage[i])
				{
					const DirectX::XMFLOAT2 spriteSize = { 600.0f,100.f };
					message->Render(rc.deviceContext, { screenPosition.x - 60, screenPosition.y - 5.0f, 0 }, { 180, 30 }, { spriteSize.x, spriteSize.y * i }, spriteSize, 0, { 1, 1, 1, 1 });
					break;
				}
			}
		}
	}
}

void Player::RenderHaveArms(ID3D11DeviceContext* dc, Sprite* frame, Sprite* arm)
{
	const DirectX::XMFLOAT2 spriteSize = { 300.0f,300.f };

	//HaveArmFrame
	for (int i = 0; i < HaveArms.size(); i++)
	{
		float spriteOffset_x = 0;
		if (i == SC_INT(CurrentUseArm)) spriteOffset_x = spriteSize.x;
		else spriteOffset_x = HaveArms[SC_AT(i)] ? spriteSize.x * 2 : 0;

		frame->Render(dc, { 1000.0f + 65 * i, hpGuage_Y - 10.0f, 0.0f }, { 70, 70 }, { spriteOffset_x, spriteSize.y }, spriteSize, 0, { 1, 1, 1, 1 });
	}
	//HaveArm
	for (int i = 0; i < HaveArms.size(); i++)
	{
		float spriteOffset_x = spriteSize.x * (i + 1);
		float spriteOffset_y = 0;
		//if (SC_AT(i) != CurrentUseArm && SC_AT(i) == GetNextArm()) {
		//	spriteOffset_y = spriteSize.y;
		//}

		float color_a = HaveArms[SC_AT(i)] ? 1.0f : 0.3f;
		float color_rb = (SC_AT(i) == CurrentUseArm) ? 0.4f : 1.0f;
		arm->Render(dc, { 1013.0f + 65 * i, hpGuage_Y + 2.0f, 0.0f }, { 45, 45 }, { spriteOffset_x, spriteOffset_y }, spriteSize, 0, { color_rb, 1, color_rb, color_a });
	}
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
		}
			
		ImGui::Checkbox("attacking", &isAttackJudge);

		ImGui::End();
	}
}

//���n�������ɌĂ΂��
void Player::OnLanding(float elapsedTime)
{
	if (isAttacking) //�U����(��ɃW�����v�U����)
	{
		// ���n���Ă����͉��������Ȃ����߂����ŏ����������Ȃ�
		// �eStateUpdate�ɂăA�j���[�V�����I�����IdleState�֑J�ڂ���
	}
	else if (InputMove(elapsedTime))
	{
		ChangeState(State::Run);
	}
	else {
		ChangeState(State::JumpEnd);
	}
}

void Player::OnDamaged()
{
	ChangeState(State::Damage);
}

void Player::OnDead()
{
	ChangeState(State::Dead);
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
void Player::RenderTrail() const
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

			XMFLOAT3 Position[2] = {};
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

//�������͍X�V
void Player::UpdateVerticalVelocity(float elapsedFrame)
{
	auto state = static_cast<Player::State>(stateMachine->GetStateNumber());

	// �W�����v�X�s�A�[�U���̂ݕʂ̉������������g�p����
	if (state == State::AttackSpearJump)
	{
		if (velocity.y > 0)
		{
			velocity.y = 0;
		}
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
	XMFLOAT3 moveVec = GetMoveVec();

	//�ړ�����
	Move(moveVec.x, moveVec.z, moveSpeed);
	//���񏈗�
	Turn(elapsedTime, moveVec.x, moveVec.z, turnSpeed);

	//�i�s�x�N�g�����[���x�N�g���łȂ��ꍇ�͓��͂��ꂽ
	return moveVec.x != 0 || moveVec.y != 0 || moveVec.z != 0;
}

// �U�����͏���
bool Player::InputAttackFromNoneAttack()
{
	// ������Ă��Ȃ�����return
	if (!InputButtonDown(Player::InputState::Attack)) return false;
	enableSpecialAttack = true;

	if (targetPlayer->enableSpecialAttack
		&& targetPlayer->CurrentUseArm == this->CurrentUseArm
		) {
		switch (CurrentUseArm)
		{
		case Player::AttackType::Hammer:
			ChangeState(State::AttackHammerJump);
			break;
		case Player::AttackType::Spear:
			ChangeState(State::AttackSpearJump);
			break;
		case Player::AttackType::Sword:
			ChangeState(State::AttackSwordJump);
			break;
		}
	}
	else
	{
		switch (CurrentUseArm)
		{
		case Player::AttackType::Hammer:
			ChangeState(State::AttackHammer1);
			break;
		case Player::AttackType::Spear:
			ChangeState(State::AttackSpear1);
			break;
		case Player::AttackType::Sword:
			ChangeState(State::AttackSword1);
			break;
		}
	}
	return true;
}
bool Player::InputAttackFromJump(float elapsedTime)
{
	// ������Ă��Ȃ�����return
	if (!InputButtonDown(Player::InputState::Attack)) return false;

	switch (CurrentUseArm)
	{
	case Player::AttackType::Hammer:
		ChangeState(State::AttackHammerJump);
		break;
	case Player::AttackType::Spear:
		ChangeState(State::AttackSpearJump);
		break;
	case Player::AttackType::Sword:
		ChangeState(State::AttackSwordJump);
		break;
	}

	return true;
}

// ���̑I�𕐊�擾
Player::AttackType Player::GetNextArm()
{
	// ���݂̎��̔ԍ��̕����I������
	for (int i = SC_INT(CurrentUseArm);;)
	{
		// ��]
		if (i == SC_INT(AttackType::MaxCount) - 1) i = 0;
		else i++;
		// �S����������break
		if (i == SC_INT(CurrentUseArm)) break;

		// �������Ȃ�continue
		if (!HaveArms[SC_AT(i)]) continue;

		// ���ɏ������Ă�����̂�I������
		return SC_AT(i);
	}
	// ���݂̕�������̂܂ܕԂ�
	return CurrentUseArm;
}

// �A�C�e���Q�b�g
void Player::AddHaveArm(Player::AttackType arm/* = AttackType::None*/)
{
	// �w�肳��Ă����炻���ݒ肷��
	if (arm != AttackType::None) {
		HaveArms[arm] = true;
		return;
	}

	// ���ݎ����Ă��Ȃ����탊�X�g
	std::unordered_map<AttackType,bool> remainArm = HaveArms;
	for (const auto& arm : HaveArms)
	{
		if (HaveArms[arm.first])
		{
			remainArm.erase(arm.first);
		}
	}

	// �S�Ď����Ă���ꍇ��return
	if (remainArm.size() == 0) return;

	// ����
	int num = rand() % remainArm.size();
	int i = 0;
	// �l�����镐����m�肷��
	for (const auto& arm : remainArm)
	{
		if (i == num)
		{
			HaveArms[arm.first] = true;
			return;
		}
		i++;
	}
}

//�񕜑J�ڊm�F����
bool Player::EnableRecoverTransition()
{
	// 20%�ȏ��false
	if (!targetPlayer->GetHpWorning()) return false;

	XMVECTOR posPlayerthis = XMLoadFloat3(&GetPosition());
	XMVECTOR posPlayertarget = XMLoadFloat3(&targetPlayer->GetPosition());
	float distSq = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(posPlayerthis, posPlayertarget)));
	// �߂��������false
	if (distSq > 3.0f * 3.0f) return false;

	// true�őJ�ډ\
	return true;
}

// �ߋ����U�����̊p�x����
void Player::ForceTurnByAttack(float elapsedTime)
{
	// �G�̔������ɐi�ޕ�������������
	if (enemySearch[nearestEnemy] >= EnemySearch::Find)
	{
		//���񏈗�
		Turn(elapsedTime, nearestVec.x, nearestVec.z, turnSpeed);
	}
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
	for (int i = 0; i < enemyManager.GetEnemyCount(); i++)
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
			// �X�y�V�����Zfalse
			enableSpecialAttack = false;

			// �U�����肵�Ȃ��ꍇ��return
			if (attackingEnemyNumber == i && !isAttackJudge) return;

			int damage = arm.damage + static_cast<int>(currentLevel * 0.2f);
			// �_���[�W��^���Ȃ��ꍇ��return
			if (!enemy->ApplyDamage(damage, 0, this, this == &Player1P::Instance() ? 0 : 1)) return;

			currentAttackEnemy = enemy;
			attackingEnemyNumber = i;
			isAttackJudge = false;
			allDamage += damage;
			// �q�b�g�G�t�F�N�g�Đ�
			{
				outPosition.y += enemy->GetHeight() * enemy->GetEffectOffset_Y();
				PlayEffect(EffectNumber::Hit, outPosition);
			}

			// ������΂���4��ڈȏ�
			if (attackCount >= 4)
			{
				// ������΂��U��
				const float power = 13.0f; //���̐����̗�
				const XMFLOAT3& ep = enemy->GetPosition();

				// �m�b�N�o�b�N�����̎Z�o
				float vx = ep.x - enemy->GetCurrentAttacker()->position.x;
				float vz = ep.z - enemy->GetCurrentAttacker()->position.z;
				float lengthXZ = sqrtf(vx * vx + vz * vz);
				vx /= lengthXZ;
				vz /= lengthXZ;

				// �m�b�N�o�b�N
				enemy->AddImpulse({ power * vx , power * 0.8f, power * vz });
			}
		}
		else if(attackingEnemyNumber == i)//�U�����̃G�l�~�[�ƈ�U�U�����O�ꂽ���A���񓖂��������ɔ�����s��
		{
			isAttackJudge = true;
		}
	}
}

//�f�o�b�O�v���~�e�B�u�`��
void Player::DrawDebugPrimitive() const
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
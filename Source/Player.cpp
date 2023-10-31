#include "Player.h"
#include "Graphics/Graphics.h"
#include "Graphics/PrimitiveRenderer.h"
#include "Input/Input.h"
#include "SceneManager.h"
#include "SceneLoading.h"
#include "SceneGame.h"
#include "SceneClear.h"
#include "EnemyManager.h"

#include "Stage.h"
#include "imgui.h"

static Player* instance = nullptr;

//�C���X�^���X�擾
Player& Player::Instance()
{
	return *instance;
}

//�R���X�g���N�^
Player::Player()
{
	//�C���X�^���X�|�C���^�ݒ�
	instance = this;

	ID3D11Device* device = Graphics::Instance().GetDevice();
	//�v���C���[���f���ǂݍ���
	model = std::make_unique<Model>(device, "Data/Model/SD-UnityChan/UnityChan.fbx", 0.02f);
	//model = std::make_unique<Model>(device, "Data/Model/Enemy/red.fbx", 0.02f);

	position = { -7,5,-66 };
	health = 100;
	maxHealth = 100;

	//�ҋ@�X�e�[�g�֑J��
	TransitionIdleState();

	//�q�b�g�G�t�F�N�g�ǂݍ���
	hitEffect = std::make_unique<Effect>("Data/Effect/Hit.efk");
}

Player::~Player()
{
}

//�X�V
void Player::Update(float elapsedTime, int remine)
{
	// �z��Y����
	ShiftTrailPositions();

	// �X�e�[�g���ɒ��ŏ�������
	UpdateEachState(elapsedTime);

	// ���̋O�Օ`��X�V����
	RenderTrail();

	// �U��������Ȃ���΃W�����v����
	if (Atype == AttackType::None) UpdateJump(elapsedTime);

	//�v���C���[�ƃG�l�~�[�Ƃ̏Փˏ���
	CollisionPlayerVsEnemies();

	//���͏����X�V
	UpdateVelocity(elapsedTime);

	//�I�u�W�F�N�g�s��X�V
	UpdateTransform();

	//���f���A�j���[�V�����X�V����
	model->UpdateAnimation(elapsedTime);

	//���f���s��X�V
	model->UpdateTransform(transform);
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
	
	//rc.deviceContext->OMSetBlendState(renderState->GetBlendState(BlendState::Transparency), blendFactor, sampleMask);
	
#if 1

	//�f�o�b�O���j���[�`��
	DebugMenu();
#endif
}

// �U���̋O�Օ`��
void Player::PrimitiveRender(const RenderContext& rc)
{
	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();

	// �|���S���`��
	primitiveRenderer->Render(rc.deviceContext, rc.camera->GetView(), rc.camera->GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}
// �U���̋O�Օ`��
void Player::HPBarRender(const RenderContext& rc, Sprite* gauge)
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	//HP�Q�[�W�̒���
	const float guageWidth = 700.0f;
	const float guageHeight = 15.0f;

	float healthRate = GetHealth() / static_cast<float>(GetMaxHealth());
	int frameExpansion = 6;
	Graphics& graphics = Graphics::Instance();
	float screenWidth = static_cast<float>(graphics.GetScreenWidth());

	//�Q�[�W�`��(���n)
	gauge->Render(dc,
		(screenWidth / 2) - (guageWidth / 2),
		525,
		0,
		guageWidth + frameExpansion,
		guageHeight + frameExpansion,
		0, 0,
		static_cast<float>(gauge->GetTextureWidth()),
		static_cast<float>(gauge->GetTextureHeight()),
		0.0f,
		0.3f, 0.3f, 0.3f, 0.8f
	);
	//�Q�[�W�`��
	gauge->Render(dc,
		(screenWidth / 2) - (guageWidth / 2) + frameExpansion/2,
		525.0f + frameExpansion/2,
		0,
		guageWidth * healthRate,
		guageHeight,
		0, 0,
		static_cast<float>(gauge->GetTextureWidth()),
		static_cast<float>(gauge->GetTextureHeight()),
		0.0f,
		0.2f, 0.8f, 0.2f, 1.0f
	);
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
			angle.x = XMConvertToRadians(a.x);
			angle.y = XMConvertToRadians(a.y);
			angle.z = XMConvertToRadians(a.z);

			//�X�P�[��
			ImGui::DragFloat3("Scale", &scale.x, 0.01f);
		}

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
	else if (InputMove(elapsedTime)) TransitionRunState();
	else TransitionJumpEndState();
}

void Player::OnDamaged()
{
	isDamaged = true;
}

void Player::OnDead()
{
	//if(Player::GetState() != Player::State::Dead)
	//	TransitionDamageState();
}

// �e�X�e�[�W���Ƃ̍X�V����
void Player::UpdateEachState(float elapsedTime)
{
	switch (state)
	{
	// �ҋ@�X�e�[�g
	case State::Idle:
		// �ړ����͏���
		if (InputMove(elapsedTime)) TransitionIdleToRunState();
		// �W�����v���͏���
		if (InputJumpButton()) TransitionJumpStartState();
		// �U������
		InputAttackFromNoneAttack();
		break;

	case State::IdleToRun:
		InputMove(elapsedTime);
		// �A�j���[�V�����I����
		if (!model->IsPlayAnimation()) TransitionRunState();
		break;

	case State::Run:
		// �ړ����͏���
		if (!InputMove(elapsedTime)) TransitionIdleState();
		// �W�����v���͏���
		if (InputJumpButton()) TransitionJumpStartState();
		// �U������
		InputAttackFromNoneAttack();
		break;

	// ���ݖ��g�p
	case State::RunToIdle:
		// �A�j���[�V�����I����
		if (!model->IsPlayAnimation()) TransitionIdleState();
		break;

	case State::JumpStart:
		InputMove(elapsedTime);
		// ����ɃW�����v���͎��̏���
		if (InputJumpButton()) TransitionJumpAirState();
		// �A�j���[�V�����I����
		if (!model->IsPlayAnimation()) TransitionJumpLoopState();
		break;

	case State::JumpLoop:
		InputMove(elapsedTime);
		// ����ɃW�����v���͎��̏���
		if (InputJumpButton()) TransitionJumpAirState();
		// �W�����v���̍U��������UpdateJump�ɂčs��
		
		// ���n(JumpEnd)�X�e�[�g�ւ̑J�ڂ͒��n���ɂ����s��Ȃ�
		// �ی��Ŋ��ɒ��n���Ă��鎞�p�ɌĂяo��
		if (isGround) OnLanding(elapsedTime);
		break;

	case State::JumpAir:
		InputMove(elapsedTime);
		// �ی��Ŋ��ɒ��n���Ă��鎞�p�ɌĂяo��
		if (isGround) OnLanding(elapsedTime);
		break;

	case State::JumpEnd:
		// InputMove���Ȃ�OnLanding�ł�����ʂ�Ȃ��\������
		// �A�j���[�V�����I����
		if (InputJumpButton()) TransitionJumpStartState();
		if (!model->IsPlayAnimation()) TransitionIdleState();
		break;

	case State::Damage:
		// �_���[�W�󂯂���~�܂�?
		// �A�j���[�V�����I����
		if (!model->IsPlayAnimation()) TransitionIdleState();
		break;

	case State::Dead:
		// Death�A�j���[�V�����͋N���オ��܂łȂ̂œr���Ŏ~�߂�̂��ǂ�?
		// �A�j���[�V�����I����Idle�ɂƂ肠�����ڍs
		if (!model->IsPlayAnimation()) TransitionIdleState();
		break;
	case State::AttackHammer1:
		Hammer.flag1 = (model->IsPlayAnimation());
		if (Hammer.flag1)
		{
			UpdateArmPositions(model.get(), Hammer);
			CollisionArmsVsEnemies(Hammer);

			//�C�ӂ̃A�j���[�V�����Đ���Ԃł̂ݎ��̍U���Z���o���悤�ɂ���
			float animationTime = model->GetCurrentAnimationSeconds();
			if (InputHammerButton() && animationTime >= 0.5f && animationTime <= 0.8f)
			{
				TransitionAttackHummer2State();
			}
		}
		else TransitionIdleState();
		break;
	case State::AttackHammer2:
		Hammer.flag1 = false;
		Hammer.flag2 = (model->IsPlayAnimation());
		if (Hammer.flag2)
		{
			UpdateArmPositions(model.get(), Hammer);
			CollisionArmsVsEnemies(Hammer);
		}
		else TransitionIdleState();
		break;
	case State::AttackHammerJump:
		Hammer.flagJump = model->IsPlayAnimation();
		if (Hammer.flagJump)
		{
			UpdateArmPositions(model.get(), Hammer);
			CollisionArmsVsEnemies(Hammer);
		}

		if (isMoveAttack)
		{
			velocity.x = sinf(angle.y) * 800 * elapsedTime;
			velocity.z = cosf(angle.y) * 800 * elapsedTime;
		}
		if (!model->IsPlayAnimation())
		{
			isMoveAttack = false;
			TransitionIdleState();
		}
		break;
	case State::AttackSpear1:
		if (model->IsPlayAnimation())
		{
			UpdateArmPositions(model.get(), Spear);
			CollisionArmsVsEnemies(Spear);

			float animationTime = model->GetCurrentAnimationSeconds();
			// ����o���A�j���[�V�����Đ����
			Spear.flag1 = (animationTime >= 0.20f && animationTime <= 0.7f);
			// ���𓥂񒣂�ۂ̑O�i�������ōs�� ���ɉ����i��ł��鎞�͂��̏��������Ȃ�
			if (animationTime < 0.25f && !InputMove(elapsedTime))
			{	
				velocity.x += sinf(angle.y) * 40 * elapsedTime;
				velocity.z += cosf(angle.y) * 40 * elapsedTime;
			}
			else if (Spear.flag1 && InputSpearButton())
			{
				TransitionAttackSpear2State();
			}
		}
		else TransitionIdleState();
		break;
	case State::AttackSpear2:
		Spear.flag1 = false;
		Spear.flag2 = model->IsPlayAnimation();
		if (Spear.flag2)
		{
			UpdateArmPositions(model.get(), Spear);
			CollisionArmsVsEnemies(Spear);

			float animationTime = model->GetCurrentAnimationSeconds();
			if (animationTime >= 0.30f && animationTime <= 0.45f)
			{
				// ���𓥂񒣂�ۂ̑O�i
				velocity.x += sinf(angle.y) * 60 * elapsedTime;
				velocity.z += cosf(angle.y) * 60 * elapsedTime;
			}
			// ����o���A�j���[�V�����Đ����
			if (animationTime >= 0.37f && animationTime <= 0.6f && InputSpearButton())
			{
				TransitionAttackSpear3State();
			}
		}
		else TransitionIdleState();
		break;
	case State::AttackSpear3:
		Spear.flag2 = false;
		Spear.flag3 = model->IsPlayAnimation();
		if (Spear.flag3)
		{
			UpdateArmPositions(model.get(), Spear);
			CollisionArmsVsEnemies(Spear);

			float animationTime = model->GetCurrentAnimationSeconds();
			// ���𓥂񒣂�ۂ̑O�i�������ōs��
			if (animationTime < 0.43f)
			{
				velocity.x += sinf(angle.y) * 39 * elapsedTime;
				velocity.z += cosf(angle.y) * 39 * elapsedTime;
			}
		}
		else TransitionIdleState();
		break;
	case State::AttackSpearJump:
		Spear.flagJump = model->IsPlayAnimation();
		if (Spear.flagJump)
		{
			UpdateArmPositions(model.get(), Spear);
			CollisionArmsVsEnemies(Spear);

			float animationTime = model->GetCurrentAnimationSeconds();
			// ��]���Ȃ���O�������ɓ˂��h���Ă����A�j���[�V����
			if (animationTime >= 0.15f && animationTime <= 0.24f)
			{
				velocity.x += sinf(angle.y) * 300 * elapsedTime;
				velocity.z += cosf(angle.y) * 300 * elapsedTime;
			}
		}
		else TransitionIdleState();
		break;
	case State::AttackSword1:
		Sword.flag1 = model->IsPlayAnimation();
		if (Sword.flag1)
		{
			UpdateArmPositions(model.get(), Sword);
			CollisionArmsVsEnemies(Sword);

			float animationTime = model->GetCurrentAnimationSeconds();
			// ���𓥂񒣂�ۂ̑O�i�������ōs�� ���ɉ����i��ł��鎞�͂��̏��������Ȃ�
			if (animationTime < 0.2f && !InputMove(elapsedTime))
			{
				velocity.x += sinf(angle.y) * 43 * elapsedTime;
				velocity.z += cosf(angle.y) * 43 * elapsedTime;
			}
			// �C�ӂ̃A�j���[�V�����Đ���Ԃł̂ݎ��̍U���Z���o���悤�ɂ���
			else if (InputSwordButton() && animationTime >= 0.2f && animationTime <= 0.6f)
			{
				TransitionAttackSword2State();
			}
			// �����̓����ɍ��킹���O�i ��a������̂ň�U�R�����g
			//else if (animationTime >= 0.50f)
			//{
			//	velocity.x += sinf(angle.y) * 38 * elapsedTime;
			//	velocity.z += cosf(angle.y) * 38 * elapsedTime;
			//}
		}
		else TransitionIdleState();
		break;
	case State::AttackSword2:
		Sword.flag1 = false;
		Sword.flag2 = model->IsPlayAnimation();
		if (Sword.flag2)
		{
			UpdateArmPositions(model.get(), Sword);
			CollisionArmsVsEnemies(Sword);

			float animationTime = model->GetCurrentAnimationSeconds();
			// ���𓥂񒣂�ۂ̑O�i�������ōs��
			if (animationTime < 0.25f)
			{
				velocity.x += sinf(angle.y) * 45 * elapsedTime;
				velocity.z += cosf(angle.y) * 45 * elapsedTime;
			}
			// �C�ӂ̃A�j���[�V�����Đ���Ԃł̂ݎ��̍U���Z���o���悤�ɂ���
			else if (InputSwordButton() && animationTime >= 0.25f && animationTime <= 0.5f)
			{
				TransitionAttackSword3State();
			}
		}
		else TransitionIdleState();
		break;
	case State::AttackSword3:
		Sword.flag2 = false;
		Sword.flag3 = model->IsPlayAnimation();
		if (Sword.flag3)
		{
			UpdateArmPositions(model.get(), Sword);
			CollisionArmsVsEnemies(Sword);

			float animationTime = model->GetCurrentAnimationSeconds();
			// ���𓥂񒣂�ۂ̑O�i�������ōs��
			if (animationTime >= 0.25f && animationTime <= 0.5f)
			{
				velocity.x += sinf(angle.y) * 48 * elapsedTime;
				velocity.z += cosf(angle.y) * 48 * elapsedTime;
				if (animationTime <= 0.4f)
					velocity.y += 150 * elapsedTime;
			}
		}
		else TransitionIdleState();
		break;
	case State::AttackSwordJump:
		Sword.flagJump = model->IsPlayAnimation();
		if (Sword.flagJump)
		{
			UpdateArmPositions(model.get(), Sword);
			CollisionArmsVsEnemies(Sword);
		}

		if (isMoveAttack)
		{
			velocity.x = sinf(angle.y) * 800 * elapsedTime;
			velocity.z = cosf(angle.y) * 800 * elapsedTime;
		}
		if (!model->IsPlayAnimation())
		{
			isMoveAttack = false;
			TransitionIdleState();
		}
		break;
	case State::CliffGrab:
		// �A�j���[�V�����I����Idle�ɂƂ肠�����ڍs
		if (!model->IsPlayAnimation()) TransitionIdleState();
		break;
	}
}

//�W�����v����
void Player::UpdateJump(float elapsedTime)
{
	//�{�^�����͂ŃW�����v
	GamePad& gamePad = Input::Instance().GetGamePad();

	switch (jumpTrg)
	{
	case CanJump:
		// �����Ă���Ԃ̏���
		if (gamePad.GetButton() & GamePad::BTN_A)
		{
			velocity.y += 300 * elapsedTime;
			// �w������x�܂ł���������
			if (velocity.y > jumpSpeed)	jumpTrg = CanDoubleJump;
		}
		// ��񗣂�����
		else if (gamePad.GetButtonUp() & GamePad::BTN_A)
		{
			jumpTrg = CanDoubleJump;
		}
		break;

	case CanDoubleJump:
		// 2�i�ڃW�����v�͍������ߕs��
		if (gamePad.GetButtonDown() & GamePad::BTN_A)
		{
			velocity.y += 20.0f;
			jumpTrg = CannotJump;
		}
		// ��i�ڃW�����v���̍U���{�^��
		else if (InputAttackFromJump(elapsedTime))
		{
			jumpTrg = CannotJump;
		}

		//break;
		// fall through
	case CannotJump:

		// �W�����v�\��Ԃ̎��̂ݒʂ�Ȃ�
		// ���n��(�n�ʂɗ����Ă��鎞�͏펞����)
		if (isGround)
		{
			// ���n���ɉ������ς̏ꍇ�͏�������Ȃ��悤�ɂ���
			if (gamePad.GetButton() & GamePad::BTN_A) jumpTrg = CannotJump;
			// ������Ă��Ȃ����͒n�ʂɂ���̂ŃW�����v�\��Ԃɂ���
			else jumpTrg = CanJump;
		}
		break;
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
	//�W�����v�U�����͏d�͂𖳎�����
	if (state == State::AttackHammerJump || state == State::AttackSwordJump)
	{
		velocity.y = 0;
	}
	
	if (state == State::AttackSpearJump)
	{
		if (velocity.y > 0)	velocity.y = 0;
		velocity.y += gravity * 0.25f * elapsedFrame;
	}
	// �X�s�A�[�U���̂ݕʂ̉������������g�p����
	else velocity.y += gravity * elapsedFrame;
}

// ===========���͏���===========
//�ړ����͏���
bool Player::InputMove(float elapsedTime)
{
	//�i�s�x�N�g�����擾
	XMFLOAT3 moveVec = GetMoveVec();

	//�ړ�����
	Move(moveVec.x, moveVec.z, moveSpeed);
	//���񏈗�
	Turn(elapsedTime, moveVec.x, moveVec.z, turnSpeed);

	//�i�s�x�N�g�����[���x�N�g���łȂ��ꍇ�͓��͂��ꂽ
	return moveVec.x != 0 || moveVec.y != 0 || moveVec.z != 0;
}

// �W�����v�{�^��BTN_A�������ꂽ��
bool Player::InputJumpButton()
{
	GamePad& gamePad = Input::Instance().GetGamePad();
	return gamePad.GetButtonDown() & GamePad::BTN_A;
}
// �U�����͏���
bool Player::InputAttackFromNoneAttack()
{
	if (InputHammerButton())	 TransitionAttackHummer1State();
	else if (InputSwordButton()) TransitionAttackSword1State();
	else if (InputSpearButton()) TransitionAttackSpear1State();
	else return false;

	return true;
}
bool Player::InputAttackFromJump(float elapsedTime)
{
	if (InputHammerButton())	 TransitionAttackHummerJumpState(elapsedTime);
	else if (InputSwordButton()) TransitionAttackSwordJumpState(elapsedTime);
	else if (InputSpearButton()) TransitionAttackSpearJumpState();
	else return false;

	return true;
}

// �n���}�[�U���{�^��BTN_B�������ꂽ��
bool Player::InputHammerButton()
{
	GamePad& gamePad = Input::Instance().GetGamePad();
	return gamePad.GetButtonDown() & GamePad::BTN_B;
}
// �\�[�h�U���{�^��BTN_X�������ꂽ��
bool Player::InputSwordButton()
{
	GamePad& gamePad = Input::Instance().GetGamePad();
	return gamePad.GetButtonDown() & GamePad::BTN_X;
}
// �\�[�h�U���{�^��BTN_Y�������ꂽ��
bool Player::InputSpearButton()
{
	GamePad& gamePad = Input::Instance().GetGamePad();
	return gamePad.GetButtonDown() & GamePad::BTN_Y;
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
			//�_���[�W��^����
			if (enemy->ApplyDamage(1, 0.5f))
			{
				//������΂�
				{
					const float power = 1.5f; //���̐����̗�
					const XMFLOAT3& ep = enemy->GetPosition();

					//�m�b�N�o�b�N�����̎Z�o
					float vx = ep.x - arm.position.x;
					float vz = ep.z - arm.position.z;
					float lengthXZ = sqrtf(vx * vx + vz * vz);
					vx /= lengthXZ;
					vz /= lengthXZ;

					//�m�b�N�o�b�N�͂̒�`
					XMFLOAT3 impluse;
					impluse.x = vx * power;
					impluse.z = vz * power;

					impluse.y = power * 0.5f;	//������ɂ��ł��グ��

					enemy->AddImpulse(impluse);
				}
				//�q�b�g�G�t�F�N�g�Đ�
				{
					DirectX::XMFLOAT3 e = enemy->GetPosition();
					e.y += enemy->GetHeight() * 0.5f;
					hitEffect->Play(e);
				}
			}
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


// ========�J��========
// �ҋ@�X�e�[�g
void Player::TransitionIdleState()
{
	state = State::Idle;
	// �U����͕K���ҋ@�ɖ߂�ׂ����ōU���^�C�v ���Z�b�g���s��
	Atype = AttackType::None;
	model->PlayAnimation(Anim_Idle, true);
}
// �ړ��X�e�[�g
void Player::TransitionRunState()
{
	state = State::Run;
	model->PlayAnimation(Anim_Running, true);
}
// �ҋ@->�ړ��ؑփX�e�[�g
void Player::TransitionIdleToRunState()
{
	state = State::IdleToRun;
	model->PlayAnimation(Anim_IdleToRun, false);
}
// �ړ�->�ҋ@�ؑփX�e�[�g
void Player::TransitionRunToIdleState()	// ���g�p
{
	state = State::RunToIdle;
	model->PlayAnimation(Anim_RunToIdle, false);
}

// �W�����v�X�e�[�g�֑J��
void Player::TransitionJumpStartState()
{
	state = State::JumpStart;
	model->PlayAnimation(Anim_JumpStart, false);
}
void Player::TransitionJumpLoopState()
{
	state = State::JumpLoop;
	model->PlayAnimation(Anim_JumpLoop, false);
}
void Player::TransitionJumpAirState()
{
	state = State::JumpAir;
	model->PlayAnimation(Anim_JumpAir, false);
}
void Player::TransitionJumpEndState()
{
	state = State::JumpEnd;
	model->PlayAnimation(Anim_JumpEnd, false);
}
// �_���[�W�X�e�[�g
void Player::TransitionDamageState()
{
	state = State::Damage;
	model->PlayAnimation(Anim_Damage, false);
}
// ���S�X�e�[�g
void Player::TransitionDeadState()
{
	state = State::Dead;
	model->PlayAnimation(Anim_Death, false);
}
// �U���X�e�[�g
void Player::TransitionAttackHummer1State()
{
	state = State::AttackHammer1;
	Atype = AttackType::Hammer;
	model->PlayAnimation(Anim_AttackHammer1, false);
}
void Player::TransitionAttackHummer2State()
{
	state = State::AttackHammer2;
	model->PlayAnimation(Anim_AttackHammer2, false);
}
void Player::TransitionAttackHummerJumpState(float elapsedTime)
{
	state = State::AttackHammerJump;
	Atype = AttackType::Hammer;
	if (InputMove(elapsedTime)) isMoveAttack = true;
	model->PlayAnimation(Anim_AttackHammerJump, false);
}
void Player::TransitionAttackSpear1State()
{
	state = State::AttackSpear1;
	Atype = AttackType::Spear;
	model->PlayAnimation(Anim_AttackSpear1, false);
}
void Player::TransitionAttackSpear2State()
{
	state = State::AttackSpear2;
	model->PlayAnimation(Anim_AttackSpear2, false);
}
void Player::TransitionAttackSpear3State()
{
	state = State::AttackSpear3;
	model->PlayAnimation(Anim_AttackSpear3, false);
}
void Player::TransitionAttackSpearJumpState()
{
	state = State::AttackSpearJump;
	Atype = AttackType::Spear;
	model->PlayAnimation(Anim_AttackSpearJump, false);
}
void Player::TransitionAttackSword1State()
{
	state = State::AttackSword1;
	Atype = AttackType::Sword;
	model->PlayAnimation(Anim_AttackSword1, false);
}
void Player::TransitionAttackSword2State()
{
	state = State::AttackSword2;
	model->PlayAnimation(Anim_AttackSword2, false);
}
void Player::TransitionAttackSword3State()
{
	state = State::AttackSword3;
	model->PlayAnimation(Anim_AttackSword3, false);
}
void Player::TransitionAttackSwordJumpState(float elapsedTime)
{
	state = State::AttackSwordJump;
	Atype = AttackType::Sword;
	if (InputMove(elapsedTime)) isMoveAttack = true;
	model->PlayAnimation(Anim_AttackSwordJump, false);
}
void Player::TransitionCliffGrabState()
{
	state = State::CliffGrab;
	model->PlayAnimation(Anim_CliffGrab, false);
}

//�X�e�B�b�N���͒l����ړ��x�N�g�����擾
XMFLOAT3 Player::GetMoveVec() const
{
	//���͏����擾
	GamePad& gamePad = Input::Instance().GetGamePad();
	float ax = gamePad.GetAxisLX();
	float ay = gamePad.GetAxisLY();

	//�J���������ƃX�e�B�b�N�̓��͒l�ɂ���Đi�s�������v�Z����
	Camera& camera = Camera::Instance();
	const DirectX::XMFLOAT3& cameraRight = camera.GetRight();
	const DirectX::XMFLOAT3& cameraFront = camera.GetFront();

	//�ړ��x�N�g����XZ���ʂɐ����ȃx�N�g���ɂȂ�悤�ɂ���

	//�J�����E�����x�N�g����XZ�P�ʃx�N�g���ɕϊ�
	float cameraRightX = cameraRight.x;
	float cameraRightZ = cameraRight.z;
	float cameraRightLength = sqrtf(cameraRightX * cameraRightX + cameraRightZ * cameraRightZ);
	if (cameraRightLength > 0.0f)
	{
		//�P�ʃx�N�g����
		cameraRightX /= cameraRightLength;
		cameraRightZ /= cameraRightLength;
	}

	//�J�����O�����x�N�g����XZ�P�ʃx�N�g���ɕϊ�
	float cameraFrontX = cameraFront.x;
	float cameraFrontZ = cameraFront.z;
	float cameraFrontLength = sqrtf(cameraFrontX * cameraFrontX + cameraFrontZ * cameraFrontZ);
	if (cameraFrontLength > 0.0f)
	{
		//�P�ʃx�N�g����
		cameraFrontX /= cameraFrontLength;
		cameraFrontZ /= cameraFrontLength;
	}

	//�X�e�B�b�N�̐������͒l���J�����E�����ɔ��f���A
	//�X�e�B�b�N�̐������͒l���J�����O�����ɔ��f���A�i�s�x�N�g�����v�Z����
	XMFLOAT3 vec;
	vec.x = cameraFrontX * ay + cameraRightX * ax;
	vec.z = cameraFrontZ * ay + cameraRightZ * ax;

	//Y�������ɂ͈ړ����Ȃ�
	vec.y = 0.0f;

	return vec;
}
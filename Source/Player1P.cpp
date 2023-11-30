#include <map>

#include "Player1P.h"
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

static Player1P* instance = nullptr;

//�C���X�^���X�擾
Player1P& Player1P::Instance()
{
	return *instance;
}

//�R���X�g���N�^
Player1P::Player1P()
{
	//�C���X�^���X�|�C���^�ݒ�
	instance = this;

	ID3D11Device* device = Graphics::Instance().GetDevice();
	//�v���C���[���f���ǂݍ���
	model = std::make_unique<Model>(device, "Data/Model/SD-UnityChan/UnityChan.fbx", 0.02f);
	//model = std::make_unique<Model>(device, "Data/Model/Enemy/red.fbx", 0.02f);

	//������
	enemySearch.clear();
	EnemyManager& enemyManager = EnemyManager::Instance();
	int enemyCount = enemyManager.GetEnemyCount();//�S�Ă̓G�Ƒ�������ŏՓˏ���
	for (int i = 0; i < enemyCount; i++)
	{
		enemySearch[enemyManager.GetEnemy(i)] = EnemySearch::None;
	}
	ESState = EnemySearch::None;

	position = { -7,5,-66 };
	health = 100;
	maxHealth = 100;

	//�ҋ@�X�e�[�g�֑J��
	TransitionIdleState();

	//�q�b�g�G�t�F�N�g�ǂݍ���
	hitEffect = std::make_unique<Effect>("Data/Effect/Hit.efk");
}

Player1P::~Player1P()
{
}

//�X�V
void Player1P::Update(float elapsedTime)
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
			if (i == (enemyCount - 1) && enemyCount == noneEnemy) ESState = EnemySearch::None;
			continue;
		}

		/***********************/

		if (dist < nearestDist) //�ŋ߃G�l�~�[�̓o�^
		{
			nearestEnemy = enemy;
			nearestDist = dist;
			ESState = enemySearch[enemy];
			XMStoreFloat3(&nearestVec, DistVec);
		}
	}

	//for (auto itr = enemySearch.begin(); itr != enemySearch.end(); ++itr) 
	//{
	//	if (static_cast<int>(itr->second) <= static_cast<int>(EnemySearch::Find))
	//	{



	//	}


	//}


	// �z��Y����
	//ShiftTrailPositions();

	// �X�e�[�g���ɒ��ŏ�������
	UpdateEachState(elapsedTime);

	// ���̋O�Օ`��X�V����
	//RenderTrail();

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
void Player1P::ShadowRender(const RenderContext& rc, ShadowMap* shadowMap)
{
	shadowMap->Draw(rc, model.get());
}
//�`��
void Player1P::Render(const RenderContext& rc, ModelShader* shader)
{
	shader->Draw(rc, model.get());
	
#ifdef _DEBUG
	//�f�o�b�O���j���[�`��
	DebugMenu();
#endif
}

// �U���̋O�Օ`��
void Player1P::PrimitiveRender(const RenderContext& rc)
{
	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();

	// �|���S���`��
	primitiveRenderer->Render(rc.deviceContext, rc.camera->GetView(), rc.camera->GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}
// �U���̋O�Օ`��
void Player1P::HPBarRender(const RenderContext& rc, Sprite* gauge)
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
		555.0f,
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
		(screenWidth / 2) - (guageWidth / 2) + frameExpansion / 2,
		555.0f + frameExpansion / 2,
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

void Player1P::DebugMenu()
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

		ImGui::End();
	}
}

//���n�������ɌĂ΂��
void Player1P::OnLanding(float elapsedTime)
{
	if (Atype != AttackType::None) //�U����(��ɃW�����v�U����)
	{
		// ���n���Ă����͉��������Ȃ����߂����ŏ����������Ȃ�
		// �eStateUpdate�ɂăA�j���[�V�����I�����IdleState�֑J�ڂ���
	}
	else if (InputMove(elapsedTime)) TransitionRunState();
	else TransitionJumpEndState();
}

//�W�����v����
void Player1P::UpdateJump(float elapsedTime)
{
	//�{�^�����͂ŃW�����v
	GamePad& gamePad = Input::Instance().GetGamePad();

	switch (jumpTrg)
	{
	case CanJump:
		// �����Ă���Ԃ̏���
		if (gamePad.GetButton() & GamePad::BTN_A)
		{
			velocity.y += 150 * elapsedTime;
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
			if (velocity.y > 0) velocity.y += 15.0f;
			else				velocity.y = 15.0f;
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

// ===========���͏���===========
//�ړ����͏���
bool Player1P::InputMove(float elapsedTime)
{
	XMFLOAT3 moveVec = GetMoveVec();
	XMVECTOR MoveVec = XMLoadFloat3(&moveVec); //�i�s�x�N�g�����擾

	if (XMVectorGetX(XMVector3Length(MoveVec)) != 0 && nearestDist < FLT_MAX)
	{
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

// �W�����v�{�^��BTN_A�������ꂽ��
bool Player1P::InputJumpButtonDown()
{
	GamePad& gamePad = Input::Instance().GetGamePad();
	return gamePad.GetButtonDown() & GamePad::BTN_A;
}
bool Player1P::InputJumpButton()
{
	GamePad& gamePad = Input::Instance().GetGamePad();
	return gamePad.GetButton() & GamePad::BTN_A;
}
bool Player1P::InputJumpButtonUp()
{
	GamePad& gamePad = Input::Instance().GetGamePad();
	return gamePad.GetButtonUp() & GamePad::BTN_A;
}

// �n���}�[�U���{�^��BTN_B�������ꂽ��
bool Player1P::InputHammerButton()
{
	GamePad& gamePad = Input::Instance().GetGamePad();
	return gamePad.GetButtonDown() & GamePad::BTN_B;
}
// �\�[�h�U���{�^��BTN_X�������ꂽ��
bool Player1P::InputSwordButton()
{
	GamePad& gamePad = Input::Instance().GetGamePad();
	return gamePad.GetButtonDown() & GamePad::BTN_X;
}
// �\�[�h�U���{�^��BTN_Y�������ꂽ��
bool Player1P::InputSpearButton()
{
	GamePad& gamePad = Input::Instance().GetGamePad();
	return gamePad.GetButtonDown() & GamePad::BTN_Y;
}

//�X�e�B�b�N���͒l����ړ��x�N�g�����擾
XMFLOAT3 Player1P::GetMoveVec() const
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
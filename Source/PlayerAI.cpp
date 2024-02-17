#include <map>

#include "PlayerAI.h"
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

static PlayerAI* instance = nullptr;

//�C���X�^���X�擾
PlayerAI& PlayerAI::Instance()
{
	return *instance;
}

//�R���X�g���N�^
PlayerAI::PlayerAI()
{
	//�C���X�^���X�|�C���^�ݒ�
	instance = this;

	ID3D11Device* device = Graphics::Instance().GetDevice();
	//�v���C���[���f���ǂݍ���
	model = std::make_unique<Model>(device, "Data/Model/SD-UnityChan/UnityChan.fbx", 0.02f);

	// ������
	Player::Init();

	position = { -7,5,-60 };

	// �{�^���X�e�[�g������
	oldInput = nowInput = nextInput = InputState::None;
	ESState = EnemySearch::None;
}

PlayerAI::~PlayerAI()
{
}

//�X�V
void PlayerAI::Update(float elapsedTime)
{
	EnemyManager& enemyManager = EnemyManager::Instance();
	int enemyCount = enemyManager.GetEnemyCount();

	// �{�^���X�e�[�g������
	oldInput = nowInput;
	nowInput = nextInput;
	nextInput = InputState::None;
	nearestEnemy = nullptr;
	nearestDist = FLT_MAX;
	nearestVec = {};

	//if (enemyCount == 0) ESState = EnemySearch::None;
	if (enemyCount == 0)
	{
		//SceneManager::Instance().ChangeScene(new SceneLoading(new SceneGame(), -255));
	}

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
			enemySearch[enemy] = EnemySearch::Attack;
		else if (dist < playerVSenemyJudgeDist[(int)EnemySearch::Find])
			enemySearch[enemy] = EnemySearch::Find;
		else
			enemySearch[enemy] = EnemySearch::None;

		/***********************/

		if (dist < nearestDist) //�ŋ߃G�l�~�[�̓o�^
		{
			nearestEnemy = enemy;
			nearestDist = dist;
			ESState = enemySearch[enemy];
			XMStoreFloat3(&nearestVec, DistVec);
		}
	}

	if (ESState == EnemySearch::Attack && nowInput != InputState::Sword) //�������łȂ��̂ō����\�[�h�̏ꍇ������
		nextInput = InputState::Sword;

	// �z��Y����
	//ShiftTrailPositions();

	// �X�e�[�g���ɒ��ŏ�������
	//UpdateEachState(elapsedTime);
	stateMachine->Update(elapsedTime);

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

//�W�����v����
void Player::UpdateJump(float elapsedTime)
{
	switch (jumpTrg)
	{
	case JumpState::CanJump:
		// ���������̏���
		if (InputJumpButtonDown())
		{
			velocity.y = 500.0f;
			jumpTrg = JumpState::CanDoubleJump;
		}
		break;

	case JumpState::CanDoubleJump:
		// 2�i�ڃW�����v�͍������ߕs��
		if (InputJumpButtonDown())
		{
			velocity.y += 15.0f;
			jumpTrg = JumpState::CannotJump;
		}
		// ��i�ڃW�����v���̍U���{�^��
		else if (InputAttackFromJump(elapsedTime))
		{
			jumpTrg = JumpState::CannotJump;
		}

		//break;
		// fall through
	case JumpState::CannotJump:

		// �W�����v�\��Ԃ̎��̂ݒʂ�Ȃ�
		// ���n��(�n�ʂɗ����Ă��鎞�͏펞����)
		if (isGround)
		{
			// ���n���ɉ������ς̏ꍇ�͏�������Ȃ��悤�ɂ���
			if (InputJumpButtonDown()) jumpTrg = JumpState::CannotJump;
			// ������Ă��Ȃ����͒n�ʂɂ���̂ŃW�����v�\��Ԃɂ���
			else jumpTrg = JumpState::CanJump;
		}
		break;
	}
}

//�`��
void PlayerAI::ShadowRender(const RenderContext& rc, ShadowMap* shadowMap)
{
	shadowMap->Draw(rc, model.get());
}
//�`��
void PlayerAI::Render(const RenderContext& rc, ModelShader* shader)
{
	shader->Draw(rc, model.get());
}

// �U���̋O�Օ`��
void PlayerAI::PrimitiveRender(const RenderContext& rc)
{
	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();

	// �|���S���`��
	primitiveRenderer->Render(rc.deviceContext, rc.camera->GetView(), rc.camera->GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}
// �U���̋O�Օ`��
void PlayerAI::HPBarRender(const RenderContext& rc, Sprite* gauge)
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	//HP�Q�[�W�̒���
	const float guageWidth = 700.0f;
	const float guageHeight = 15.0f;

	float healthRate = GetHealth() / static_cast<float>(GetMaxHealth());
	bool hpWorning = healthRate < 0.2f;
	int frameExpansion = 6;
	Graphics& graphics = Graphics::Instance();
	float screenWidth = static_cast<float>(graphics.GetScreenWidth());

	//�Q�[�W�`��(���n)
	gauge->Render(dc,
		(screenWidth / 2) - (guageWidth / 2),
		580.0f,
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
		580.0f + frameExpansion / 2,
		0,
		guageWidth * healthRate,
		guageHeight,
		0, 0,
		static_cast<float>(gauge->GetTextureWidth()),
		static_cast<float>(gauge->GetTextureHeight()),
		0.0f,
		hpWorning ? 0.8f : 0.2f, hpWorning ? 0.2f : 0.6f, 0.2f, 1.0f
	);
}


//�W�����v����
void PlayerAI::UpdateJump(float elapsedTime)
{

}

// ===========���͏���===========
//�ړ����͏���
bool PlayerAI::InputMove(float elapsedTime)
{
	XMFLOAT3 moveVec = {};
	XMFLOAT3 playerPos = Player1P::Instance().GetPosition();
	XMFLOAT3 playerAng = Player1P::Instance().GetAngle();
	// ��Ƀv���C���[�̎΂ߌ��ӂ�ɕt������
	playerPos.x -= sinf(playerAng.y-45) * 2;
	playerPos.z -= cosf(playerAng.y-45) * 2;
	playerPos.y = position.y; //Y�����͎��g�̍����ŗǂ�

	XMVECTOR AIto1P = XMVectorSubtract(XMLoadFloat3(&playerPos), XMLoadFloat3(&position));
	if (XMVectorGetX(XMVector3LengthSq(AIto1P)) > 0.2f * 0.2f)
		XMStoreFloat3(&moveVec, AIto1P);

	if (Player1P::Instance().GetESState() >= EnemySearch::Find)
	{
		if (nearestDist < 10.0f) moveVec = nearestVec;
		else XMStoreFloat3(&moveVec, AIto1P);
	}						

	//�ړ�����
	Move(moveVec.x, moveVec.z, moveSpeed);
	//���񏈗�
	Turn(elapsedTime, moveVec.x, moveVec.z, turnSpeed);

	//�i�s�x�N�g�����[���x�N�g���łȂ��ꍇ�͓��͂��ꂽ
	return moveVec.x != 0 || moveVec.y != 0 || moveVec.z != 0;
}

// �W�����v�{�^���������ꂽ��
bool PlayerAI::InputJumpButtonDown()
{
	return InputButtonDown(InputState::Jump);
}
bool PlayerAI::InputJumpButton()
{
	return InputButton(InputState::Jump);
}
bool PlayerAI::InputJumpButtonUp()
{
	return InputButtonUp(InputState::Jump);
}

// �n���}�[�U���{�^���������ꂽ��
bool PlayerAI::InputHammerButton()
{
	return InputButtonDown(InputState::Hammer);
}
// �\�[�h�U���{�^���������ꂽ��
bool PlayerAI::InputSwordButton()
{
	return InputButtonDown(InputState::Sword);
}

// �X�s�A�[�U���{�^���������ꂽ��
bool PlayerAI::InputSpearButton()
{
	return InputButtonDown(InputState::Spear);
}

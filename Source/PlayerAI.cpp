#include "PlayerAI.h"
#include "Player1P.h"

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

	// ������
	Player::Init();

	position = { -7,5,-60 };

	// �{�^���X�e�[�g������
	oldInput = nowInput = nextInput = InputState::None;
	currentEnemySearch = EnemySearch::None;
}

PlayerAI::~PlayerAI()
{
}

//�X�V
void PlayerAI::Update(float elapsedTime)
{
	// �{�^���X�e�[�g������
	oldInput = nowInput;
	nowInput = nextInput;
	nextInput = InputState::None;

	UpdateEnemyDistance(elapsedTime);

	// �U������
	if (currentEnemySearch == EnemySearch::Attack && nowInput != InputState::Sword) //�������łȂ��̂ō����\�[�h�̏ꍇ������
	{
		nextInput = InputState::Sword;
	}

	UpdateUtils(elapsedTime);
}

// HP�`��
void PlayerAI::HPBarRender(const RenderContext& rc, Sprite* gauge)
{
	Player::HPBarRender(rc, gauge, false);
}

// �ړ��x�N�g��
XMFLOAT3 PlayerAI::GetMoveVec() const
{
	XMFLOAT3 moveVec = {};

	XMFLOAT3 playerPos = Player1P::Instance().GetPosition();
	XMFLOAT3 playerAng = Player1P::Instance().GetAngle();
	// ��Ƀv���C���[�̎΂ߌ��ӂ�ɕt������
	playerPos.x -= sinf(playerAng.y - 45) * 2;
	playerPos.z -= cosf(playerAng.y - 45) * 2;
	playerPos.y = position.y; //Y�����͎��g�̍����ŗǂ�

	// �v���C���[�ւ̃x�N�g��
	XMVECTOR AIto1P = XMVectorSubtract(XMLoadFloat3(&playerPos), XMLoadFloat3(&position));
	// ��苗�����ꂽ���A�i�s�x�N�g�����X�V
	if (XMVectorGetX(XMVector3LengthSq(AIto1P)) > 0.2f * 0.2f)
	{
		XMStoreFloat3(&moveVec, AIto1P);
	}

	// �v���C���[�������X�e�[�g��
	if (Player1P::Instance().GetEnemySearch() >= EnemySearch::Find)
	{
		// �ŋ߃G�l�~�[�Ƃ̋������ߋ����̏ꍇ�A�i�s�x�N�g�����X�V
		if (nearestDist < 10.0f) moveVec = nearestVec;
	}

	return moveVec;
}

// ===========���͏���===========
// �{�^������
bool PlayerAI::InputButtonDown(InputState button)
{
	if (oldInput == button) return false;
	if (nowInput == button) return true;
	return false;
}
bool PlayerAI::InputButton(InputState button)
{
	if (nowInput == button) return true;
	return false;
}
bool PlayerAI::InputButtonUp(InputState button)
{
	if (nowInput == button) return false;
	if (oldInput == button) return true;
	return false;
}
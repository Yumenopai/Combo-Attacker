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
	if (currentEnemySearch == EnemySearch::Attack && nowInput != InputState::Attack) //�������łȂ��̂ō����U���̏ꍇ������
	{
		nextInput = InputState::Attack;
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
	const float range = 2.5f;	// �v���C���[�Ɩ����̔��苗��
	XMFLOAT3 moveVec = {};		// �ړ��x�N�g��(return�l)

	XMFLOAT3 playerPos = Player1P::Instance().GetPosition();
	XMFLOAT3 playerAng = Player1P::Instance().GetAngle();

	// �v���C���[�ւ̃x�N�g��
	XMVECTOR AIto1P = XMVectorSubtract(XMLoadFloat3(&playerPos), XMLoadFloat3(&position));

	// ��苗�����痣��Ă���ꍇ
	if (XMVectorGetX(XMVector3LengthSq(AIto1P)) > range * range)
	{
		// �ړ����̓v���C���[�̎΂ߌ��ӂ�ɕt������
		playerPos.x -= sinf(playerAng.y - 45) * 2;
		playerPos.z -= cosf(playerAng.y - 45) * 2;
		playerPos.y = position.y; //Y�����͎��g�̍����ŗǂ�
		// �ڕW�ʒu�ւ̃x�N�g��
		AIto1P = XMVectorSubtract(XMLoadFloat3(&playerPos), XMLoadFloat3(&position));
		// �ړ��x�N�g�����X�V
		XMStoreFloat3(&moveVec, AIto1P);
	}

	// �ŋ߃G�l�~�[�Ƃ̋������ߋ����̏ꍇ�A�i�s�x�N�g�����X�V
	if (nearestDist < 10.0f) moveVec = nearestVec;

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
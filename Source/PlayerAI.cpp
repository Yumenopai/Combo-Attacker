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

	//��������
	InitialArm = AttackType::Hammer;

	// ������
	Player::Init();

	position = { -7,5,-60 };

	characterName = " BUDDY";
	nameColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	//UI
	hpGuage_Y = 620.0f;
	hpColorNormal = { 0.2f, 0.6f , 0.2f, 1.0f };
	hpColorWorning = { 0.6f, 0.2f, 0.2f, 1.0f };

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
	if (targetPlayer == nullptr) {
		targetPlayer = &Player1P::Instance();
	}

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

// �ړ��x�N�g��
XMFLOAT3 PlayerAI::GetMoveVec() const
{
	const float range = 2.5f;	// �v���C���[�Ɩ����̔��苗��
	XMFLOAT3 moveVec = {};		// �ړ��x�N�g��(return�l)

	XMFLOAT3 playerPos = Player1P::Instance().GetPosition();
	XMFLOAT3 playerAng = Player1P::Instance().GetAngle();

	// �v���C���[�ւ̃x�N�g��
	XMVECTOR AIto1P = XMVectorSubtract(XMLoadFloat3(&playerPos), XMLoadFloat3(&position));

	bool readyRecover = targetPlayer->GetHpWorning();

	// ��苗�����痣��Ă���ꍇ or �^�[�Q�b�g�񕜂��K�v�Ȏ�
	if (XMVectorGetX(XMVector3LengthSq(AIto1P)) > range * range || readyRecover)
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
	if (!readyRecover && nearestDist < 10.0f) moveVec = nearestVec;

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

// ����ύX����
void PlayerAI::InputChangeArm(AttackType arm/* = AttackType::None*/)
{
	// ������Ă��Ȃ�����return
	if (!Player1P::Instance().InputButtonDown(Player::InputState::Buddy)) return;

	// �w�肳��Ă����炻���ݒ肷��
	if (arm != AttackType::None) {
		CurrentUseArm = arm;
		return;
	}

	// ���ɏ������Ă�����̂�I������
	CurrentUseArm = GetNextArm();
}

// �^�[�Q�b�g�񕜏���
void PlayerAI::InputRecover()
{
	//�񕜑J�ډ\�Ȃ�X�e�[�g�ڍs
	if (enableRecoverTransition)
	{
		ChangeState(State::Recover);
	}
}
#include "PlayerAI.h"
#include "Player1P.h"
#include "Enemy.h"

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
	InitialArm = AttackType::Spear;

	// ������
	Player::Init();

	position = { -7,5,-60 };
	turnSpeed = XMConvertToRadians(360);

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
	AttackUpdate();
	UpdateUtils(elapsedTime);
}

// �U�����̍X�V����
void PlayerAI::AttackUpdate()
{
	// timerUpdate
	if (waitTimer > 0)
	{
		waitTimer++;
		if (waitTimer > 80)
		{
			waitTimer = 0;
			avoidEnemy = nullptr;
			SetEnableShowMessage(Player::MessageNotification::Attack, false);
		}
	}

	// �U���X�e�[�g�̏ꍇ�̂ݏ�������
	if (currentEnemySearch != EnemySearch::Attack) return;

	// 50�_���[�W�ȏ�^�������A80%�ȏオ���g�̍U�����Ă���
	ranAwayFromPlayer1P = (allDamage > 50
		&& (100 * allDamage / (Player1P::Instance().GetAllDamage() + allDamage)) > 80/*%*/);
	SetEnableShowMessage(Player::MessageNotification::Indifference, ranAwayFromPlayer1P);

	// ���g�̃_���[�W���c��͂�
	ranAwayFromEnemy = GetHpWorning();
	SetEnableShowMessage(Player::MessageNotification::RanAway, ranAwayFromEnemy);

	// �G�_���[�W���c��͂��ŁA1P�̕���l�����܂��̎�
	if (currentAttackEnemy != nullptr
		&& currentAttackEnemy->GetHealthRate() < (maxHealth / 5)
		&& currentAttackEnemy->GetAttackedDamagePersent(0) > 20/*%*/
		&& Player1P::Instance().GetHaveArmCount() <= this->GetHaveArmCount()
		&& waitTimer == 0)
	{
		avoidEnemy = currentAttackEnemy;
		SetEnableShowMessage(Player::MessageNotification::Attack,true);
		waitTimer++;
		return;
	}

	// �U������
	if (!ranAwayFromEnemy 
		&& nearestEnemy != avoidEnemy
		&& nowInput != InputState::Attack) //�������łȂ��̂ō����U���̏ꍇ������
	{
		nextInput = InputState::Attack;
	}
}

// �ړ��x�N�g��
XMFLOAT3 PlayerAI::GetMoveVec() const
{
	const float playerFollowRange = 2.5f;	// �����ɂ��Ă������苗��
	XMFLOAT3 moveVec = {};		// �ړ��x�N�g��(return�l)

	XMFLOAT3 playerPos = Player1P::Instance().GetPosition();
	XMFLOAT3 playerAng = Player1P::Instance().GetAngle();

	// �v���C���[�ւ̃x�N�g��
	XMVECTOR playerVec = XMVectorSubtract(XMLoadFloat3(&playerPos), XMLoadFloat3(&position));
	float playerDist = XMVectorGetX(XMVector3LengthSq(playerVec));

	// �v���C���[����̓���
	if (ranAwayFromPlayer1P)
	{
		// �v���C���[�߂���
		if (playerDist < 10.0f)
		{
			// ��ԋ߂��v���C���[���瓦����悤�ɓ���
			XMStoreFloat3(&moveVec, playerVec);
			moveVec.x = -moveVec.x;
			moveVec.z = -moveVec.z;
		}
		else
		{
			// ��ԋ߂��G�ɏ���Ɉړ�����
			moveVec = nearestVec;
			// 1P���U�����Ă���G��������2�Ԗڂɋ߂����Ɉړ�
			if (nearestEnemy == targetPlayer->GetCurrentAttackEnemy())
			{
				moveVec = secondDistEnemyVec;
			}
		}
		return moveVec;
	}

	// �񕜂Ɍ�����
	bool readyRecover = targetPlayer->GetHpWorning();
	// ��苗�����痣��Ă���ꍇ or �^�[�Q�b�g�񕜂��K�v�Ȏ�
	if (playerDist > playerFollowRange * playerFollowRange || readyRecover)
	{
		// �ړ����̓v���C���[�̎΂ߌ��ӂ�ɕt������
		playerPos.x -= sinf(playerAng.y - 45) * 2;
		playerPos.z -= cosf(playerAng.y - 45) * 2;
		playerPos.y = position.y; //Y�����͎��g�̍����ŗǂ�
		// �ڕW�ʒu�ւ̃x�N�g��
		playerVec = XMVectorSubtract(XMLoadFloat3(&playerPos), XMLoadFloat3(&position));
		// �ړ��x�N�g�����X�V
		XMStoreFloat3(&moveVec, playerVec);

		// �񕜂̏ꍇ�͂����ŕԂ�
		if (readyRecover) return moveVec;
	}

	// �G���瓦����
	if (ranAwayFromEnemy)
	{
		if (nearestDist < 4.0f)
		{
			// ��ԋ߂��G���瓦����悤�ɓ���
			moveVec = nearestVec;
			moveVec.x = -moveVec.x;
			moveVec.z = -moveVec.z;

			XMFLOAT3 playerVecF3 = {};
			XMStoreFloat3(&playerVecF3, playerVec);
			moveVec = { playerVecF3.x + moveVec.x, moveVec.y, playerVecF3.z + moveVec.z };
		}
	}
	// �U����C����G������ꍇ
	else if (avoidEnemy != nullptr && nearestEnemy == avoidEnemy)
	{
		if (secondDist < 10.0f)
		{
			moveVec = secondDistEnemyVec;
		}
	}
	// �ŋ߃G�l�~�[�Ƃ̋������ߋ����̏ꍇ�A�i�s�x�N�g�����X�V
	else if (nearestDist < 10.0f)
	{
		moveVec = nearestVec;
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

// ����ύX����
void PlayerAI::InputChangeArm(AttackType arm/* = AttackType::None*/)
{
	// �w�肳��Ă����炻���ݒ肷��
	if (arm != AttackType::None) {
		CurrentUseArm = arm;
		return;
	}

	// ���g��HP�����₤�����A�{�^����������Ă���Ƃ��̏���
	if (GetHpWorning()) return;
	if (!Player1P::Instance().InputButtonDown(Player::InputState::Buddy)) return;

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
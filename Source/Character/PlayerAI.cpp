#include "PlayerAI.h"
#include "Player1P.h"
#include "Core/Enemy.h"

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
	InitialWeapon = AttackType::Spear;

	// ������
	Player::Init();

	position = ai_initial_position;
	angle = initial_angle;

	turn_speed = ai_turn_speed;

	playerName = ai_Name;
	serialNumber = ai_serial_number;
	nameColor = ai_name_color;
	//UI
	hpGaugePosition_Y = ai_hp_gauge_position_y;

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
		if (waitTimer > ai_wait_timer_max)
		{
			waitTimer = 0;
			lastAvoidEnemy = currentAvoidEnemy;
			currentAvoidEnemy = nullptr;
			SetEnableShowMessage(Player::PlayerMessage::Attack, false);
		}
	}

	// �U���X�e�[�g�̏ꍇ�̂ݏ�������
	if (currentEnemySearch != EnemySearch::Attack) return;

	// 50�_���[�W�ȏ�^�������A80%�ȏオ���g�̍U�����Ă���
	ranAwayFromPlayer1P = (allDamage > ai_ran_away_min_damage
		&& (100 * allDamage / (Player1P::Instance().GetAllDamage() + allDamage)) > ai_ran_away_damage_rate/*%*/); // ���g���^�����S�_���[�W��(%�Ŏ�������100���|����)/�S���ŗ^�����S�_���[�W��
	SetEnableShowMessage(Player::PlayerMessage::Indifference, ranAwayFromPlayer1P);

	// ���g�̃_���[�W���c��͂��Ȃ瓦����
	if (!ranAwayFromEnemy && GetHpWorning())
	{
		ranAwayFromEnemy = true;
		SetEnableShowMessage(Player::PlayerMessage::RanAway, ranAwayFromEnemy);
	}

	// �Ƃǂ߂�1P�ɏ���
	if (currentAttackEnemy != nullptr // ���ݍU����
		&& currentAttackEnemy->GetHealthRate() < ai_enemy_few_remain_damage_rate // �G�̃_���[�W���킸��
		&& currentAttackEnemy->GetAttackedDamagePersent(Player1P::Instance().GetSerialNumber()) > ai_concede_finish_min_damage_rate // 1P���Œ���U�����Ă���
		&& Player1P::Instance().GetHaveWeaponCount() <= this->GetHaveWeaponCount() // 1P�̕��폊������AI�̕��퐔��菭�Ȃ�
		&& lastAvoidEnemy != currentAttackEnemy && waitTimer == 0) // �d�������h�~/�^�C�}�[�������Ă��Ȃ�
	{
		currentAvoidEnemy = currentAttackEnemy;
		SetEnableShowMessage(Player::PlayerMessage::Attack,true);
		waitTimer++;
		return;
	}

	// �U������
	if (!ranAwayFromEnemy 
		&& nearestEnemy != currentAvoidEnemy
		&& nowInput != InputState::Attack) //�������łȂ��̂ō����U���̏ꍇ������
	{
		nextInput = InputState::Attack;
	}
}

// �ړ��x�N�g��
XMFLOAT3 PlayerAI::GetMoveVec() const
{
	XMFLOAT3 moveVec = {};		// �ړ��x�N�g��(return�l)

	XMFLOAT3 playerPos = Player1P::Instance().GetPosition();
	// �v���C���[�ւ̃x�N�g��
	XMVECTOR playerVec = XMVectorSubtract(XMLoadFloat3(&playerPos), XMLoadFloat3(&position));
	float playerDistSq = XMVectorGetX(XMVector3LengthSq(playerVec));

	// �v���C���[����̓���
	if (ranAwayFromPlayer1P)
	{
		// �v���C���[�߂���
		if (playerDistSq < ai_ran_away_from_p1_dist * ai_ran_away_from_p1_dist)
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
	if (playerDistSq > ai_player_follow_dist * ai_player_follow_dist || readyRecover)
	{
		// �ړ����̓v���C���[�̎΂ߌ��ӂ�ɕt������
		XMFLOAT3 playerAng = Player1P::Instance().GetAngle();
		playerPos.x -= sinf(playerAng.y - 45/*��*/) * 2; // 45�x����2�{����
		playerPos.z -= cosf(playerAng.y - 45/*��*/) * 2; // 45�x����2�{����
		playerPos.y = position.y; // Y�����͎��g�̍����ŗǂ�
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
		if (nearestDist < ai_ran_away_from_enemy_dist)
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
	else if (currentAvoidEnemy != nullptr && nearestEnemy == currentAvoidEnemy)
	{
		if (secondDist < ai_go_toward_enemy_dist)
		{
			moveVec = secondDistEnemyVec;
		}
	}
	// �ŋ߃G�l�~�[�Ƃ̋������ߋ����̏ꍇ�A�i�s�x�N�g�����X�V
	else if (nearestDist < ai_go_toward_enemy_dist)
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
void PlayerAI::InputChangeWeapon(AttackType weapon/* = AttackType::None*/)
{
	// �w�肳��Ă����炻���ݒ肷��
	if (weapon != AttackType::None) {
		CurrentUseWeapon = weapon;
		return;
	}

	// ���g��HP�����₤�����A�{�^����������Ă���Ƃ��̏���
	if (GetHpWorning()) return;
	if (!Player1P::Instance().InputButtonDown(Player::InputState::Buddy)) return;

	// ���ɏ������Ă�����̂�I������
	CurrentUseWeapon = GetNextWeapon();
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
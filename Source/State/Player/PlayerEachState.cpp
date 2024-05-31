#include "PlayerEachState.h"
#include "Player.h"
#include "PlayerManager.h"
#include "AnimationTimeStruct.h"

/************************************
	StateIdle : �ҋ@
************************************/

void StateIdle::Init()
{
	player->SetAttackCount(0); // attackCount�̃��Z�b�g
	// �U����͕K���ҋ@�ɖ߂�ׂ����ōU���^�C�v ���Z�b�g���s��
	player->SetAttacking(false);
	player->PlayAnimation(Player::Animation::Idle, true);
}

void StateIdle::Update(float elapsedTime)
{
	// �ړ����͏���
	if (player->InputMove(elapsedTime)) {
		player->ChangeState(Player::State::IdleToRun);
	}
	// �W�����v���͏���
	if (player->InputButtonDown(Player::InputState::Jump)) {
		player->ChangeState(Player::State::JumpStart);
	}

	// �U������
	player->InputAttackFromNoneAttack();
	// ����ύX
	player->InputChangeWeapon();
	// �\�͏���
	player->InputRecover();
}

/************************************
	StateIdleToRun : �ҋ@->�ړ�
************************************/
void StateIdleToRun::Init()
{
	player->PlayAnimation(Player::Animation::IdleToRun, false);
}

void StateIdleToRun::Update(float elapsedTime)
{
	player->InputMove(elapsedTime);
	// �A�j���[�V�����I����
	if (!player->GetModel()->IsPlayAnimation()) {
		player->ChangeState(Player::State::Run);
	}
}

/************************************
	StateRun : �ړ�
************************************/
void StateRun::Init()
{
	player->PlayAnimation(Player::Animation::Running, true);
}

void StateRun::Update(float elapsedTime)
{
	// �ړ����͏���
	if (!player->InputMove(elapsedTime)) {
		player->ChangeState(Player::State::Idle);
	}
	// �W�����v���͏���
	if (player->InputButtonDown(Player::InputState::Jump)) {
		player->ChangeState(Player::State::JumpStart);
	}

	// �U������
	player->InputAttackFromNoneAttack();
	// ����ύX
	player->InputChangeWeapon();
	// �\�͏���
	player->InputRecover();
}

/************************************
	StateRunToIdle : �ړ�->�ҋ@	// ���g�p
************************************/
void StateRunToIdle::Init()
{
	player->PlayAnimation(Player::Animation::RunToIdle, false);
}

void StateRunToIdle::Update(float elapsedTime)
{
	// �A�j���[�V�����I����
	if (!player->GetModel()->IsPlayAnimation()) {
		player->ChangeState(Player::State::Idle);
	}
}

/************************************
	StateRunToIdle : �W�����v�J�n
************************************/
void StateJumpStart::Init()
{
	player->PlayAnimation(Player::Animation::JumpStart, false);
}

void StateJumpStart::Update(float elapsedTime)
{
	// �ړ����͏���
	player->InputMove(elapsedTime);
	// ����ɃW�����v���͎��̏���
	if (player->InputButtonDown(Player::InputState::Jump)) {
		player->ChangeState(Player::State::JumpAir);
	}
	// �A�j���[�V�����I����
	if (!player->GetModel()->IsPlayAnimation()) {
		player->ChangeState(Player::State::JumpLoop);
	}
}

/************************************
	StateJumpLoop : �W�����v��
************************************/
void StateJumpLoop::Init()
{
	player->PlayAnimation(Player::Animation::JumpLoop, false);
}

void StateJumpLoop::Update(float elapsedTime)
{
	// �ړ����͏���
	player->InputMove(elapsedTime);
	// ����ɃW�����v���͎��̏���
	if (player->InputButtonDown(Player::InputState::Jump)) {
		player->ChangeState(Player::State::JumpAir);
	}
	// �W�����v���̍U��������UpdateJump�ɂčs��

	// ���n(JumpEnd)�X�e�[�g�ւ̑J�ڂ͒��n���ɂ����s��Ȃ�
	// �ی��Ŋ��ɒ��n���Ă��鎞�p�ɌĂяo��
	if (player->IsGround()) {
		player->OnLanding(elapsedTime);
	}
}

/************************************
	StateJumpAir : �W�����v2�i��
************************************/
void StateJumpAir::Init()
{
	player->PlayAnimation(Player::Animation::JumpAir, false);
}

void StateJumpAir::Update(float elapsedTime)
{
	// �ړ����͏���
	player->InputMove(elapsedTime);
	// �W�����v���̍U��������UpdateJump�ɂčs��

	// ���n(JumpEnd)�X�e�[�g�ւ̑J�ڂ͒��n���ɂ����s��Ȃ�
	// �ی��Ŋ��ɒ��n���Ă��鎞�p�ɌĂяo��
	if (player->IsGround()) {
		player->OnLanding(elapsedTime);
	}
}

/************************************
	StateJumpEnd : �W�����v���n
************************************/
void StateJumpEnd::Init()
{
	player->PlayAnimation(Player::Animation::JumpEnd, false);
}

void StateJumpEnd::Update(float elapsedTime)
{
	// Worning�FInputMove���Ȃ�OnLanding�ł�����ʂ�Ȃ��\������

	// �A�j���[�V�������ł����̃W�����v�͉\
	if (player->InputButtonDown(Player::InputState::Jump)) {
		player->ChangeState(Player::State::JumpStart);
	}
	// �A�j���[�V�����I����
	if (!player->GetModel()->IsPlayAnimation()) {
		player->ChangeState(Player::State::Idle);
	}
}

/************************************
	StateDamage : �_���[�W
************************************/
void StateDamage::Init()
{
	player->PlayAnimation(Player::Animation::Damage, false);
}

void StateDamage::Update(float elapsedTime)
{
	// �_���[�W�󂯂Ă���Ԃ͈ړ����͕s��
	 
	// �A�j���[�V�����I����
	if (!player->GetModel()->IsPlayAnimation()) {
		player->ChangeState(Player::State::Idle);
	}
}

/************************************
	StateDead : ���S
************************************/
void StateDead::Init()
{
	player->PlayAnimation(Player::Animation::Death, false);
}

void StateDead::Update(float elapsedTime)
{
	// Death�A�j���[�V�����͋N���オ��܂łȂ̂œr���Ŏ~�߂�̂��ǂ�?
	// �A�j���[�V�����I����A���i�K��Idle�Ɉڍs
	if (!player->GetModel()->IsPlayAnimation())
	{
		// ���i�K�͎����I�ɉ񕜂���
		player->SetHealth(player->GetMaxHealth());
		player->ChangeState(Player::State::Idle);
	}
}

/************************************
	StateRecover : ��
************************************/
void StateRecover::Init()
{
	player->PlayAnimation(Player::Animation::JumpEnd, false);
	player->PlayEffect(Player::EffectNumber::Recovery, player->GetTargetPlayer()->GetPosition(), 0.6f);
}

void StateRecover::Update(float elapsedTime)
{
	if (player->GetModel()->IsPlayAnimation()) return;

	// ��
	player->GetTargetPlayer()->AddHealth(recover_add_health);

	// PlayerAI�̓�������
	if (player->GetTargetPlayer()->GetSerialNumber() == PlayerAI::Instance().GetSerialNumber())
	{
		if (!PlayerAI::Instance().GetHpWorning())
		{
			PlayerAI::Instance().SetRanAwayFromEnemy(false);
			PlayerAI::Instance().SetEnableShowMessage(Player::PlayerMessage::RanAway, false);
		}
	}
	// �ڍs
	player->ChangeState(Player::State::Idle);
}

/************************************
	StateAttackHammer1 : �U���n���}�[
************************************/
void StateAttackHammer1::Init()
{
	player->SetAttacking(true);
	player->SetAttackJadge(true);
	player->PlayAnimation(Player::Animation::AttackHammer1, false);
}

void StateAttackHammer1::Update(float elapsedTime)
{
	Player::Weapon Hammer = player->GetHammer();
	Hammer.flag1 = (player->GetModel()->IsPlayAnimation());
	if (Hammer.flag1)
	{
		player->UpdateWeaponPositions(player->GetModel(), Hammer);
		float animationTime = player->GetModel()->GetCurrentAnimationSeconds();
		//�U�������蔻�莞��
		if (animationTime >= attack_time_hammer1_start)
		{
			player->CollisionWeaponsVsEnemies(Hammer);
		}
		//�C�ӂ̃A�j���[�V�����Đ���Ԃł̂ݎ��̍U���Z���o���悤�ɂ���
		if (player->InputButtonDown(Player::InputState::Attack)
			&& IsDuringTime(animationTime, next_attack_time_hammer1to2))
		{
			player->ChangeState(Player::State::AttackHammer2);
		}

		// �ߋ����U�����̊p�x����
		player->ForceTurnByAttack(elapsedTime);
	}
	else {
		player->ChangeState(Player::State::Idle);
	}
}

/************************************
	StateAttackHammer2 : �U���n���}�[
************************************/
void StateAttackHammer2::Init()
{
	player->SetAttackJadge(true);
	player->PlayAnimation(Player::Animation::AttackHammer2, false);
}

void StateAttackHammer2::Update(float elapsedTime)
{
	Player::Weapon Hammer = player->GetHammer();
	Hammer.flag1 = false;
	Hammer.flag2 = (player->GetModel()->IsPlayAnimation());
	if (Hammer.flag2)
	{
		player->UpdateWeaponPositions(player->GetModel(), Hammer);
		if (player->GetModel()->GetCurrentAnimationSeconds() <= attack_time_hammer2_end)
		{
			player->CollisionWeaponsVsEnemies(Hammer);
		}
	}
	else {
		player->ChangeState(Player::State::Idle);
	}
}

/************************************
	StateAttackHammerJump : �W�����v�U���n���}�[
************************************/
void StateAttackHammerJump::Init()
{
	player->SetAttacking(true);
	player->SetAttackJadge(true);
	player->SetMoveAttack(player->InputMove(0));
	player->PlayAnimation(Player::Animation::AttackHammerJump, false);
}

void StateAttackHammerJump::Update(float elapsedTime)
{
	Player::Weapon Hammer = player->GetHammer();
	Hammer.flagJump = player->GetModel()->IsPlayAnimation();
	if (Hammer.flagJump)
	{
		player->UpdateWeaponPositions(player->GetModel(), Hammer);
		player->CollisionWeaponsVsEnemies(Hammer);
		player->PlayEffect(Player::EffectNumber::RedVortex, Hammer.position, attack_effect_size);
	}

	if (player->GetMoveAttack())
	{
		// �ړ����Ȃ���̍U���̏ꍇ�Axz�����Ɉ�葬�x������������
		player->HorizontalVelocityByAttack(false, attack_hammerJ_velocity, elapsedTime);
	}
	if (!Hammer.flagJump)
	{
		player->SetMoveAttack(false);
		player->ChangeState(Player::State::Idle);
	}

	//�W�����v�U�����͏d�͂𖳎�����
	player->SetVelocity(FLT_MAX, 0);
}

/************************************
	StateAttackSpear1 : �U���X�s�A�[
************************************/
void StateAttackSpear1::Init()
{
	player->SetAttacking(true);
	player->SetAttackJadge(true);
	player->PlayAnimation(Player::Animation::AttackSpear1, false);
}

void StateAttackSpear1::Update(float elapsedTime)
{
	Player::Weapon Spear = player->GetSpear();
	if (player->GetModel()->IsPlayAnimation())
	{
		player->UpdateWeaponPositions(player->GetModel(), Spear);
		player->CollisionWeaponsVsEnemies(Spear);

		float animationTime = player->GetModel()->GetCurrentAnimationSeconds();
		// ����o���A�j���[�V�����Đ����
		Spear.flag1 = IsDuringTime(animationTime, next_attack_time_spear1to2);
		// ���𓥂񒣂�ۂ̑O�i�������ōs�� ���ɉ����i��ł��鎞�͂��̏��������Ȃ�
		if (animationTime < add_velocity_time_spear1_end && !player->InputMove(elapsedTime))
		{
			player->HorizontalVelocityByAttack(true, attack_spear1_add_velocity, elapsedTime);
		}
		else if (Spear.flag1 && player->InputButtonDown(Player::InputState::Attack))
		{
			player->AddAttackCount();
			player->ChangeState(Player::State::AttackSpear2);
		}
	}
	else {
		player->ChangeState(Player::State::Idle);
	}
}

/************************************
	StateAttackSpear2 : �U���X�s�A�[
************************************/
void StateAttackSpear2::Init()
{
	player->SetAttackJadge(true);
	player->PlayAnimation(Player::Animation::AttackSpear2, false);
}

void StateAttackSpear2::Update(float elapsedTime)
{
	Player::Weapon Spear = player->GetSpear();
	Spear.flag1 = false;
	Spear.flag2 = player->GetModel()->IsPlayAnimation();
	if (Spear.flag2)
	{
		player->UpdateWeaponPositions(player->GetModel(), Spear);

		float animationTime = player->GetModel()->GetCurrentAnimationSeconds();
		if (animationTime >= attack_time_spear2_start) {
			player->CollisionWeaponsVsEnemies(Spear);
		}
		if (IsDuringTime(animationTime, add_velocity_time_spear2))
		{
			// ���𓥂񒣂�ۂ̑O�i
			player->HorizontalVelocityByAttack(true, attack_spear2_add_velocity, elapsedTime);
		}
		// ����o���A�j���[�V�����Đ����
		if (IsDuringTime(animationTime, next_attack_time_spear2to3) && player->InputButtonDown(Player::InputState::Attack))
		{
			player->AddAttackCount();
			if (player->GetAttackCount() < attack_count_finish) {
				player->ChangeState(Player::State::AttackSpear1);
			}
			else {
				player->ChangeState(Player::State::AttackSpear3);
			}
		}
	}
	else {
		player->ChangeState(Player::State::Idle);
	}
}

/************************************
	StateAttackSpear3 : �U���X�s�A�[
************************************/
void StateAttackSpear3::Init()
{
	player->SetAttackJadge(true);
	player->PlayAnimation(Player::Animation::AttackSpear3, false);
}

void StateAttackSpear3::Update(float elapsedTime)
{
	Player::Weapon Spear = player->GetSpear();
	Spear.flag2 = false;
	Spear.flag3 = player->GetModel()->IsPlayAnimation();
	if (Spear.flag3)
	{
		player->UpdateWeaponPositions(player->GetModel(), Spear);
		float animationTime = player->GetModel()->GetCurrentAnimationSeconds();
		if (animationTime >= attack_time_spear3_start) {
			player->CollisionWeaponsVsEnemies(Spear);
		}
		// ���𓥂񒣂�ۂ̑O�i�������ōs��
		if (animationTime < add_velocity_time_spear3_end)
		{
			player->HorizontalVelocityByAttack(true, attack_spear2_add_velocity, elapsedTime);
		}
	}
	else {
		player->ChangeState(Player::State::Idle);
	}
}

/************************************
	StateAttackSpearJump : �W�����v�U���X�s�A�[
************************************/
void StateAttackSpearJump::Init()
{
	player->SetAttacking(true);
	player->SetAttackJadge(true);
	player->SetMoveAttack(player->InputMove(0));
	player->PlayAnimation(Player::Animation::AttackSpearJump, false);
}

void StateAttackSpearJump::Update(float elapsedTime)
{
	Player::Weapon Spear = player->GetSpear();
	Spear.flagJump = player->GetModel()->IsPlayAnimation();
	if (Spear.flagJump)
	{
		player->UpdateWeaponPositions(player->GetModel(), Spear);
		player->CollisionWeaponsVsEnemies(Spear);

		float animationTime = player->GetModel()->GetCurrentAnimationSeconds();
		// ��]���Ȃ���O�������ɓ˂��h���Ă����A�j���[�V����
		if (IsDuringTime(animationTime, add_velocity_time_spearJ))
		{
			player->HorizontalVelocityByAttack(false, attack_spearJ_velocity, elapsedTime);
			player->PlayEffect(Player::EffectNumber::BlueVortex, Spear.position, attack_effect_size);
		}
	}
	else {
		player->ChangeState(Player::State::Idle);
	}
	/**** �d�͋�����[UpdateVerticalVelocity]�ɂčs�� ****/
}

/************************************
	StateAttackSword1 : �U���\�[�h
************************************/
void StateAttackSword1::Init()
{
	player->SetAttacking(true);
	player->SetAttackJadge(true);
	player->PlayAnimation(Player::Animation::AttackSword1, false);
}

void StateAttackSword1::Update(float elapsedTime)
{
	Player::Weapon Sword = player->GetSword();
	Sword.flag1 = player->GetModel()->IsPlayAnimation();
	if (Sword.flag1)
	{
		player->UpdateWeaponPositions(player->GetModel(), Sword);
		player->CollisionWeaponsVsEnemies(Sword);
		
		float animationTime = player->GetModel()->GetCurrentAnimationSeconds();
		// ���𓥂񒣂�ۂ̑O�i�������ōs�� ���ɉ����i��ł��鎞�͂��̏��������Ȃ�
		if (animationTime < add_velocity_time_sword1_end && !player->InputMove(elapsedTime))
		{
			player->HorizontalVelocityByAttack(true, attack_sword1_add_velocity, elapsedTime);
		}
		// �C�ӂ̃A�j���[�V�����Đ���Ԃł̂ݎ��̍U���Z���o���悤�ɂ���
		else if (player->InputButtonDown(Player::InputState::Attack)
			&& IsDuringTime(animationTime, next_attack_time_sword1to2))
		{
			player->AddAttackCount();
			player->ChangeState(Player::State::AttackSword2);
		}
	}
	else {
		player->ChangeState(Player::State::Idle);
	}
}

/************************************
	StateAttackSword2 : �U���\�[�h
************************************/
void StateAttackSword2::Init()
{
	player->SetAttackJadge(true);
	player->PlayAnimation(Player::Animation::AttackSword2, false);
}

void StateAttackSword2::Update(float elapsedTime)
{
	Player::Weapon Sword = player->GetSword();
	Sword.flag1 = false;
	Sword.flag2 = player->GetModel()->IsPlayAnimation();
	if (Sword.flag2)
	{
		player->UpdateWeaponPositions(player->GetModel(), Sword);
		player->CollisionWeaponsVsEnemies(Sword);

		float animationTime = player->GetModel()->GetCurrentAnimationSeconds();
		// ���𓥂񒣂�ۂ̑O�i�������ōs��
		if (animationTime < add_velocity_time_sword2_end)
		{
			player->HorizontalVelocityByAttack(true, attack_sword1_add_velocity, elapsedTime);
		}
		// �C�ӂ̃A�j���[�V�����Đ���Ԃł̂ݎ��̍U���Z���o���悤�ɂ���
		else if (player->InputButtonDown(Player::InputState::Attack)
			&& IsDuringTime(animationTime, next_attack_time_sword2to3))
		{
			player->AddAttackCount();
			if (player->GetAttackCount() < attack_count_finish) {
				player->ChangeState(Player::State::AttackSword1);
			}
			else {
				player->ChangeState(Player::State::AttackSword3);
			}
		}
	}
	else {
		player->ChangeState(Player::State::Idle);
	}
}

/************************************
	StateAttackSword3 : �U���\�[�h
************************************/
void StateAttackSword3::Init()
{
	player->SetAttackJadge(true);
	player->PlayAnimation(Player::Animation::AttackSword3, false);
}

void StateAttackSword3::Update(float elapsedTime)
{
	Player::Weapon Sword = player->GetSword();
	Sword.flag2 = false;
	Sword.flag3 = player->GetModel()->IsPlayAnimation();
	if (Sword.flag3)
	{
		player->UpdateWeaponPositions(player->GetModel(), Sword);
		float animationTime = player->GetModel()->GetCurrentAnimationSeconds();
		if (IsDuringTime(animationTime, attack_time_sword3)) {
			player->CollisionWeaponsVsEnemies(Sword);
		}

		// ���𓥂񒣂�ۂ̑O�i�������ōs��
		if (IsDuringTime(animationTime, add_velocity_time_sword3))
		{
			player->HorizontalVelocityByAttack(true, attack_sword3_add_velocity, elapsedTime);
			// Y�������֒��˂�
			if (animationTime <= add_velocity_y_time_sword3_end) {
				player->AddVelocity({ 0, attack_sword3_add_velocity_y * elapsedTime, 0 });
			}
		}
	}
	else {
		player->ChangeState(Player::State::Idle);
	}
}

/************************************
	StateAttackSwordJump : �W�����v�U���\�[�h
************************************/
void StateAttackSwordJump::Init()
{
	player->SetAttacking(true);
	player->SetAttackJadge(true);
	player->SetMoveAttack(player->InputMove(0));
	player->PlayAnimation(Player::Animation::AttackSwordJump, false);
}

void StateAttackSwordJump::Update(float elapsedTime)
{
	Player::Weapon Sword = player->GetSword();
	Sword.flagJump = player->GetModel()->IsPlayAnimation();
	if (Sword.flagJump)
	{
		player->UpdateWeaponPositions(player->GetModel(), Sword);
		player->CollisionWeaponsVsEnemies(Sword);
		player->PlayEffect(Player::EffectNumber::GreenVortex, Sword.position, attack_effect_size);
	}

	if (player->GetMoveAttack())
	{
		player->HorizontalVelocityByAttack(false, attack_swordJ_velocity, elapsedTime);
	}
	if (!player->GetModel()->IsPlayAnimation())
	{
		player->SetMoveAttack(false);
		player->ChangeState(Player::State::Idle);
	}

	//�W�����v�U�����͏d�͂𖳎�����
	player->SetVelocity(FLT_MAX, 0);
}


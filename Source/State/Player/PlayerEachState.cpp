#include "PlayerEachState.h"
#include "Player.h"
#include "PlayerManager.h"

/************************************
	StateIdle : �ҋ@
************************************/

void StateIdle::Init()
{
	player->SetAttackCount(0);
	// �U����͕K���ҋ@�ɖ߂�ׂ����ōU���^�C�v ���Z�b�g���s��
	player->SetAttackType(Player::AttackType::None);
	player->GetModel()->PlayAnimation(static_cast<int>(Player::Animation::Idle), true);
}

void StateIdle::Update(float elapsedTime)
{
	// �ړ����͏���
	if (player->InputMove(elapsedTime)) {
		player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::IdleToRun));
	}
	// �W�����v���͏���
	if (player->InputJumpButtonDown()) {
		player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::JumpStart));
	}

	// �U������
	player->InputAttackFromNoneAttack(elapsedTime);
}

/************************************
	StateIdleToRun : �ҋ@->�ړ�
************************************/
void StateIdleToRun::Init()
{
	player->GetModel()->PlayAnimation(static_cast<int>(Player::Animation::IdleToRun), false);
}

void StateIdleToRun::Update(float elapsedTime)
{
	player->InputMove(elapsedTime);
	// �A�j���[�V�����I����
	if (!player->GetModel()->IsPlayAnimation()) {
		player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::Run));
	}
}

/************************************
	StateRun : �ړ�
************************************/
void StateRun::Init()
{
	player->GetModel()->PlayAnimation(static_cast<int>(Player::Animation::Running), true);
}

void StateRun::Update(float elapsedTime)
{
	// �ړ����͏���
	if (!player->InputMove(elapsedTime)) {
		player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::Idle));
	}
	// �W�����v���͏���
	if (player->InputJumpButtonDown()) {
		player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::JumpStart));
	}
	// �񕜑J�ڊm�F����
	if (player->IsRecoverTransition()) {
		player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::Recover));
	}

	// �U������
	player->InputAttackFromNoneAttack(elapsedTime);
}

/************************************
	StateRunToIdle : �ړ�->�ҋ@	// ���g�p
************************************/
void StateRunToIdle::Init()
{
	player->GetModel()->PlayAnimation(static_cast<int>(Player::Animation::RunToIdle), false);
}

void StateRunToIdle::Update(float elapsedTime)
{
	// �A�j���[�V�����I����
	if (!player->GetModel()->IsPlayAnimation()) {
		player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::Idle));
	}
}

/************************************
	StateRunToIdle : �W�����v�J�n
************************************/
void StateJumpStart::Init()
{
	player->GetModel()->PlayAnimation(static_cast<int>(Player::Animation::JumpStart), false);
}

void StateJumpStart::Update(float elapsedTime)
{
	// �ړ����͏���
	player->InputMove(elapsedTime);
	// ����ɃW�����v���͎��̏���
	if (player->InputJumpButtonDown()) {
		player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::JumpAir));
	}
	// �A�j���[�V�����I����
	if (!player->GetModel()->IsPlayAnimation()) {
		player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::JumpLoop));
	}
}

/************************************
	StateJumpLoop : �W�����v��
************************************/
void StateJumpLoop::Init()
{
	player->GetModel()->PlayAnimation(static_cast<int>(Player::Animation::JumpLoop), false);
}

void StateJumpLoop::Update(float elapsedTime)
{
	// �ړ����͏���
	player->InputMove(elapsedTime);
	// ����ɃW�����v���͎��̏���
	if (player->InputJumpButtonDown()) {
		player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::JumpAir));
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
	player->GetModel()->PlayAnimation(static_cast<int>(Player::Animation::JumpAir), false);
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
	player->GetModel()->PlayAnimation(static_cast<int>(Player::Animation::JumpEnd), false);
}

void StateJumpEnd::Update(float elapsedTime)
{
	// Worning�FInputMove���Ȃ�OnLanding�ł�����ʂ�Ȃ��\������

	// �A�j���[�V�������ł����̃W�����v�͉\
	if (player->InputJumpButtonDown()) {
		player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::JumpStart));
	}
	// �A�j���[�V�����I����
	if (!player->GetModel()->IsPlayAnimation()) {
		player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::Idle));
	}
}

/************************************
	StateDamage : �_���[�W
************************************/
void StateDamage::Init()
{
	player->GetModel()->PlayAnimation(static_cast<int>(Player::Animation::Damage), false);
}

void StateDamage::Update(float elapsedTime)
{
	// �_���[�W�󂯂Ă���Ԃ͈ړ����͕s��
	 
	// �A�j���[�V�����I����
	if (!player->GetModel()->IsPlayAnimation()) {
		player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::Idle));
	}
}

/************************************
	StateDead : ���S
************************************/
void StateDead::Init()
{
	player->GetModel()->PlayAnimation(static_cast<int>(Player::Animation::Death), false);
}

void StateDead::Update(float elapsedTime)
{
	// Death�A�j���[�V�����͋N���オ��܂łȂ̂œr���Ŏ~�߂�̂��ǂ�?
	// �A�j���[�V�����I����A���i�K��Idle�Ɉڍs
	if (!player->GetModel()->IsPlayAnimation())
	{
		// ���i�K�͎����I�ɉ񕜂���
		player->SetHealth(player->GetMaxHealth());
		player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::Idle));
	}
}

/************************************
	StateRecover : ��
************************************/
void StateRecover::Init()
{
	player->GetModel()->PlayAnimation(static_cast<int>(Player::Animation::JumpEnd), false);
}

void StateRecover::Update(float elapsedTime)
{
	// �A�j���[�V�����I����
	if (!player->GetModel()->IsPlayAnimation())
	{
		//�񕜏���
		Player* targetplayer = player;
		PlayerManager& playerManager = PlayerManager::Instance();
		int playerCount = playerManager.GetPlayerCount();
		for (int i = 0; i < playerCount; i++)
		{
			if (playerManager.GetPlayer(i) == player) continue;
			targetplayer = playerManager.GetPlayer(i);
		}
		targetplayer->AddHealth(30);

		//�ڍs
		player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::Idle));
	}
}

/************************************
	StateAttackHammer1 : �U���n���}�[
************************************/
void StateAttackHammer1::Init()
{
	player->SetAttackType(Player::AttackType::Hammer);
	player->SetAttackJadge(true);
	player->GetModel()->PlayAnimation(static_cast<int>(Player::Animation::AttackHammer1), false);
}

void StateAttackHammer1::Update(float elapsedTime)
{
	Player::Arms Hammer = player->GetHammer();
	Hammer.flag1 = (player->GetModel()->IsPlayAnimation());
	if (Hammer.flag1)
	{
		player->UpdateArmPositions(player->GetModel(), Hammer);
		float animationTime = player->GetModel()->GetCurrentAnimationSeconds();
		//�U�������蔻�莞��
		if (animationTime >= 0.4f) {
			player->CollisionArmsVsEnemies(Hammer);
		}
		//�C�ӂ̃A�j���[�V�����Đ���Ԃł̂ݎ��̍U���Z���o���悤�ɂ���
		if (player->InputHammerButton() && animationTime >= 0.5f && animationTime <= 0.8f) {
			player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::AttackHammer2));
		}
	}
	else {
		player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::Idle));
	}
}

/************************************
	StateAttackHammer2 : �U���n���}�[
************************************/
void StateAttackHammer2::Init()
{
	player->SetAttackJadge(true);
	player->GetModel()->PlayAnimation(static_cast<int>(Player::Animation::AttackHammer2), false);
}

void StateAttackHammer2::Update(float elapsedTime)
{
	Player::Arms Hammer = player->GetHammer();
	Hammer.flag1 = false;
	Hammer.flag2 = (player->GetModel()->IsPlayAnimation());
	if (Hammer.flag2)
	{
		player->UpdateArmPositions(player->GetModel(), Hammer);
		player->CollisionArmsVsEnemies(Hammer);
	}
	else {
		player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::Idle));
	}
}

/************************************
	StateAttackHammerJump : �W�����v�U���n���}�[
************************************/
void StateAttackHammerJump::Init()
{
	player->SetAttackType(Player::AttackType::Hammer);
	player->SetAttackJadge(true);
	player->SetMoveAttack(player->InputMove(0));
	player->GetModel()->PlayAnimation(static_cast<int>(Player::Animation::AttackHammerJump), false);
}

void StateAttackHammerJump::Update(float elapsedTime)
{
	Player::Arms Hammer = player->GetHammer();
	Hammer.flagJump = player->GetModel()->IsPlayAnimation();
	if (Hammer.flagJump)
	{
		player->UpdateArmPositions(player->GetModel(), Hammer);
		player->CollisionArmsVsEnemies(Hammer);
	}

	if (player->GetMoveAttack())
	{
		player->HorizontalVelocityByAttack(false, 800, elapsedTime);
	}
	if (!Hammer.flagJump)
	{
		player->SetMoveAttack(false);
		player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::Idle));
	}

	//�W�����v�U�����͏d�͂𖳎�����
	player->SetVelocity(FLT_MAX, 0);
}

/************************************
	StateAttackSpear1 : �U���X�s�A�[
************************************/
void StateAttackSpear1::Init()
{
	player->SetAttackType(Player::AttackType::Spear);
	player->SetAttackJadge(true);
	player->GetModel()->PlayAnimation(static_cast<int>(Player::Animation::AttackSpear1), false);
}

void StateAttackSpear1::Update(float elapsedTime)
{
	Player::Arms Spear = player->GetSpear();
	if (player->GetModel()->IsPlayAnimation())
	{
		player->UpdateArmPositions(player->GetModel(), Spear);
		player->CollisionArmsVsEnemies(Spear);

		float animationTime = player->GetModel()->GetCurrentAnimationSeconds();
		// ����o���A�j���[�V�����Đ����
		Spear.flag1 = (animationTime >= 0.20f && animationTime <= 0.7f);
		// ���𓥂񒣂�ۂ̑O�i�������ōs�� ���ɉ����i��ł��鎞�͂��̏��������Ȃ�
		if (animationTime < 0.25f && !player->InputMove(elapsedTime))
		{
			player->HorizontalVelocityByAttack(true, 40, elapsedTime);
		}
		else if (Spear.flag1 && player->InputSpearButton())
		{
			player->AddAttackCount();
			player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::AttackSpear2));
		}
	}
	else {
		player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::Idle));
	}
}

/************************************
	StateAttackSpear2 : �U���X�s�A�[
************************************/
void StateAttackSpear2::Init()
{
	player->SetAttackJadge(true);
	player->GetModel()->PlayAnimation(static_cast<int>(Player::Animation::AttackSpear2), false);
}

void StateAttackSpear2::Update(float elapsedTime)
{
	Player::Arms Spear = player->GetSpear();
	Spear.flag1 = false;
	Spear.flag2 = player->GetModel()->IsPlayAnimation();
	if (Spear.flag2)
	{
		player->UpdateArmPositions(player->GetModel(), Spear);

		float animationTime = player->GetModel()->GetCurrentAnimationSeconds();
		if (animationTime >= 0.30f) player->CollisionArmsVsEnemies(Spear);

		if (animationTime >= 0.30f && animationTime <= 0.45f)
		{
			// ���𓥂񒣂�ۂ̑O�i
			player->HorizontalVelocityByAttack(true, 60, elapsedTime);
		}
		// ����o���A�j���[�V�����Đ����
		if (animationTime >= 0.37f && animationTime <= 0.6f && player->InputSpearButton())
		{
			player->AddAttackCount();
			if (player->GetAttackCount() < 4) {
				player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::AttackSpear1));
			}
			else {
				player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::AttackSpear3));
			}
		}
	}
	else {
		player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::Idle));
	}
}

/************************************
	StateAttackSpear3 : �U���X�s�A�[
************************************/
void StateAttackSpear3::Init()
{
	player->SetAttackJadge(true);
	player->GetModel()->PlayAnimation(static_cast<int>(Player::Animation::AttackSpear3), false);
}

void StateAttackSpear3::Update(float elapsedTime)
{
	Player::Arms Spear = player->GetSpear();
	Spear.flag2 = false;
	Spear.flag3 = player->GetModel()->IsPlayAnimation();
	if (Spear.flag3)
	{
		player->UpdateArmPositions(player->GetModel(), Spear);
		float animationTime = player->GetModel()->GetCurrentAnimationSeconds();
		if (animationTime >= 0.30f) player->CollisionArmsVsEnemies(Spear);

		// ���𓥂񒣂�ۂ̑O�i�������ōs��
		if (animationTime < 0.43f)
		{
			player->HorizontalVelocityByAttack(true, 39, elapsedTime);
		}
	}
	else {
		player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::Idle));
	}
}

/************************************
	StateAttackSpearJump : �W�����v�U���X�s�A�[
************************************/
void StateAttackSpearJump::Init()
{
	player->SetAttackType(Player::AttackType::Spear);
	player->SetAttackJadge(true);
	player->SetMoveAttack(player->InputMove(0));
	player->GetModel()->PlayAnimation(static_cast<int>(Player::Animation::AttackSpearJump), false);
}

void StateAttackSpearJump::Update(float elapsedTime)
{
	Player::Arms Spear = player->GetSpear();
	Spear.flagJump = player->GetModel()->IsPlayAnimation();
	if (Spear.flagJump)
	{
		player->UpdateArmPositions(player->GetModel(), Spear);
		player->CollisionArmsVsEnemies(Spear);

		float animationTime = player->GetModel()->GetCurrentAnimationSeconds();
		// ��]���Ȃ���O�������ɓ˂��h���Ă����A�j���[�V����
		if (animationTime >= 0.15f && animationTime <= 0.5f)
		{
			player->HorizontalVelocityByAttack(false, 800, elapsedTime);
		}
	}
	else {
		player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::Idle));
	}

	// �d�͋���(UpdateVerticalVelocity)
}

/************************************
	StateAttackSword1 : �U���\�[�h
************************************/
void StateAttackSword1::Init()
{
	player->SetAttackType(Player::AttackType::Sword);
	player->SetAttackJadge(true);
	player->GetModel()->PlayAnimation(static_cast<int>(Player::Animation::AttackSword1), false);
}

void StateAttackSword1::Update(float elapsedTime)
{
	Player::Arms Sword = player->GetSword();
	Sword.flag1 = player->GetModel()->IsPlayAnimation();
	if (Sword.flag1)
	{
		player->UpdateArmPositions(player->GetModel(), Sword);
		player->CollisionArmsVsEnemies(Sword);

		float animationTime = player->GetModel()->GetCurrentAnimationSeconds();
		// ���𓥂񒣂�ۂ̑O�i�������ōs�� ���ɉ����i��ł��鎞�͂��̏��������Ȃ�
		if (animationTime < 0.2f && !player->InputMove(elapsedTime))
		{
			player->HorizontalVelocityByAttack(true, 43, elapsedTime);
		}
		// �C�ӂ̃A�j���[�V�����Đ���Ԃł̂ݎ��̍U���Z���o���悤�ɂ���
		else if (player->InputSwordButton() && animationTime >= 0.3f && animationTime <= 0.7f)
		{
			player->AddAttackCount();
			player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::AttackSword2));
		}
		// �����̓����ɍ��킹���O�i ��a������̂ň�U�R�����g
		//else if (animationTime >= 0.50f)
		//{
		//	velocity.x += sinf(angle.y) * 38 * elapsedTime;
		//	velocity.z += cosf(angle.y) * 38 * elapsedTime;
		//}
	}
	else {
		player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::Idle));
	}
}

/************************************
	StateAttackSword2 : �U���\�[�h
************************************/
void StateAttackSword2::Init()
{
	player->SetAttackJadge(true);
	player->GetModel()->PlayAnimation(static_cast<int>(Player::Animation::AttackSword2), false);
}

void StateAttackSword2::Update(float elapsedTime)
{
	Player::Arms Sword = player->GetSword();
	Sword.flag1 = false;
	Sword.flag2 = player->GetModel()->IsPlayAnimation();
	if (Sword.flag2)
	{
		player->UpdateArmPositions(player->GetModel(), Sword);
		player->CollisionArmsVsEnemies(Sword);

		float animationTime = player->GetModel()->GetCurrentAnimationSeconds();
		// ���𓥂񒣂�ۂ̑O�i�������ōs��
		if (animationTime < 0.25f)
		{
			player->HorizontalVelocityByAttack(true, 45, elapsedTime);
		}
		// �C�ӂ̃A�j���[�V�����Đ���Ԃł̂ݎ��̍U���Z���o���悤�ɂ���
		else if (player->InputSwordButton() && animationTime >= 0.35f && animationTime <= 0.6f)
		{
			player->AddAttackCount();
			if (player->GetAttackCount() < 4) {
				player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::AttackSword1));
			}
			else {
				player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::AttackSword3));
			}
		}
	}
	else {
		player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::Idle));
	}
}

/************************************
	StateAttackSword3 : �U���\�[�h
************************************/
void StateAttackSword3::Init()
{
	player->SetAttackJadge(true);
	player->GetModel()->PlayAnimation(static_cast<int>(Player::Animation::AttackSword3), false);
}

void StateAttackSword3::Update(float elapsedTime)
{
	Player::Arms Sword = player->GetSword();
	Sword.flag2 = false;
	Sword.flag3 = player->GetModel()->IsPlayAnimation();
	if (Sword.flag3)
	{
		player->UpdateArmPositions(player->GetModel(), Sword);
		float animationTime = player->GetModel()->GetCurrentAnimationSeconds();
		if (animationTime >= 0.25f && animationTime <= 0.6f) {
			player->CollisionArmsVsEnemies(Sword);
		}

		// ���𓥂񒣂�ۂ̑O�i�������ōs��
		if (animationTime >= 0.25f && animationTime <= 0.5f)
		{
			player->HorizontalVelocityByAttack(true, 48, elapsedTime);
			if (animationTime <= 0.4f) {
				player->AddVelocity({ 0, 150 * elapsedTime, 0 });
			}
		}
	}
	else {
		player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::Idle));
	}
}

/************************************
	StateAttackSwordJump : �W�����v�U���\�[�h
************************************/
void StateAttackSwordJump::Init()
{
	player->SetAttackType(Player::AttackType::Sword);
	player->SetAttackJadge(true);
	player->SetMoveAttack(player->InputMove(0));
	player->GetModel()->PlayAnimation(static_cast<int>(Player::Animation::AttackSwordJump), false);
}

void StateAttackSwordJump::Update(float elapsedTime)
{
	Player::Arms Sword = player->GetSword();
	Sword.flagJump = player->GetModel()->IsPlayAnimation();
	if (Sword.flagJump)
	{
		player->UpdateArmPositions(player->GetModel(), Sword);
		player->CollisionArmsVsEnemies(Sword);
	}

	if (player->GetMoveAttack())
	{
		player->HorizontalVelocityByAttack(false, 800, elapsedTime);
	}
	if (!player->GetModel()->IsPlayAnimation())
	{
		player->SetMoveAttack(false);
		player->GetStateMachine()->ChangeState(static_cast<int>(Player::State::Idle));
	}

	//�W�����v�U�����͏d�͂𖳎�����
	player->SetVelocity(FLT_MAX, 0);
}


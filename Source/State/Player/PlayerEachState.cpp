#include "PlayerEachState.h"
#include "Player.h"
#include "PlayerManager.h"
#include "AnimationTimeStruct.h"

/************************************
	StateIdle : 待機
************************************/

void StateIdle::Init()
{
	player->SetAttackCount(0); // attackCountのリセット
	// 攻撃後は必ず待機に戻る為ここで攻撃タイプ リセットを行う
	player->SetAttacking(false);
	player->PlayAnimation(Player::Animation::Idle, true);
}

void StateIdle::Update(float elapsedTime)
{
	// 移動入力処理
	if (player->InputMove(elapsedTime)) {
		player->ChangeState(Player::State::IdleToRun);
	}
	// ジャンプ入力処理
	if (player->InputButtonDown(Player::InputState::Jump)) {
		player->ChangeState(Player::State::JumpStart);
	}

	// 攻撃処理
	player->InputAttackFromNoneAttack();
	// 武器変更
	player->InputChangeWeapon();
	// 能力処理
	player->InputRecover();
}

/************************************
	StateIdleToRun : 待機->移動
************************************/
void StateIdleToRun::Init()
{
	player->PlayAnimation(Player::Animation::IdleToRun, false);
}

void StateIdleToRun::Update(float elapsedTime)
{
	player->InputMove(elapsedTime);
	// アニメーション終了後
	if (!player->GetModel()->IsPlayAnimation()) {
		player->ChangeState(Player::State::Run);
	}
}

/************************************
	StateRun : 移動
************************************/
void StateRun::Init()
{
	player->PlayAnimation(Player::Animation::Running, true);
}

void StateRun::Update(float elapsedTime)
{
	// 移動入力処理
	if (!player->InputMove(elapsedTime)) {
		player->ChangeState(Player::State::Idle);
	}
	// ジャンプ入力処理
	if (player->InputButtonDown(Player::InputState::Jump)) {
		player->ChangeState(Player::State::JumpStart);
	}

	// 攻撃処理
	player->InputAttackFromNoneAttack();
	// 武器変更
	player->InputChangeWeapon();
	// 能力処理
	player->InputRecover();
}

/************************************
	StateRunToIdle : 移動->待機	// 未使用
************************************/
void StateRunToIdle::Init()
{
	player->PlayAnimation(Player::Animation::RunToIdle, false);
}

void StateRunToIdle::Update(float elapsedTime)
{
	// アニメーション終了後
	if (!player->GetModel()->IsPlayAnimation()) {
		player->ChangeState(Player::State::Idle);
	}
}

/************************************
	StateRunToIdle : ジャンプ開始
************************************/
void StateJumpStart::Init()
{
	player->PlayAnimation(Player::Animation::JumpStart, false);
}

void StateJumpStart::Update(float elapsedTime)
{
	// 移動入力処理
	player->InputMove(elapsedTime);
	// さらにジャンプ入力時の処理
	if (player->InputButtonDown(Player::InputState::Jump)) {
		player->ChangeState(Player::State::JumpAir);
	}
	// アニメーション終了後
	if (!player->GetModel()->IsPlayAnimation()) {
		player->ChangeState(Player::State::JumpLoop);
	}
}

/************************************
	StateJumpLoop : ジャンプ中
************************************/
void StateJumpLoop::Init()
{
	player->PlayAnimation(Player::Animation::JumpLoop, false);
}

void StateJumpLoop::Update(float elapsedTime)
{
	// 移動入力処理
	player->InputMove(elapsedTime);
	// さらにジャンプ入力時の処理
	if (player->InputButtonDown(Player::InputState::Jump)) {
		player->ChangeState(Player::State::JumpAir);
	}
	// ジャンプ中の攻撃処理はUpdateJumpにて行う

	// 着地(JumpEnd)ステートへの遷移は着地時にしか行わない
	// 保険で既に着地している時用に呼び出し
	if (player->IsGround()) {
		player->OnLanding(elapsedTime);
	}
}

/************************************
	StateJumpAir : ジャンプ2段目
************************************/
void StateJumpAir::Init()
{
	player->PlayAnimation(Player::Animation::JumpAir, false);
}

void StateJumpAir::Update(float elapsedTime)
{
	// 移動入力処理
	player->InputMove(elapsedTime);
	// ジャンプ中の攻撃処理はUpdateJumpにて行う

	// 着地(JumpEnd)ステートへの遷移は着地時にしか行わない
	// 保険で既に着地している時用に呼び出し
	if (player->IsGround()) {
		player->OnLanding(elapsedTime);
	}
}

/************************************
	StateJumpEnd : ジャンプ着地
************************************/
void StateJumpEnd::Init()
{
	player->PlayAnimation(Player::Animation::JumpEnd, false);
}

void StateJumpEnd::Update(float elapsedTime)
{
	// Worning：InputMove時などOnLandingでここを通らない可能性あり

	// アニメーション中でも次のジャンプは可能
	if (player->InputButtonDown(Player::InputState::Jump)) {
		player->ChangeState(Player::State::JumpStart);
	}
	// アニメーション終了後
	if (!player->GetModel()->IsPlayAnimation()) {
		player->ChangeState(Player::State::Idle);
	}
}

/************************************
	StateDamage : ダメージ
************************************/
void StateDamage::Init()
{
	player->PlayAnimation(Player::Animation::Damage, false);
}

void StateDamage::Update(float elapsedTime)
{
	// ダメージ受けている間は移動入力不可
	 
	// アニメーション終了後
	if (!player->GetModel()->IsPlayAnimation()) {
		player->ChangeState(Player::State::Idle);
	}
}

/************************************
	StateDead : 死亡
************************************/
void StateDead::Init()
{
	player->PlayAnimation(Player::Animation::Death, false);
}

void StateDead::Update(float elapsedTime)
{
	// Deathアニメーションは起き上がりまでなので途中で止めるのが良い?
	// アニメーション終了後、現段階はIdleに移行
	if (!player->GetModel()->IsPlayAnimation())
	{
		// 現段階は自動的に回復する
		player->SetHealth(player->GetMaxHealth());
		player->ChangeState(Player::State::Idle);
	}
}

/************************************
	StateRecover : 回復
************************************/
void StateRecover::Init()
{
	player->PlayAnimation(Player::Animation::JumpEnd, false);
	player->PlayEffect(Player::EffectNumber::Recovery, player->GetTargetPlayer()->GetPosition(), 0.6f);
}

void StateRecover::Update(float elapsedTime)
{
	if (player->GetModel()->IsPlayAnimation()) return;

	// 回復
	player->GetTargetPlayer()->AddHealth(recover_add_health);

	// PlayerAIの逃げ解除
	if (player->GetTargetPlayer()->GetSerialNumber() == PlayerAI::Instance().GetSerialNumber())
	{
		if (!PlayerAI::Instance().GetHpWorning())
		{
			PlayerAI::Instance().SetRanAwayFromEnemy(false);
			PlayerAI::Instance().SetEnableShowMessage(Player::PlayerMessage::RanAway, false);
		}
	}
	// 移行
	player->ChangeState(Player::State::Idle);
}

/************************************
	StateAttackHammer1 : 攻撃ハンマー
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
		//攻撃当たり判定時間
		if (animationTime >= attack_time_hammer1_start)
		{
			player->CollisionWeaponsVsEnemies(Hammer);
		}
		//任意のアニメーション再生区間でのみ次の攻撃技を出すようにする
		if (player->InputButtonDown(Player::InputState::Attack)
			&& IsDuringTime(animationTime, next_attack_time_hammer1to2))
		{
			player->ChangeState(Player::State::AttackHammer2);
		}

		// 近距離攻撃時の角度矯正
		player->ForceTurnByAttack(elapsedTime);
	}
	else {
		player->ChangeState(Player::State::Idle);
	}
}

/************************************
	StateAttackHammer2 : 攻撃ハンマー
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
	StateAttackHammerJump : ジャンプ攻撃ハンマー
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
		// 移動しながらの攻撃の場合、xz方向に一定速度を加え続ける
		player->HorizontalVelocityByAttack(false, attack_hammerJ_velocity, elapsedTime);
	}
	if (!Hammer.flagJump)
	{
		player->SetMoveAttack(false);
		player->ChangeState(Player::State::Idle);
	}

	//ジャンプ攻撃中は重力を無視する
	player->SetVelocity(FLT_MAX, 0);
}

/************************************
	StateAttackSpear1 : 攻撃スピアー
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
		// 武器出現アニメーション再生区間
		Spear.flag1 = IsDuringTime(animationTime, next_attack_time_spear1to2);
		// 足を踏ん張る際の前進をここで行う 既に何か進んでいる時はこの処理をしない
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
	StateAttackSpear2 : 攻撃スピアー
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
			// 足を踏ん張る際の前進
			player->HorizontalVelocityByAttack(true, attack_spear2_add_velocity, elapsedTime);
		}
		// 武器出現アニメーション再生区間
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
	StateAttackSpear3 : 攻撃スピアー
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
		// 足を踏ん張る際の前進をここで行う
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
	StateAttackSpearJump : ジャンプ攻撃スピアー
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
		// 回転しながら前下方向に突き刺していくアニメーション
		if (IsDuringTime(animationTime, add_velocity_time_spearJ))
		{
			player->HorizontalVelocityByAttack(false, attack_spearJ_velocity, elapsedTime);
			player->PlayEffect(Player::EffectNumber::BlueVortex, Spear.position, attack_effect_size);
		}
	}
	else {
		player->ChangeState(Player::State::Idle);
	}
	/**** 重力矯正を[UpdateVerticalVelocity]にて行う ****/
}

/************************************
	StateAttackSword1 : 攻撃ソード
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
		// 足を踏ん張る際の前進をここで行う 既に何か進んでいる時はこの処理をしない
		if (animationTime < add_velocity_time_sword1_end && !player->InputMove(elapsedTime))
		{
			player->HorizontalVelocityByAttack(true, attack_sword1_add_velocity, elapsedTime);
		}
		// 任意のアニメーション再生区間でのみ次の攻撃技を出すようにする
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
	StateAttackSword2 : 攻撃ソード
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
		// 足を踏ん張る際の前進をここで行う
		if (animationTime < add_velocity_time_sword2_end)
		{
			player->HorizontalVelocityByAttack(true, attack_sword1_add_velocity, elapsedTime);
		}
		// 任意のアニメーション再生区間でのみ次の攻撃技を出すようにする
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
	StateAttackSword3 : 攻撃ソード
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

		// 足を踏ん張る際の前進をここで行う
		if (IsDuringTime(animationTime, add_velocity_time_sword3))
		{
			player->HorizontalVelocityByAttack(true, attack_sword3_add_velocity, elapsedTime);
			// Y軸方向へ跳ねる
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
	StateAttackSwordJump : ジャンプ攻撃ソード
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

	//ジャンプ攻撃中は重力を無視する
	player->SetVelocity(FLT_MAX, 0);
}


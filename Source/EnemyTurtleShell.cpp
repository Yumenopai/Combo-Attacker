#include "EnemyTurtleShell.h"
#include "EnemyConst.h"
#include "Graphics/Graphics.h"
#include "PlayerManager.h"
#include "Collision.h"
#include "MathScript.h"

//コンストラクタ
EnemyTurtleShell::EnemyTurtleShell()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	model = std::make_unique<Model>(device, "Data/Model/RPG_TurtleShell/TurtleShellPBR.fbx", 0.01f);
	eye_ball_node_name = "Eyeball";

	angle.y = Math::RandomRange(-360, 360);
	radius = turtle_radius;
	height = turtle_height;
	health = maxHealth = turtle_max_health;

	attackDamage = turtle_attack_damage;
	//待機ステートへ遷移
	TransitionState(State::Idle);
}

//プレイヤー索敵
bool EnemyTurtleShell::SearchPlayer()
{
	XMVECTOR pPos = XMLoadFloat3(&targetPosition);
	XMVECTOR ePos = XMLoadFloat3(&position);
	float dist = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(pPos, ePos)));

	if (dist < slime_search_range * slime_search_range)
	{
		return true;
	}
	return false;
}

//遷移時アニメーション再生
void EnemyTurtleShell::TransitionPlayAnimation(State nowState)
{
	Animation anime = Anim_IdleNormal; //アニメーション設定
	bool animeLoop = true;

	switch (nowState)
	{
	case State::Idle:
		anime = Anim_IdleNormal;
		break;
	case State::Wander:
		anime = Anim_WalkFWD;
		break;
	case State::Pursuit:
		anime = Anim_RunFWD;
		break;
	case State::Attack:
		anime = Anim_Attack1;
		animeLoop = false;
		break;
	case State::IdleBattle:
		anime = Anim_IdleBattle;
		break;
	case State::HitDamage:
		anime = Anim_GetHit;
		animeLoop = false;
		break;
	}

	model->PlayAnimation(static_cast<int>(anime), animeLoop); //アニメーション再生
}

// 各ステージごとの更新処理
void EnemyTurtleShell::UpdateEachState(float elapsedTime)
{
	switch (state)
	{
	case State::Wander:
		UpdateWanderState(elapsedTime);
		break;
	case State::Idle:
		UpdateIdleState(elapsedTime);
		break;
	case State::Pursuit:
		UpdatePursuitState(elapsedTime);
		break;
	case State::Attack:
		UpdateAttackState(elapsedTime);
		break;
	case State::IdleBattle:
		UpdateIdleBattleState(elapsedTime);
		break;
	case State::HitDamage:
		UpdateHitDamageState(elapsedTime);
		break;
	}
}

//追跡ステート更新処理
void EnemyTurtleShell::UpdatePursuitState(float elapsedTime)
{
	if (FirstAttacker != nullptr)
	{
		targetPosition = FirstAttacker->GetPosition();
		MoveToTarget(elapsedTime);
	}

	EnemySlime::UpdatePursuitState(elapsedTime);
}

//ダメージ時に呼ばれる
void EnemyTurtleShell::OnDamaged()
{
	//ダメージステートへ遷移
	if (health % 4 == 0) { // ダメージステートへの遷移はランダムっぽくする
		TransitionState(State::HitDamage);
	}
}
//死亡した時に呼ばれる
void EnemyTurtleShell::OnDead()
{
	// とどめを刺したプレイヤーに武器を与える
	LastAttacker->AddHaveWeapon();
	LastAttacker->SetEnableShowMessage(Player::PlayerMessage::WeaponGet, true);

	int i = 0;
	// 与えたダメージ量が少なすぎるとLevelをあげない
	for (Player* player : PlayerManager::Instance().players)
	{
		if (static_cast<float>(attackedDamage[i]) / maxHealth > add_level_min_damage_rate) {
			player->AddLevel(turtle_add_level_up);
		}
		i++;
	}
	// 最もダメージを与えたプレイヤーはさらにLevelUp
	GetMostAttackPlayer()->AddLevel(most_attack_bonus_level_up);

	// 死亡時エフェクト再生
	PlayEffect(EffectNumber::dead, position, turtle_dead_effect_size);

	//自身を破棄
	Destroy();
}
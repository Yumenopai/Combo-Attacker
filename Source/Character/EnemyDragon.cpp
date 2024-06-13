#include "EnemyDragon.h"
#include "Graphics/Graphics.h"
#include "SceneManager.h"
#include "SceneLoading.h"
#include "SceneClear.h"

//コンストラクタ
EnemyDragon::EnemyDragon()
{
	radius = dragon_radius;
	height = dragon_height;
	health = maxHealth = dragon_max_health;
	attackDamage = dragon_attack_damage;

	// 共通初期処理
	EnemyBoss::Init();
}

//デストラクタ
EnemyDragon::~EnemyDragon()
{
}

//遷移時アニメーション再生
void EnemyDragon::TransitionPlayAnimation(State nowState)
{
	//アニメーション設定
	Animation anime = Animation::Idle1;
	bool animeLoop = true;

	switch (nowState)
	{
	case State::Idle:
		anime = Animation::Idle1;
		break;
	case State::Wander:
		anime = Animation::Walk;
		break;
	case State::Pursuit:
		anime = Animation::Run;
		break;
	case State::Attack:
		anime = Animation::Attack01;
		animeLoop = false;
		break;
	case State::IdleBattle:
		anime = Animation::Idle02;
		break;
	case State::Scream:
		anime = Animation::Scream;
		animeLoop = false;
		break;
	case State::AttackClaw:
		anime = Animation::AttackClaw;
		animeLoop = false;
		break;
	case State::GetHit:
		anime = Animation::GetDamage;
		animeLoop = false;
		break;
	case State::Die:
		anime = Animation::Die;
		animeLoop = false;
		break;
	}

	//アニメーション再生
	model->PlayAnimation(static_cast<int>(anime), animeLoop);
}

//死亡ステート更新処理
void EnemyDragon::UpdateDieState(float elapsedTime)
{
	// 基底処理
	EnemyBoss::UpdateDieState(elapsedTime);

	// アニメーション終了後
	if (!model->IsPlayAnimation())
	{
		// クリア画面へ遷移
		SceneManager::Instance().ChangeScene(new SceneLoading(new SceneClear(), -255));
	}
}

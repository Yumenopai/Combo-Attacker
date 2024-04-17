#include "EnemyTurtleShell.h"
#include "Graphics/Graphics.h"
#include "PlayerManager.h"
#include "Collision.h"
#include "MathScript.h"

//コンストラクタ
EnemyTurtleShell::EnemyTurtleShell()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	model = std::make_unique<Model>(device, "Data/Model/RPG_TurtleShell/TurtleShellPBR.fbx", 0.01f);

	angle.y = Math::RandomRange(-360, 360);
	//radius = 0.5f;
	height = 1.0f;
	//待機ステートへ遷移
	TransitionState(State::Idle);
}

//プレイヤー索敵
bool EnemyTurtleShell::SearchPlayer()
{
	XMVECTOR pPos = XMLoadFloat3(&targetPosition);
	XMVECTOR ePos = XMLoadFloat3(&position);
	float dist = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(pPos, ePos)));

	if (dist < searchRange * searchRange)
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
//ダメージ時に呼ばれる
void EnemyTurtleShell::OnDamaged()
{
	//ダメージステートへ遷移
	TransitionState(State::HitDamage);
}
//死亡した時に呼ばれる
void EnemyTurtleShell::OnDead()
{
	// 死亡時エフェクト再生
	PlayEffect(EffectNumber::dead, position, 0.6f);

	//自身を破棄
	Destroy();

	// アイテムゲット
	// 現在持っていない武器リスト取得
	auto remainList = Player1P::Instance().GetHaventArm();
	// 残り出現個数
	int remainCount = static_cast<int>(remainList.size());

	if (remainCount == 0) {
		return;
	}
	else {
		// 乱数
		int num = rand() % remainCount;

		int i = 0;
		// 獲得する武器を確定する
		for (const auto& arm : remainList)
		{
			if (i == num)
			{
				Player1P::Instance().AddHaveArm(arm);
				return;
			}
			i++;
		}
	}
}
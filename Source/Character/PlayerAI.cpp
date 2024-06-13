#include "PlayerAI.h"
#include "Player1P.h"
#include "Core/Enemy.h"

static PlayerAI* instance = nullptr;
//インスタンス取得
PlayerAI& PlayerAI::Instance()
{
	return *instance;
}

//コンストラクタ
PlayerAI::PlayerAI()
{
	//インスタンスポインタ設定
	instance = this;

	//初期装備
	InitialWeapon = AttackType::Spear;

	// 初期化
	Player::Init();

	position = ai_initial_position;
	angle = initial_angle;

	turn_speed = ai_turn_speed;

	playerName = ai_Name;
	serialNumber = ai_serial_number;
	nameColor = ai_name_color;
	//UI
	hpGaugePosition_Y = ai_hp_gauge_position_y;

	// ボタンステート初期化
	oldInput = nowInput = nextInput = InputState::None;
	currentEnemySearch = EnemySearch::None;
}

PlayerAI::~PlayerAI()
{
}

//更新
void PlayerAI::Update(float elapsedTime)
{
	if (targetPlayer == nullptr) {
		targetPlayer = &Player1P::Instance();
	}

	// ボタンステート初期化
	oldInput = nowInput;
	nowInput = nextInput;
	nextInput = InputState::None;

	UpdateEnemyDistance(elapsedTime);
	AttackUpdate();
	UpdateUtils(elapsedTime);
}

// 攻撃時の更新処理
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

	// 攻撃ステートの場合のみ処理する
	if (currentEnemySearch != EnemySearch::Attack) return;

	// 50ダメージ以上与えたかつ、80%以上が自身の攻撃している
	ranAwayFromPlayer1P = (allDamage > ai_ran_away_min_damage
		&& (100 * allDamage / (Player1P::Instance().GetAllDamage() + allDamage)) > ai_ran_away_damage_rate/*%*/); // 自身が与えた全ダメージ量(%で示すため100を掛ける)/全員で与えた全ダメージ量
	SetEnableShowMessage(Player::PlayerMessage::Indifference, ranAwayFromPlayer1P);

	// 自身のダメージが残り僅かなら逃げる
	if (!ranAwayFromEnemy && GetHpWorning())
	{
		ranAwayFromEnemy = true;
		SetEnableShowMessage(Player::PlayerMessage::RanAway, ranAwayFromEnemy);
	}

	// とどめを1Pに譲る
	if (currentAttackEnemy != nullptr // 現在攻撃中
		&& currentAttackEnemy->GetHealthRate() < ai_enemy_few_remain_damage_rate // 敵のダメージがわずか
		&& currentAttackEnemy->GetAttackedDamagePersent(Player1P::Instance().GetSerialNumber()) > ai_concede_finish_min_damage_rate // 1Pが最低限攻撃している
		&& Player1P::Instance().GetHaveWeaponCount() <= this->GetHaveWeaponCount() // 1Pの武器所持数がAIの武器数より少ない
		&& lastAvoidEnemy != currentAttackEnemy && waitTimer == 0) // 重複処理防止/タイマーが動いていない
	{
		currentAvoidEnemy = currentAttackEnemy;
		SetEnableShowMessage(Player::PlayerMessage::Attack,true);
		waitTimer++;
		return;
	}

	// 攻撃入力
	if (!ranAwayFromEnemy 
		&& nearestEnemy != currentAvoidEnemy
		&& nowInput != InputState::Attack) //長押しでないので今が攻撃の場合を除く
	{
		nextInput = InputState::Attack;
	}
}

// 移動ベクトル
XMFLOAT3 PlayerAI::GetMoveVec() const
{
	XMFLOAT3 moveVec = {};		// 移動ベクトル(return値)

	XMFLOAT3 playerPos = Player1P::Instance().GetPosition();
	// プレイヤーへのベクトル
	XMVECTOR playerVec = XMVectorSubtract(XMLoadFloat3(&playerPos), XMLoadFloat3(&position));
	float playerDistSq = XMVectorGetX(XMVector3LengthSq(playerVec));

	// プレイヤーからの逃避
	if (ranAwayFromPlayer1P)
	{
		// プレイヤー近い時
		if (playerDistSq < ai_ran_away_from_p1_dist * ai_ran_away_from_p1_dist)
		{
			// 一番近いプレイヤーから逃げるように動く
			XMStoreFloat3(&moveVec, playerVec);
			moveVec.x = -moveVec.x;
			moveVec.z = -moveVec.z;
		}
		else
		{
			// 一番近い敵に勝手に移動する
			moveVec = nearestVec;
			// 1Pが攻撃している敵だったら2番目に近い所に移動
			if (nearestEnemy == targetPlayer->GetCurrentAttackEnemy())
			{
				moveVec = secondDistEnemyVec;
			}
		}
		return moveVec;
	}

	// 回復に向かう
	bool readyRecover = targetPlayer->GetHpWorning();
	// 一定距離から離れている場合 or ターゲット回復が必要な時
	if (playerDistSq > ai_player_follow_dist * ai_player_follow_dist || readyRecover)
	{
		// 移動時はプレイヤーの斜め後ろ辺りに付かせる
		XMFLOAT3 playerAng = Player1P::Instance().GetAngle();
		playerPos.x -= sinf(playerAng.y - 45/*°*/) * 2; // 45度後ろの2倍距離
		playerPos.z -= cosf(playerAng.y - 45/*°*/) * 2; // 45度後ろの2倍距離
		playerPos.y = position.y; // Y方向は自身の高さで良い
		// 目標位置へのベクトル
		playerVec = XMVectorSubtract(XMLoadFloat3(&playerPos), XMLoadFloat3(&position));
		// 移動ベクトルを更新
		XMStoreFloat3(&moveVec, playerVec);

		// 回復の場合はここで返す
		if (readyRecover) return moveVec;
	}

	// 敵から逃げる
	if (ranAwayFromEnemy)
	{
		if (nearestDist < ai_ran_away_from_enemy_dist)
		{
			// 一番近い敵から逃げるように動く
			moveVec = nearestVec;
			moveVec.x = -moveVec.x;
			moveVec.z = -moveVec.z;

			XMFLOAT3 playerVecF3 = {};
			XMStoreFloat3(&playerVecF3, playerVec);
			moveVec = { playerVecF3.x + moveVec.x, moveVec.y, playerVecF3.z + moveVec.z };
		}
	}
	// 攻撃を任せる敵がいる場合
	else if (currentAvoidEnemy != nullptr && nearestEnemy == currentAvoidEnemy)
	{
		if (secondDist < ai_go_toward_enemy_dist)
		{
			moveVec = secondDistEnemyVec;
		}
	}
	// 最近エネミーとの距離が近距離の場合、進行ベクトルを更新
	else if (nearestDist < ai_go_toward_enemy_dist)
	{
		moveVec = nearestVec;
	}

	return moveVec;
}

// ===========入力処理===========
// ボタン判定
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

// 武器変更処理
void PlayerAI::InputChangeWeapon(AttackType weapon/* = AttackType::None*/)
{
	// 指定されていたらそれを設定する
	if (weapon != AttackType::None) {
		CurrentUseWeapon = weapon;
		return;
	}

	// 自身のHPがあやういかつ、ボタンが押されているときの処理
	if (GetHpWorning()) return;
	if (!Player1P::Instance().InputButtonDown(Player::InputState::Buddy)) return;

	// 次に所持しているものを選択する
	CurrentUseWeapon = GetNextWeapon();
}

// ターゲット回復処理
void PlayerAI::InputRecover()
{
	//回復遷移可能ならステート移行
	if (enableRecoverTransition)
	{
		ChangeState(State::Recover);
	}
}
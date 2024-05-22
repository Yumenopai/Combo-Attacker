#include "PlayerAI.h"
#include "Player1P.h"
#include "Enemy.h"

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
	InitialArm = AttackType::Spear;

	// 初期化
	Player::Init();

	position = { -7,5,-60 };
	turnSpeed = XMConvertToRadians(360);

	characterName = " BUDDY";
	nameColor = { 1.0f, 1.0f, 1.0f, 1.0f };
	//UI
	hpGuage_Y = 620.0f;
	hpColorNormal = { 0.2f, 0.6f , 0.2f, 1.0f };
	hpColorWorning = { 0.6f, 0.2f, 0.2f, 1.0f };

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
		if (waitTimer > 80)
		{
			waitTimer = 0;
			avoidEnemy = nullptr;
			SetEnableShowMessage(Player::MessageNotification::Attack, false);
		}
	}

	// 攻撃ステートの場合のみ処理する
	if (currentEnemySearch != EnemySearch::Attack) return;

	// 50ダメージ以上与えたかつ、80%以上が自身の攻撃している
	ranAwayFromPlayer1P = (allDamage > 50
		&& (100 * allDamage / (Player1P::Instance().GetAllDamage() + allDamage)) > 80/*%*/);
	SetEnableShowMessage(Player::MessageNotification::Indifference, ranAwayFromPlayer1P);

	// 自身のダメージが残り僅か
	ranAwayFromEnemy = GetHpWorning();
	SetEnableShowMessage(Player::MessageNotification::RanAway, ranAwayFromEnemy);

	// 敵ダメージが残り僅かで、1Pの武器獲得がまだの時
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

	// 攻撃入力
	if (!ranAwayFromEnemy 
		&& nearestEnemy != avoidEnemy
		&& nowInput != InputState::Attack) //長押しでないので今が攻撃の場合を除く
	{
		nextInput = InputState::Attack;
	}
}

// 移動ベクトル
XMFLOAT3 PlayerAI::GetMoveVec() const
{
	const float playerFollowRange = 2.5f;	// 味方についていく判定距離
	XMFLOAT3 moveVec = {};		// 移動ベクトル(return値)

	XMFLOAT3 playerPos = Player1P::Instance().GetPosition();
	XMFLOAT3 playerAng = Player1P::Instance().GetAngle();

	// プレイヤーへのベクトル
	XMVECTOR playerVec = XMVectorSubtract(XMLoadFloat3(&playerPos), XMLoadFloat3(&position));
	float playerDist = XMVectorGetX(XMVector3LengthSq(playerVec));

	// プレイヤーからの逃避
	if (ranAwayFromPlayer1P)
	{
		// プレイヤー近い時
		if (playerDist < 10.0f)
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
	if (playerDist > playerFollowRange * playerFollowRange || readyRecover)
	{
		// 移動時はプレイヤーの斜め後ろ辺りに付かせる
		playerPos.x -= sinf(playerAng.y - 45) * 2;
		playerPos.z -= cosf(playerAng.y - 45) * 2;
		playerPos.y = position.y; //Y方向は自身の高さで良い
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
		if (nearestDist < 4.0f)
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
	else if (avoidEnemy != nullptr && nearestEnemy == avoidEnemy)
	{
		if (secondDist < 10.0f)
		{
			moveVec = secondDistEnemyVec;
		}
	}
	// 最近エネミーとの距離が近距離の場合、進行ベクトルを更新
	else if (nearestDist < 10.0f)
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
void PlayerAI::InputChangeArm(AttackType arm/* = AttackType::None*/)
{
	// 指定されていたらそれを設定する
	if (arm != AttackType::None) {
		CurrentUseArm = arm;
		return;
	}

	// 自身のHPがあやういかつ、ボタンが押されているときの処理
	if (GetHpWorning()) return;
	if (!Player1P::Instance().InputButtonDown(Player::InputState::Buddy)) return;

	// 次に所持しているものを選択する
	CurrentUseArm = GetNextArm();
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
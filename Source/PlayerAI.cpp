#include "PlayerAI.h"
#include "Player1P.h"

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

	// 初期化
	Player::Init();

	position = { -7,5,-60 };

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
	// ボタンステート初期化
	oldInput = nowInput;
	nowInput = nextInput;
	nextInput = InputState::None;

	UpdateEnemyDistance(elapsedTime);

	// 攻撃入力
	if (currentEnemySearch == EnemySearch::Attack && nowInput != InputState::Sword) //長押しでないので今がソードの場合を除く
	{
		nextInput = InputState::Sword;
	}

	UpdateUtils(elapsedTime);
}

// HP描画
void PlayerAI::HPBarRender(const RenderContext& rc, Sprite* gauge)
{
	Player::HPBarRender(rc, gauge, false);
}

// 移動ベクトル
XMFLOAT3 PlayerAI::GetMoveVec() const
{
	XMFLOAT3 moveVec = {};

	XMFLOAT3 playerPos = Player1P::Instance().GetPosition();
	XMFLOAT3 playerAng = Player1P::Instance().GetAngle();
	// 常にプレイヤーの斜め後ろ辺りに付かせる
	playerPos.x -= sinf(playerAng.y - 45) * 2;
	playerPos.z -= cosf(playerAng.y - 45) * 2;
	playerPos.y = position.y; //Y方向は自身の高さで良い

	// プレイヤーへのベクトル
	XMVECTOR AIto1P = XMVectorSubtract(XMLoadFloat3(&playerPos), XMLoadFloat3(&position));
	// 一定距離離れた時、進行ベクトルを更新
	if (XMVectorGetX(XMVector3LengthSq(AIto1P)) > 0.2f * 0.2f)
	{
		XMStoreFloat3(&moveVec, AIto1P);
	}

	// プレイヤーが発見ステート時
	if (Player1P::Instance().GetEnemySearch() >= EnemySearch::Find)
	{
		// 最近エネミーとの距離が近距離の場合、進行ベクトルを更新
		if (nearestDist < 10.0f) moveVec = nearestVec;
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
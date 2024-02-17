#include <map>

#include "PlayerAI.h"
#include "Graphics/Graphics.h"
#include "Graphics/PrimitiveRenderer.h"
#include "Input/Input.h"
#include "SceneManager.h"
#include "SceneLoading.h"
#include "SceneGame.h"
#include "SceneClear.h"
#include "EnemyManager.h"

#include "Stage.h"
#include "imgui.h"

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

	ID3D11Device* device = Graphics::Instance().GetDevice();
	//プレイヤーモデル読み込み
	model = std::make_unique<Model>(device, "Data/Model/SD-UnityChan/UnityChan.fbx", 0.02f);

	// 初期化
	Player::Init();

	position = { -7,5,-60 };

	// ボタンステート初期化
	oldInput = nowInput = nextInput = InputState::None;
	ESState = EnemySearch::None;
}

PlayerAI::~PlayerAI()
{
}

//更新
void PlayerAI::Update(float elapsedTime)
{
	EnemyManager& enemyManager = EnemyManager::Instance();
	int enemyCount = enemyManager.GetEnemyCount();

	// ボタンステート初期化
	oldInput = nowInput;
	nowInput = nextInput;
	nextInput = InputState::None;
	nearestEnemy = nullptr;
	nearestDist = FLT_MAX;
	nearestVec = {};

	//if (enemyCount == 0) ESState = EnemySearch::None;
	if (enemyCount == 0)
	{
		//SceneManager::Instance().ChangeScene(new SceneLoading(new SceneGame(), -255));
	}

	for (int i = 0; i < enemyCount; i++)
	{
		Enemy* enemy = enemyManager.GetEnemy(i);
		// それぞれのエネミーの距離判定
		XMVECTOR PosPlayer = XMLoadFloat3(&GetPosition());
		XMVECTOR PosEnemy = XMLoadFloat3(&enemy->GetPosition());
		
		XMVECTOR DistVec = XMVectorSubtract(PosEnemy, PosPlayer);
		float dist = XMVectorGetX(XMVector3Length(DistVec));

		enemyDist[enemy] = dist; //各エネミーとの距離等を更新毎に記録する

		if (dist < playerVSenemyJudgeDist[(int)EnemySearch::Attack])
			enemySearch[enemy] = EnemySearch::Attack;
		else if (dist < playerVSenemyJudgeDist[(int)EnemySearch::Find])
			enemySearch[enemy] = EnemySearch::Find;
		else
			enemySearch[enemy] = EnemySearch::None;

		/***********************/

		if (dist < nearestDist) //最近エネミーの登録
		{
			nearestEnemy = enemy;
			nearestDist = dist;
			ESState = enemySearch[enemy];
			XMStoreFloat3(&nearestVec, DistVec);
		}
	}

	if (ESState == EnemySearch::Attack && nowInput != InputState::Sword) //長押しでないので今がソードの場合を除く
		nextInput = InputState::Sword;

	// 配列ズラし
	//ShiftTrailPositions();

	// ステート毎に中で処理分け
	//UpdateEachState(elapsedTime);
	stateMachine->Update(elapsedTime);

	// 剣の軌跡描画更新処理
	//RenderTrail();

	// 攻撃中じゃなければジャンプ処理
	if (Atype == AttackType::None) UpdateJump(elapsedTime);

	//プレイヤーとエネミーとの衝突処理
	CollisionPlayerVsEnemies();

	//速力処理更新
	UpdateVelocity(elapsedTime);

	//オブジェクト行列更新
	UpdateTransform();

	//モデルアニメーション更新処理
	model->UpdateAnimation(elapsedTime);

	//モデル行列更新
	model->UpdateTransform(transform);
}

//ジャンプ処理
void Player::UpdateJump(float elapsedTime)
{
	switch (jumpTrg)
	{
	case JumpState::CanJump:
		// 押した時の処理
		if (InputJumpButtonDown())
		{
			velocity.y = 500.0f;
			jumpTrg = JumpState::CanDoubleJump;
		}
		break;

	case JumpState::CanDoubleJump:
		// 2段目ジャンプは高さ調節不可
		if (InputJumpButtonDown())
		{
			velocity.y += 15.0f;
			jumpTrg = JumpState::CannotJump;
		}
		// 一段目ジャンプ中の攻撃ボタン
		else if (InputAttackFromJump(elapsedTime))
		{
			jumpTrg = JumpState::CannotJump;
		}

		//break;
		// fall through
	case JumpState::CannotJump:

		// ジャンプ可能状態の時のみ通らない
		// 着地時(地面に立っている時は常時処理)
		if (isGround)
		{
			// 着地時に押しっぱの場合は処理されないようにする
			if (InputJumpButtonDown()) jumpTrg = JumpState::CannotJump;
			// 押されていない時は地面にいるのでジャンプ可能状態にする
			else jumpTrg = JumpState::CanJump;
		}
		break;
	}
}

//描画
void PlayerAI::ShadowRender(const RenderContext& rc, ShadowMap* shadowMap)
{
	shadowMap->Draw(rc, model.get());
}
//描画
void PlayerAI::Render(const RenderContext& rc, ModelShader* shader)
{
	shader->Draw(rc, model.get());
}

// 攻撃の軌跡描画
void PlayerAI::PrimitiveRender(const RenderContext& rc)
{
	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();

	// ポリゴン描画
	primitiveRenderer->Render(rc.deviceContext, rc.camera->GetView(), rc.camera->GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}
// 攻撃の軌跡描画
void PlayerAI::HPBarRender(const RenderContext& rc, Sprite* gauge)
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	//HPゲージの長さ
	const float guageWidth = 700.0f;
	const float guageHeight = 15.0f;

	float healthRate = GetHealth() / static_cast<float>(GetMaxHealth());
	bool hpWorning = healthRate < 0.2f;
	int frameExpansion = 6;
	Graphics& graphics = Graphics::Instance();
	float screenWidth = static_cast<float>(graphics.GetScreenWidth());

	//ゲージ描画(下地)
	gauge->Render(dc,
		(screenWidth / 2) - (guageWidth / 2),
		580.0f,
		0,
		guageWidth + frameExpansion,
		guageHeight + frameExpansion,
		0, 0,
		static_cast<float>(gauge->GetTextureWidth()),
		static_cast<float>(gauge->GetTextureHeight()),
		0.0f,
		0.3f, 0.3f, 0.3f, 0.8f
	);
	//ゲージ描画
	gauge->Render(dc,
		(screenWidth / 2) - (guageWidth / 2) + frameExpansion / 2,
		580.0f + frameExpansion / 2,
		0,
		guageWidth * healthRate,
		guageHeight,
		0, 0,
		static_cast<float>(gauge->GetTextureWidth()),
		static_cast<float>(gauge->GetTextureHeight()),
		0.0f,
		hpWorning ? 0.8f : 0.2f, hpWorning ? 0.2f : 0.6f, 0.2f, 1.0f
	);
}


//ジャンプ処理
void PlayerAI::UpdateJump(float elapsedTime)
{

}

// ===========入力処理===========
//移動入力処理
bool PlayerAI::InputMove(float elapsedTime)
{
	XMFLOAT3 moveVec = {};
	XMFLOAT3 playerPos = Player1P::Instance().GetPosition();
	XMFLOAT3 playerAng = Player1P::Instance().GetAngle();
	// 常にプレイヤーの斜め後ろ辺りに付かせる
	playerPos.x -= sinf(playerAng.y-45) * 2;
	playerPos.z -= cosf(playerAng.y-45) * 2;
	playerPos.y = position.y; //Y方向は自身の高さで良い

	XMVECTOR AIto1P = XMVectorSubtract(XMLoadFloat3(&playerPos), XMLoadFloat3(&position));
	if (XMVectorGetX(XMVector3LengthSq(AIto1P)) > 0.2f * 0.2f)
		XMStoreFloat3(&moveVec, AIto1P);

	if (Player1P::Instance().GetESState() >= EnemySearch::Find)
	{
		if (nearestDist < 10.0f) moveVec = nearestVec;
		else XMStoreFloat3(&moveVec, AIto1P);
	}						

	//移動処理
	Move(moveVec.x, moveVec.z, moveSpeed);
	//旋回処理
	Turn(elapsedTime, moveVec.x, moveVec.z, turnSpeed);

	//進行ベクトルがゼロベクトルでない場合は入力された
	return moveVec.x != 0 || moveVec.y != 0 || moveVec.z != 0;
}

// ジャンプボタンが押されたか
bool PlayerAI::InputJumpButtonDown()
{
	return InputButtonDown(InputState::Jump);
}
bool PlayerAI::InputJumpButton()
{
	return InputButton(InputState::Jump);
}
bool PlayerAI::InputJumpButtonUp()
{
	return InputButtonUp(InputState::Jump);
}

// ハンマー攻撃ボタンが押されたか
bool PlayerAI::InputHammerButton()
{
	return InputButtonDown(InputState::Hammer);
}
// ソード攻撃ボタンが押されたか
bool PlayerAI::InputSwordButton()
{
	return InputButtonDown(InputState::Sword);
}

// スピアー攻撃ボタンが押されたか
bool PlayerAI::InputSpearButton()
{
	return InputButtonDown(InputState::Spear);
}

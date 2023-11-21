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
	//model = std::make_unique<Model>(device, "Data/Model/Enemy/red.fbx", 0.02f);

	//初期化
	enemySearch.clear();
	EnemyManager& enemyManager = EnemyManager::Instance();
	int enemyCount = enemyManager.GetEnemyCount();//全ての敵と総当たりで衝突処理
	for (int i = 0; i < enemyCount; i++)
	{
		enemySearch[enemyManager.GetEnemy(i)] = EnemySearch::None;
	}

	position = { -7,5,-60 };
	health = 100;
	maxHealth = 100;

	//待機ステートへ遷移
	TransitionIdleState();

	oldInput = nowInput = nextInput = InputState::None;
	ESState = EnemySearch::None;

	//ヒットエフェクト読み込み
	hitEffect = std::make_unique<Effect>("Data/Effect/Hit.efk");
}

PlayerAI::~PlayerAI()
{
}

//更新
void PlayerAI::Update(float elapsedTime, int remine)
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
		SceneManager::Instance().ChangeScene(new SceneLoading(new SceneGame(3), -255));

	for (int i = 0; i < enemyCount; i++)
	{
		Enemy* enemy = enemyManager.GetEnemy(i);
		// それぞれのエネミーの距離判定
		XMVECTOR PosPlayer = XMLoadFloat3(&GetPosition());
		XMVECTOR PosEnemy = XMLoadFloat3(&enemy->GetPosition());
		
		XMVECTOR DistVec = XMVectorSubtract(PosEnemy, PosPlayer);
		float dist = XMVectorGetX(XMVector3Length(DistVec));

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

	if (ESState == EnemySearch::Attack && nowInput != InputState::Sword) nextInput = InputState::Sword;
	// 配列ズラし
	//ShiftTrailPositions();

	// ステート毎に中で処理分け
	UpdateEachState(elapsedTime);

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
	case CanJump:
		// 押した時の処理
		if (InputJumpButtonDown())
		{
			velocity.y = 500.0f;
			jumpTrg = CanDoubleJump;
		}
		break;

	case CanDoubleJump:
		// 2段目ジャンプは高さ調節不可
		if (InputJumpButtonDown())
		{
			velocity.y += 15.0f;
			jumpTrg = CannotJump;
		}
		// 一段目ジャンプ中の攻撃ボタン
		else if (InputAttackFromJump(elapsedTime))
		{
			jumpTrg = CannotJump;
		}

		//break;
		// fall through
	case CannotJump:

		// ジャンプ可能状態の時のみ通らない
		// 着地時(地面に立っている時は常時処理)
		if (isGround)
		{
			// 着地時に押しっぱの場合は処理されないようにする
			if (InputJumpButtonDown()) jumpTrg = CannotJump;
			// 押されていない時は地面にいるのでジャンプ可能状態にする
			else jumpTrg = CanJump;
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
	
	//rc.deviceContext->OMSetBlendState(renderState->GetBlendState(BlendState::Transparency), blendFactor, sampleMask);
	
#if 0

	//デバッグメニュー描画
	DebugMenu();
#endif
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
		0.2f, 0.6f, 0.2f, 1.0f
	);
}

void PlayerAI::DebugMenu()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Player", nullptr, ImGuiWindowFlags_None))
	{
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//位置
			ImGui::DragFloat3("Position", &position.x, 0.1f);
			ImGui::DragFloat3("Offset", &offset.x, 0.1f);

			//回転
			XMFLOAT3 a;
			a.x = XMConvertToDegrees(angle.x);
			a.y = XMConvertToDegrees(angle.y);
			a.z = XMConvertToDegrees(angle.z);
			ImGui::DragFloat3("Angle", &a.x, 1.0f);
			if (a.y > 360) a.y = 0;
			if (a.y < 0) a.y = 360;
			angle.x = XMConvertToRadians(a.x);
			angle.y = XMConvertToRadians(a.y);
			angle.z = XMConvertToRadians(a.z);

			//スケール
			ImGui::DragFloat3("Scale", &scale.x, 0.01f);
		}

		ImGui::End();
	}
}

//着地した時に呼ばれる
void PlayerAI::OnLanding(float elapsedTime)
{
	if (Atype != AttackType::None) //攻撃中(主にジャンプ攻撃後)
	{
		// 着地してすぐは何もさせないためここで処理を書かない
		// 各StateUpdateにてアニメーション終了後にIdleStateへ遷移する
	}
	else if (InputMove(elapsedTime)) TransitionRunState();
	else TransitionJumpEndState();
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

	XMVECTOR AIto1P = XMVectorSubtract(XMLoadFloat3(&playerPos), XMLoadFloat3(&position));
	if (XMVectorGetX(XMVector3Length(AIto1P)) > 0.2f)
		XMStoreFloat3(&moveVec, AIto1P);

	//if (XMVectorGetX(XMVector3Length(XMLoadFloat3(&GetMoveVec()))) != 0/* && nowInput != InputState::None*/)
	if (Player1P::Instance().GetESState() > EnemySearch::None)
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

//スティック入力値から移動ベクトルを取得
XMFLOAT3 PlayerAI::GetMoveVec() const
{
	//入力情報を取得
	GamePad& gamePad = Input::Instance().GetGamePad();
	float ax = gamePad.GetAxisLX();
	float ay = gamePad.GetAxisLY();

	//カメラ方向とスティックの入力値によって進行方向を計算する
	Camera& camera = Camera::Instance();
	const DirectX::XMFLOAT3& cameraRight = camera.GetRight();
	const DirectX::XMFLOAT3& cameraFront = camera.GetFront();

	//移動ベクトルはXZ平面に水平なベクトルになるようにする

	//カメラ右方向ベクトルをXZ単位ベクトルに変換
	float cameraRightX = cameraRight.x;
	float cameraRightZ = cameraRight.z;
	float cameraRightLength = sqrtf(cameraRightX * cameraRightX + cameraRightZ * cameraRightZ);
	if (cameraRightLength > 0.0f)
	{
		//単位ベクトル化
		cameraRightX /= cameraRightLength;
		cameraRightZ /= cameraRightLength;
	}

	//カメラ前方向ベクトルをXZ単位ベクトルに変換
	float cameraFrontX = cameraFront.x;
	float cameraFrontZ = cameraFront.z;
	float cameraFrontLength = sqrtf(cameraFrontX * cameraFrontX + cameraFrontZ * cameraFrontZ);
	if (cameraFrontLength > 0.0f)
	{
		//単位ベクトル化
		cameraFrontX /= cameraFrontLength;
		cameraFrontZ /= cameraFrontLength;
	}

	//スティックの水平入力値をカメラ右方向に反映し、
	//スティックの垂直入力値をカメラ前方向に反映し、進行ベクトルを計算する
	XMFLOAT3 vec;
	vec.x = cameraFrontX * ay + cameraRightX * ax;
	vec.z = cameraFrontZ * ay + cameraRightZ * ax;

	//Y軸方向には移動しない
	vec.y = 0.0f;

	return vec;
}
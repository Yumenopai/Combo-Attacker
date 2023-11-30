#include <map>

#include "Player1P.h"
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

static Player1P* instance = nullptr;

//インスタンス取得
Player1P& Player1P::Instance()
{
	return *instance;
}

//コンストラクタ
Player1P::Player1P()
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
	ESState = EnemySearch::None;

	position = { -7,5,-66 };
	health = 100;
	maxHealth = 100;

	//待機ステートへ遷移
	TransitionIdleState();

	//ヒットエフェクト読み込み
	hitEffect = std::make_unique<Effect>("Data/Effect/Hit.efk");
}

Player1P::~Player1P()
{
}

//更新
void Player1P::Update(float elapsedTime)
{
	EnemyManager& enemyManager = EnemyManager::Instance();
	int enemyCount = enemyManager.GetEnemyCount();

	nearestEnemy = nullptr;
	nearestDist = FLT_MAX;
	nearestVec = {};
	int noneEnemy = 0;
	for (int i = 0; i < enemyCount; i++)
	{
		Enemy* enemy = enemyManager.GetEnemy(i);
		// それぞれのエネミーの距離判定
		XMVECTOR PosPlayer = XMLoadFloat3(&GetPosition());
		XMVECTOR PosEnemy = XMLoadFloat3(&enemy->GetPosition());
		
		XMVECTOR DistVec = XMVectorSubtract(PosEnemy, PosPlayer);
		float dist = XMVectorGetX(XMVector3Length(DistVec));

		if (dist < playerVSenemyJudgeDist[(int)EnemySearch::Attack])
		{
			enemySearch[enemy] = EnemySearch::Attack;
		}
		else if (dist < playerVSenemyJudgeDist[(int)EnemySearch::Find])
		{
			enemySearch[enemy] = EnemySearch::Find;
		}
		else
		{
			enemySearch[enemy] = EnemySearch::None;
			noneEnemy++;
			if (i == (enemyCount - 1) && enemyCount == noneEnemy) ESState = EnemySearch::None;
			continue;
		}

		/***********************/

		if (dist < nearestDist) //最近エネミーの登録
		{
			nearestEnemy = enemy;
			nearestDist = dist;
			ESState = enemySearch[enemy];
			XMStoreFloat3(&nearestVec, DistVec);
		}
	}

	//for (auto itr = enemySearch.begin(); itr != enemySearch.end(); ++itr) 
	//{
	//	if (static_cast<int>(itr->second) <= static_cast<int>(EnemySearch::Find))
	//	{



	//	}


	//}


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

//描画
void Player1P::ShadowRender(const RenderContext& rc, ShadowMap* shadowMap)
{
	shadowMap->Draw(rc, model.get());
}
//描画
void Player1P::Render(const RenderContext& rc, ModelShader* shader)
{
	shader->Draw(rc, model.get());
	
#ifdef _DEBUG
	//デバッグメニュー描画
	DebugMenu();
#endif
}

// 攻撃の軌跡描画
void Player1P::PrimitiveRender(const RenderContext& rc)
{
	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();

	// ポリゴン描画
	primitiveRenderer->Render(rc.deviceContext, rc.camera->GetView(), rc.camera->GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}
// 攻撃の軌跡描画
void Player1P::HPBarRender(const RenderContext& rc, Sprite* gauge)
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
		555.0f,
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
		555.0f + frameExpansion / 2,
		0,
		guageWidth * healthRate,
		guageHeight,
		0, 0,
		static_cast<float>(gauge->GetTextureWidth()),
		static_cast<float>(gauge->GetTextureHeight()),
		0.0f,
		0.2f, 0.8f, 0.2f, 1.0f
	);
}

void Player1P::DebugMenu()
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
void Player1P::OnLanding(float elapsedTime)
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
void Player1P::UpdateJump(float elapsedTime)
{
	//ボタン入力でジャンプ
	GamePad& gamePad = Input::Instance().GetGamePad();

	switch (jumpTrg)
	{
	case CanJump:
		// 押している間の処理
		if (gamePad.GetButton() & GamePad::BTN_A)
		{
			velocity.y += 150 * elapsedTime;
			// 指定加速度まであがったら
			if (velocity.y > jumpSpeed)	jumpTrg = CanDoubleJump;
		}
		// 一回離した時
		else if (gamePad.GetButtonUp() & GamePad::BTN_A)
		{
			jumpTrg = CanDoubleJump;
		}
		break;

	case CanDoubleJump:
		// 2段目ジャンプは高さ調節不可
		if (gamePad.GetButtonDown() & GamePad::BTN_A)
		{
			if (velocity.y > 0) velocity.y += 15.0f;
			else				velocity.y = 15.0f;
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
			if (gamePad.GetButton() & GamePad::BTN_A) jumpTrg = CannotJump;
			// 押されていない時は地面にいるのでジャンプ可能状態にする
			else jumpTrg = CanJump;
		}
		break;
	}
}

// ===========入力処理===========
//移動入力処理
bool Player1P::InputMove(float elapsedTime)
{
	XMFLOAT3 moveVec = GetMoveVec();
	XMVECTOR MoveVec = XMLoadFloat3(&moveVec); //進行ベクトルを取得

	if (XMVectorGetX(XMVector3Length(MoveVec)) != 0 && nearestDist < FLT_MAX)
	{
		float dot = XMVectorGetX(XMVector3Dot(MoveVec, XMLoadFloat3(&nearestVec)));

		// 最近エネミーに向かう
		if (dot > 0) moveVec = nearestVec;
	}

	//移動処理
	Move(moveVec.x, moveVec.z, moveSpeed);
	//旋回処理
	Turn(elapsedTime, moveVec.x, moveVec.z, turnSpeed);

	//進行ベクトルがゼロベクトルでない場合は入力された
	return moveVec.x != 0 || moveVec.y != 0 || moveVec.z != 0;
}

// ジャンプボタンBTN_Aが押されたか
bool Player1P::InputJumpButtonDown()
{
	GamePad& gamePad = Input::Instance().GetGamePad();
	return gamePad.GetButtonDown() & GamePad::BTN_A;
}
bool Player1P::InputJumpButton()
{
	GamePad& gamePad = Input::Instance().GetGamePad();
	return gamePad.GetButton() & GamePad::BTN_A;
}
bool Player1P::InputJumpButtonUp()
{
	GamePad& gamePad = Input::Instance().GetGamePad();
	return gamePad.GetButtonUp() & GamePad::BTN_A;
}

// ハンマー攻撃ボタンBTN_Bが押されたか
bool Player1P::InputHammerButton()
{
	GamePad& gamePad = Input::Instance().GetGamePad();
	return gamePad.GetButtonDown() & GamePad::BTN_B;
}
// ソード攻撃ボタンBTN_Xが押されたか
bool Player1P::InputSwordButton()
{
	GamePad& gamePad = Input::Instance().GetGamePad();
	return gamePad.GetButtonDown() & GamePad::BTN_X;
}
// ソード攻撃ボタンBTN_Yが押されたか
bool Player1P::InputSpearButton()
{
	GamePad& gamePad = Input::Instance().GetGamePad();
	return gamePad.GetButtonDown() & GamePad::BTN_Y;
}

//スティック入力値から移動ベクトルを取得
XMFLOAT3 Player1P::GetMoveVec() const
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
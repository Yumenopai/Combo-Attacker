#include <map>

#include "Player.h"
#include "Graphics/Graphics.h"
#include "Graphics/PrimitiveRenderer.h"
#include "Input/Input.h"
#include "SceneManager.h"
#include "SceneLoading.h"
#include "SceneGame.h"
#include "SceneClear.h"
#include "PlayerManager.h"
#include "State/Player/PlayerEachState.h"
#include "EnemyManager.h"

#include "Stage.h"
#include "imgui.h"

//コンストラクタ
Player::Player()
{
}

Player::~Player()
{
}

// 初期化
void Player::Init()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	//プレイヤーモデル読み込み
	model = std::make_unique<Model>(device, "Data/Model/SD-UnityChan/UnityChan.fbx", 0.02f);

	//ヒットエフェクト読み込み
	hitEffect = std::make_unique<Effect>("Data/Effect/Hit.efk");

	//初期化
	enemySearch.clear();
	enemyDist.clear();
	EnemyManager& enemyManager = EnemyManager::Instance();
	int enemyCount = enemyManager.GetEnemyCount();//全ての敵と総当たりで衝突処理
	for (int i = 0; i < enemyCount; i++)
	{
		enemySearch[enemyManager.GetEnemy(i)] = EnemySearch::None;
		enemyDist[enemyManager.GetEnemy(i)] = FLT_MAX;
	}

	health = 100;
	maxHealth = 100;

	// StateMachineを生成 
	stateMachine = new PlayerStateMachine();
	// ステートマシンにステート登録 
	stateMachine->RegisterState(new StateIdle(this));
	stateMachine->RegisterState(new StateIdleToRun(this));
	stateMachine->RegisterState(new StateRun(this));
	stateMachine->RegisterState(new StateRunToIdle(this));
	stateMachine->RegisterState(new StateJumpStart(this));
	stateMachine->RegisterState(new StateJumpLoop(this));
	stateMachine->RegisterState(new StateJumpAir(this));
	stateMachine->RegisterState(new StateJumpEnd(this));
	stateMachine->RegisterState(new StateDamage(this));
	stateMachine->RegisterState(new StateDead(this));
	stateMachine->RegisterState(new StateRecover(this));
	stateMachine->RegisterState(new StateAttackHammer1(this));
	stateMachine->RegisterState(new StateAttackHammer2(this));
	stateMachine->RegisterState(new StateAttackHammerJump(this));
	stateMachine->RegisterState(new StateAttackSpear1(this));
	stateMachine->RegisterState(new StateAttackSpear2(this));
	stateMachine->RegisterState(new StateAttackSpear3(this));
	stateMachine->RegisterState(new StateAttackSpearJump(this));
	stateMachine->RegisterState(new StateAttackSword1(this));
	stateMachine->RegisterState(new StateAttackSword2(this));
	stateMachine->RegisterState(new StateAttackSword3(this));
	stateMachine->RegisterState(new StateAttackSwordJump(this));

	// ステートをセット 
	stateMachine->SetState(static_cast<int>(State::Idle));
}

//更新
void Player::Update(float elapsedTime)
{
}

void Player::UpdateUtils(float elapsedTime)
{
	// 配列ズラし
	//ShiftTrailPositions();

	// ステート毎に中で処理分け
	stateMachine->Update(elapsedTime);

	// 剣の軌跡描画更新処理
	//RenderTrail();

	// 攻撃中じゃなければジャンプ処理
	if (currentAttackType == AttackType::None) UpdateJumpState(elapsedTime);

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

void Player::UpdateEnemyDistance(float elapsedTime)
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

		enemyDist[enemy] = dist; //各エネミーとの距離等を更新毎に記録する
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
			if (i == (enemyCount - 1) && enemyCount == noneEnemy) currentEnemySearch = EnemySearch::None;
			continue;
		}

		/***********************/

		if (dist < nearestDist) //最近エネミーの登録
		{
			nearestEnemy = enemy;
			nearestDist = dist;
			currentEnemySearch = enemySearch[enemy];
			XMStoreFloat3(&nearestVec, DistVec);
		}
	}
}

//ジャンプ処理
void Player::UpdateJumpState(float elapsedTime)
{
	switch (jumpTrg)
	{
	case JumpState::CanJump:
		// 押している間の処理
		if (InputButton(Player::InputState::Jump))
		{
			velocity.y += 150 * elapsedTime;
			// 指定加速度まであがったら
			if (velocity.y > jumpSpeed)
			{
				jumpTrg = JumpState::CanDoubleJump;
			}
		}
		// 一回離した時
		else if (InputButtonUp(Player::InputState::Jump))
		{
			jumpTrg = JumpState::CanDoubleJump;
		}
		break;

	case JumpState::CanDoubleJump:
		// 2段目ジャンプは高さ調節不可
		if (InputButtonDown(Player::InputState::Jump))
		{
			if (velocity.y > 0) velocity.y += 15.0f;
			else				velocity.y = 15.0f;

			jumpTrg = JumpState::CannotJump;
		}
		// 一段目ジャンプ中の攻撃ボタン
		else if (InputAttackFromJump(elapsedTime))
		{
			jumpTrg = JumpState::CannotJump;
		}

		//break;
		[[fallthrough]];
	case JumpState::CannotJump:

		// ジャンプ可能状態の時のみ通らない
		// 着地時(地面に立っている時は常時処理)
		if (isGround)
		{
			// 着地時に押しっぱの場合は処理されないようにする
			if (InputButton(Player::InputState::Jump))
			{
				jumpTrg = JumpState::CannotJump;
			}
			// 押されていない時は地面にいるのでジャンプ可能状態にする
			else
			{
				jumpTrg = JumpState::CanJump;
			}
		}
		break;
	}
}

//描画
void Player::ShadowRender(const RenderContext& rc, ShadowMap* shadowMap)
{
	shadowMap->Draw(rc, model.get());
}
//描画
void Player::Render(const RenderContext& rc, ModelShader* shader)
{
	shader->Draw(rc, model.get());
}

// 攻撃の軌跡描画
void Player::PrimitiveRender(const RenderContext& rc)
{
	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();

	// ポリゴン描画
	primitiveRenderer->Render(rc.deviceContext, rc.camera->GetView(), rc.camera->GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}

void Player::HPBarRender(const RenderContext& rc, Sprite* gauge, bool is1P)
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	//HPゲージの長さ
	const float guageWidth = 700.0f;
	const float guageHeight = 15.0f;

	float healthRate = GetHealth() / static_cast<float>(GetMaxHealth());
	bool hpWorning = healthRate < 0.2f;
	const int frameExpansion = 6;
	Graphics& graphics = Graphics::Instance();
	const float screenWidth = static_cast<float>(graphics.GetScreenWidth());

	//ゲージ描画(下地)
	gauge->Render(dc,
		(screenWidth / 2) - (guageWidth / 2),
		is1P ? 555.0f : 580.0f,
		0,
		guageWidth + frameExpansion,
		guageHeight + frameExpansion,
		0, 0,
		static_cast<float>(gauge->GetTextureWidth()),
		static_cast<float>(gauge->GetTextureHeight()),
		0.0f,
		{ 0.3f, 0.3f, 0.3f, 0.8f }
	);
	//ゲージ描画
	gauge->Render(dc,
		(screenWidth / 2) - (guageWidth / 2) + frameExpansion / 2,
		(is1P ? 555.0f : 580.0f) + frameExpansion / 2,
		0,
		guageWidth * healthRate,
		guageHeight,
		0, 0,
		static_cast<float>(gauge->GetTextureWidth()),
		static_cast<float>(gauge->GetTextureHeight()),
		0.0f,
		{ hpWorning ? 0.8f : 0.2f, hpWorning ? 0.2f : (is1P ? 0.8f : 0.6f), 0.2f, 1.0f }
	);
}

void Player::DebugMenu()
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
			
		ImGui::Checkbox("attacking", &isAttackJudge);

		ImGui::End();
	}
}

//着地した時に呼ばれる
void Player::OnLanding(float elapsedTime)
{
	if (currentAttackType != AttackType::None) //攻撃中(主にジャンプ攻撃後)
	{
		// 着地してすぐは何もさせないためここで処理を書かない
		// 各StateUpdateにてアニメーション終了後にIdleStateへ遷移する
	}
	else if (InputMove(elapsedTime))
	{
		stateMachine->ChangeState(static_cast<int>(State::Run));
	}
	else {
		stateMachine->ChangeState(static_cast<int>(State::JumpEnd));
	}
}

void Player::OnDamaged()
{
	stateMachine->ChangeState(static_cast<int>(State::Damage));
}

void Player::OnDead()
{
	stateMachine->ChangeState(static_cast<int>(State::Dead));
}

void Player::UpdateArmPositions(Model* model, Arms& arm)
{
	Model::Node* armBone = model->FindNode(arm.nodeName);
	XMMATRIX W = XMLoadFloat4x4(&armBone->worldTransform);
	XMVECTOR V = arm.tipOffset;
	XMVECTOR P = XMVector3Transform(V, W);
	XMStoreFloat3(&arm.position, P);

	// 剣の根本と先端の座標を取得し、頂点バッファに保存
	// 剣の原点から根本と先端までのオフセット値
	DirectX::XMVECTOR RootOffset = arm.rootOffset;
	DirectX::XMVECTOR TipOffset = arm.tipOffset;

	XMVECTOR RootP = XMVector3Transform(RootOffset, W);
	XMVECTOR TipP = XMVector3Transform(TipOffset, W);
	DirectX::XMStoreFloat3(&trailPositions[0][0], RootP);
	DirectX::XMStoreFloat3(&trailPositions[1][0], TipP);
}

void Player::HorizontalVelocityByAttack(bool plus, int velo, float elapsedTime)
{
	if (enemySearch[nearestEnemy] == EnemySearch::Attack) return;

	if (plus)
	{
		velocity.x += sinf(angle.y) * velo * elapsedTime;
		velocity.z += cosf(angle.y) * velo * elapsedTime;
	}
	else
	{
		velocity.x = sinf(angle.y) * velo * elapsedTime;
		velocity.z = cosf(angle.y) * velo * elapsedTime;
	}
}

void Player::ShiftTrailPositions()
{
	for (int i = MAX_POLYGON - 1; i > 0; i--)
	{
		// 後ろへずらしていく
		trailPositions[0][i] = trailPositions[0][i - 1];
		trailPositions[1][i] = trailPositions[1][i - 1];
	}
}
void Player::RenderTrail()
{
	// ポリゴン作成
	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();
	for (int i = 0; i < MAX_POLYGON - 3; ++i)
	{
		const int division = 10;

		XMVECTOR RPos1 = XMLoadFloat3(&trailPositions[0][i + 0]);
		XMVECTOR RPos2 = XMLoadFloat3(&trailPositions[0][i + 1]);
		XMVECTOR RPos3 = XMLoadFloat3(&trailPositions[0][i + 2]);
		XMVECTOR RPos4 = XMLoadFloat3(&trailPositions[0][i + 3]);
		XMVECTOR TPos1 = XMLoadFloat3(&trailPositions[1][i + 0]);
		XMVECTOR TPos2 = XMLoadFloat3(&trailPositions[1][i + 1]);
		XMVECTOR TPos3 = XMLoadFloat3(&trailPositions[1][i + 2]);
		XMVECTOR TPos4 = XMLoadFloat3(&trailPositions[1][i + 3]);
		for (int j = 1; j < division; ++j)
		{
			float t = j / static_cast<float>(division);

			XMFLOAT3 Position[2];
			DirectX::XMStoreFloat3(&Position[0], XMVectorCatmullRom(RPos1, RPos2, RPos3, RPos4, t));
			DirectX::XMStoreFloat3(&Position[1], XMVectorCatmullRom(TPos1, TPos2, TPos3, TPos4, t));

			if (isAddVertex)
			{
				primitiveRenderer->AddVertex(Position[0], color);
				primitiveRenderer->AddVertex(Position[1], color);
			}
		}
	}
}

//垂直速力更新
void Player::UpdateVerticalVelocity(float elapsedFrame)
{
	auto state = static_cast<Player::State>(stateMachine->GetStateNumber());

	// ジャンプスピアー攻撃のみ別の下向き処理を使用する
	if (state == State::AttackSpearJump)
	{
		if (velocity.y > 0)	velocity.y = 0;
		velocity.y += gravity * 0.25f * elapsedFrame;
	}
	else
	{
		velocity.y += gravity * elapsedFrame;
	}
}

// ===========入力処理===========
//移動入力処理
bool Player::InputMove(float elapsedTime)
{
	XMFLOAT3 moveVec = GetMoveVec();

	//移動処理
	Move(moveVec.x, moveVec.z, moveSpeed);
	//旋回処理
	Turn(elapsedTime, moveVec.x, moveVec.z, turnSpeed);

	//進行ベクトルがゼロベクトルでない場合は入力された
	return moveVec.x != 0 || moveVec.y != 0 || moveVec.z != 0;
}

// 攻撃入力処理
bool Player::InputAttackFromNoneAttack(float elapsedTime)
{
	if (InputButtonDown(Player::InputState::Hammer)) {
		stateMachine->ChangeState(static_cast<int>(State::AttackHammer1));
	}
	else if (InputButtonDown(Player::InputState::Sword)) {
		stateMachine->ChangeState(static_cast<int>(State::AttackSword1));
	}
	else if (InputButtonDown(Player::InputState::Spear)) {
		stateMachine->ChangeState(static_cast<int>(State::AttackSpear1));
	}
	else {
		return false;
	}

	if (enemySearch[nearestEnemy] >= EnemySearch::Find)
	{
		//旋回処理
		Turn(elapsedTime, nearestVec.x, nearestVec.z, 1000);
	}

	return true;
}
bool Player::InputAttackFromJump(float elapsedTime)
{
	if (InputButtonDown(Player::InputState::Hammer)) {
		stateMachine->ChangeState(static_cast<int>(State::AttackHammerJump));
	}
	else if (InputButtonDown(Player::InputState::Sword)) {
		stateMachine->ChangeState(static_cast<int>(State::AttackSwordJump));
	}
	else if (InputButtonDown(Player::InputState::Spear)) {
		stateMachine->ChangeState(static_cast<int>(State::AttackSpearJump));
	}
	else {
		return false;
	}

	return true;
}

//回復遷移確認処理
bool Player::IsRecoverTransition()
{
	Player* targetplayer = this;
	PlayerManager& playerManager = PlayerManager::Instance();
	int playerCount = playerManager.GetPlayerCount();
	for (int i = 0; i < playerCount; i++)
	{
		if (playerManager.GetPlayer(i) == this) continue;
		targetplayer = playerManager.GetPlayer(i);
	}
	// 20%以上はfalse
	if (targetplayer->GetHealthRate() > 20) return false;

	XMVECTOR posPlayerthis = XMLoadFloat3(&GetPosition());
	XMVECTOR posPlayertarget = XMLoadFloat3(&targetplayer->GetPosition());
	float distSq = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(posPlayerthis, posPlayertarget)));
	// 近く無ければfalse
	if (distSq > 2.0f * 2.0f) return false;

	// trueで回復遷移
	return true;
}

//プレイヤーとエネミーとの衝突処理
void Player::CollisionPlayerVsEnemies()
{
	EnemyManager& enemyManager = EnemyManager::Instance();

	//全ての敵と総当たりで衝突処理
	int enemyCount = enemyManager.GetEnemyCount();
	for (int i = 0; i < enemyCount; i++)
	{
		Enemy* enemy = enemyManager.GetEnemy(i);

		//衝突処理
		DirectX::XMFLOAT3 outPosition;
		if (Collision::IntersectCylinderVsCylinder(
			enemy->GetPosition(), enemy->GetRadius(), enemy->GetHeight(),
			position, radius, height,
			XMFLOAT3{}, outPosition
		))
		{
			SetPosition(outPosition);
		}
	}
}

// 武器とエネミーの衝突処理
void Player::CollisionArmsVsEnemies(Arms arm)
{
	//指定のノードと全ての敵を総当たりで衝突処理
	EnemyManager& enemyManager = EnemyManager::Instance();
	int enemyCount = enemyManager.GetEnemyCount();

	for (int i = 0; i < enemyCount; ++i)
	{
		Enemy* enemy = enemyManager.GetEnemy(i);

		//衝突処理
		XMFLOAT3 outPosition;
		if (Collision::IntersectSphereVsCylinder(
			arm.position, arm.radius,
			enemy->GetPosition(), enemy->GetRadius(), enemy->GetHeight(),
			outPosition
		))
		{
			if (attackingEnemyNumber == i && !isAttackJudge) return; //攻撃判定しない場合は処理しない

			//ダメージを与える
			if (enemy->ApplyDamage(1, 0))
			{
				//吹き飛ばす
				if (attackCount >= 4)
				{
					const float power = 13.0f; //仮の水平の力
					const XMFLOAT3& ep = enemy->GetPosition();

					//ノックバック方向の算出
					float vx = ep.x - Player1P::Instance().position.x;
					float vz = ep.z - Player1P::Instance().position.z;
					float lengthXZ = sqrtf(vx * vx + vz * vz);
					vx /= lengthXZ;
					vz /= lengthXZ;

					//ノックバック力の定義
					XMFLOAT3 impluse;
					impluse.x = vx * power;
					impluse.z = vz * power;

					impluse.y = power * 0.8f;	//上方向にも打ち上げる

					enemy->AddImpulse(impluse);
				}
				//ヒットエフェクト再生
				{
					DirectX::XMFLOAT3 e = enemy->GetPosition();
					e.y += enemy->GetHeight() * 0.5f;
					hitEffect->Play(e);
				}
				attackingEnemyNumber = i;
				isAttackJudge = false;
			}
		}
		else if(attackingEnemyNumber == i)//攻撃中のエネミーと一旦攻撃が外れた時、次回当たった時に判定を行う
		{
			isAttackJudge = true;
		}
	}
}

//デバッグプリミティブ描画
void Player::DrawDebugPrimitive()
{
	Gizmos* gizmos = Graphics::Instance().GetGizmos();
	//衝突判定用のデバッグ円柱を描画
	gizmos->DrawCylinder(
		position,		//位置
		radius,			//半径
		height,			//高さ
		{ 0,0,0 },		//角度
		{ 1,0,0,1 });	//色

	if (Hammer.flag1 || Hammer.flag2 || Hammer.flagJump)
	{
		gizmos->DrawSphere(
			Hammer.position,
			Hammer.radius,
			XMFLOAT4(0, 0, 1, 1)
		);
	}
	if (Spear.flag1 || Spear.flag2 || Spear.flag3 || Spear.flagJump)
	{
		gizmos->DrawSphere(
			Spear.position,
			Spear.radius,
			XMFLOAT4(0, 0, 1, 1)
		);
	}
	if (Sword.flag1 || Sword.flag2 || Sword.flag3 || Sword.flagJump)
	{
		gizmos->DrawSphere(
			Sword.position,
			Sword.radius,
			XMFLOAT4(0, 0, 1, 1)
		);
	}

	Camera& camera = Camera::Instance();
	//描画コンテキスト設定
	RenderContext rc;
	rc.camera = &camera;
	rc.deviceContext = Graphics::Instance().GetDeviceContext();
	rc.renderState = Graphics::Instance().GetRenderState();

	//描画実行
	gizmos->Render(rc);
}
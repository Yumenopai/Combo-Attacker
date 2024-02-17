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

// 攻撃の軌跡描画
void Player::PrimitiveRender(const RenderContext& rc)
{
	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();

	// ポリゴン描画
	primitiveRenderer->Render(rc.deviceContext, rc.camera->GetView(), rc.camera->GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
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
			
		ImGui::Checkbox("attacking", &isAttackjudge);

		ImGui::End();
	}
}

//着地した時に呼ばれる
void Player::OnLanding(float elapsedTime)
{
	if (Atype != AttackType::None) //攻撃中(主にジャンプ攻撃後)
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
	isDamaged = true;
	stateMachine->ChangeState(static_cast<int>(State::Damage));
}

void Player::OnDead()
{
	if(!isDead)
	{
		stateMachine->ChangeState(static_cast<int>(State::Dead));
		isDead = true;
	}
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

//垂直速力更新オーバーライド
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
	XMFLOAT3 moveVec = {}; //GetMoveVec();
	XMVECTOR MoveVec = XMLoadFloat3(&moveVec); //進行ベクトルを取得

	// 動いていて且つ最近距離が登録されている
	if (XMVectorGetX(XMVector3LengthSq(MoveVec)) != 0 && nearestDist < FLT_MAX)
	{
		// 向かっているのがエネミーベクトルと鋭角関係なら
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

// 攻撃入力処理
bool Player::InputAttackFromNoneAttack(float elapsedTime)
{
	if (InputHammerButton()) {
		stateMachine->ChangeState(static_cast<int>(State::AttackHammer1));
	}
	else if (InputSwordButton()) {
		stateMachine->ChangeState(static_cast<int>(State::AttackSword1));
	}
	else if (InputSpearButton()) {
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
	if (InputHammerButton()) {
		stateMachine->ChangeState(static_cast<int>(State::AttackHammerJump));
	}
	else if (InputSwordButton()) {
		stateMachine->ChangeState(static_cast<int>(State::AttackSwordJump));
	}
	else if (InputSpearButton()) {
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
			if (attackingEnemyNumber == i && !isAttackjudge) return; //攻撃判定しない場合は処理しない

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
				isAttackjudge = false;
			}
		}
		else if(attackingEnemyNumber == i)//攻撃中のエネミーと一旦攻撃が外れた時、次回当たった時に判定を行う
		{
			isAttackjudge = true;
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
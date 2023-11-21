#include <map>

#include "Player.h"
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

//コンストラクタ
Player::Player()
{
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

	position = { -7,5,-66 };
	health = 100;
	maxHealth = 100;

	//待機ステートへ遷移
	TransitionIdleState();

	//ヒットエフェクト読み込み
	hitEffect = std::make_unique<Effect>("Data/Effect/Hit.efk");
}

Player::~Player()
{
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

#if 0
	//デバッグメニュー描画
	DebugMenu();
#endif
}

// 攻撃の軌跡描画
void Player::PrimitiveRender(const RenderContext& rc)
{
	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();

	// ポリゴン描画
	primitiveRenderer->Render(rc.deviceContext, rc.camera->GetView(), rc.camera->GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
}
// 攻撃の軌跡描画
void Player::HPBarRender(const RenderContext& rc, Sprite* gauge)
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
	else if (InputMove(elapsedTime)) TransitionRunState();
	else TransitionJumpEndState();
}

void Player::OnDamaged()
{
	isDamaged = true;
}

void Player::OnDead()
{
	//if(Player::GetState() != Player::State::Dead)
	//	TransitionDamageState();
}

// 各ステージごとの更新処理
void Player::UpdateEachState(float elapsedTime)
{
	switch (state)
	{
	// 待機ステート
	case State::Idle:
		// 移動入力処理
		if (InputMove(elapsedTime)) TransitionIdleToRunState();
		// ジャンプ入力処理
		if (InputJumpButtonDown()) TransitionJumpStartState();
		// 攻撃処理
		InputAttackFromNoneAttack(elapsedTime);
		break;

	case State::IdleToRun:
		InputMove(elapsedTime);
		// アニメーション終了後
		if (!model->IsPlayAnimation()) TransitionRunState();
		break;

	case State::Run:
		// 移動入力処理
		if (!InputMove(elapsedTime)) TransitionIdleState();
		// ジャンプ入力処理
		if (InputJumpButtonDown()) TransitionJumpStartState();
		// 攻撃処理
		InputAttackFromNoneAttack(elapsedTime);
		break;

	// 現在未使用
	case State::RunToIdle:
		// アニメーション終了後
		if (!model->IsPlayAnimation()) TransitionIdleState();
		break;

	case State::JumpStart:
		InputMove(elapsedTime);
		// さらにジャンプ入力時の処理
		if (InputJumpButtonDown()) TransitionJumpAirState();
		// アニメーション終了後
		if (!model->IsPlayAnimation()) TransitionJumpLoopState();
		break;

	case State::JumpLoop:
		InputMove(elapsedTime);
		// さらにジャンプ入力時の処理
		if (InputJumpButtonDown()) TransitionJumpAirState();
		// ジャンプ中の攻撃処理はUpdateJumpにて行う
		
		// 着地(JumpEnd)ステートへの遷移は着地時にしか行わない
		// 保険で既に着地している時用に呼び出し
		if (isGround) OnLanding(elapsedTime);
		break;

	case State::JumpAir:
		InputMove(elapsedTime);
		// 保険で既に着地している時用に呼び出し
		if (isGround) OnLanding(elapsedTime);
		break;

	case State::JumpEnd:
		// InputMove時などOnLandingでここを通らない可能性あり
		// アニメーション終了後
		if (InputJumpButtonDown()) TransitionJumpStartState();
		if (!model->IsPlayAnimation()) TransitionIdleState();
		break;

	case State::Damage:
		// ダメージ受けたら止まる?
		// アニメーション終了後
		if (!model->IsPlayAnimation()) TransitionIdleState();
		break;

	case State::Dead:
		// Deathアニメーションは起き上がりまでなので途中で止めるのが良い?
		// アニメーション終了後Idleにとりあえず移行
		if (!model->IsPlayAnimation()) TransitionIdleState();
		break;
	case State::AttackHammer1:
		Hammer.flag1 = (model->IsPlayAnimation());
		if (Hammer.flag1)
		{
			UpdateArmPositions(model.get(), Hammer);
			CollisionArmsVsEnemies(Hammer);

			//任意のアニメーション再生区間でのみ次の攻撃技を出すようにする
			float animationTime = model->GetCurrentAnimationSeconds();
			if (InputHammerButton() && animationTime >= 0.5f && animationTime <= 0.8f)
			{
				TransitionAttackHummer2State();
			}
		}
		else TransitionIdleState();
		break;
	case State::AttackHammer2:
		Hammer.flag1 = false;
		Hammer.flag2 = (model->IsPlayAnimation());
		if (Hammer.flag2)
		{
			UpdateArmPositions(model.get(), Hammer);
			CollisionArmsVsEnemies(Hammer);
		}
		else TransitionIdleState();
		break;
	case State::AttackHammerJump:
		Hammer.flagJump = model->IsPlayAnimation();
		if (Hammer.flagJump)
		{
			UpdateArmPositions(model.get(), Hammer);
			CollisionArmsVsEnemies(Hammer);
		}

		if (isMoveAttack)
		{
			HorizontalVelocityByAttack(false, 800, elapsedTime);
		}
		if (!model->IsPlayAnimation())
		{
			isMoveAttack = false;
			TransitionIdleState();
		}
		break;
	case State::AttackSpear1:
		if (model->IsPlayAnimation())
		{
			UpdateArmPositions(model.get(), Spear);
			CollisionArmsVsEnemies(Spear);

			float animationTime = model->GetCurrentAnimationSeconds();
			// 武器出現アニメーション再生区間
			Spear.flag1 = (animationTime >= 0.20f && animationTime <= 0.7f);
			// 足を踏ん張る際の前進をここで行う 既に何か進んでいる時はこの処理をしない
			if (animationTime < 0.25f && !InputMove(elapsedTime))
			{	
				HorizontalVelocityByAttack(true, 40, elapsedTime);
			}
			else if (Spear.flag1 && InputSpearButton())
			{
				attackCount++;
				TransitionAttackSpear2State();
			}
		}
		else TransitionIdleState();
		break;
	case State::AttackSpear2:
		Spear.flag1 = false;
		Spear.flag2 = model->IsPlayAnimation();
		if (Spear.flag2)
		{
			UpdateArmPositions(model.get(), Spear);
			CollisionArmsVsEnemies(Spear);

			float animationTime = model->GetCurrentAnimationSeconds();
			if (animationTime >= 0.30f && animationTime <= 0.45f)
			{
				// 足を踏ん張る際の前進
				HorizontalVelocityByAttack(true, 60, elapsedTime);
			}
			// 武器出現アニメーション再生区間
			if (animationTime >= 0.37f && animationTime <= 0.6f && InputSpearButton())
			{
				attackCount++;
				if (attackCount >= 4) TransitionAttackSpear3State();
				else TransitionAttackSpear1State();
			}
		}
		else TransitionIdleState();
		break;
	case State::AttackSpear3:
		Spear.flag2 = false;
		Spear.flag3 = model->IsPlayAnimation();
		if (Spear.flag3)
		{
			UpdateArmPositions(model.get(), Spear);
			CollisionArmsVsEnemies(Spear);

			float animationTime = model->GetCurrentAnimationSeconds();
			// 足を踏ん張る際の前進をここで行う
			if (animationTime < 0.43f)
			{
				HorizontalVelocityByAttack(true, 39, elapsedTime);
			}
		}
		else TransitionIdleState();
		break;
	case State::AttackSpearJump:
		Spear.flagJump = model->IsPlayAnimation();
		if (Spear.flagJump)
		{
			UpdateArmPositions(model.get(), Spear);
			CollisionArmsVsEnemies(Spear);

			float animationTime = model->GetCurrentAnimationSeconds();
			// 回転しながら前下方向に突き刺していくアニメーション
			if (animationTime >= 0.15f && animationTime <= 0.5f)
			{
				HorizontalVelocityByAttack(false, 800, elapsedTime);
			}
		}
		else TransitionIdleState();
		break;
	case State::AttackSword1:
		Sword.flag1 = model->IsPlayAnimation();
		if (Sword.flag1)
		{
			UpdateArmPositions(model.get(), Sword);
			CollisionArmsVsEnemies(Sword);

			float animationTime = model->GetCurrentAnimationSeconds();
			// 足を踏ん張る際の前進をここで行う 既に何か進んでいる時はこの処理をしない
			if (animationTime < 0.2f && !InputMove(elapsedTime))
			{
				HorizontalVelocityByAttack(true, 43, elapsedTime);
			}
			// 任意のアニメーション再生区間でのみ次の攻撃技を出すようにする
			else if (InputSwordButton() && animationTime >= 0.3f && animationTime <= 0.7f)
			{
				attackCount++;
				TransitionAttackSword2State();
			}
			// 足元の動きに合わせた前進 違和感あるので一旦コメント
			//else if (animationTime >= 0.50f)
			//{
			//	velocity.x += sinf(angle.y) * 38 * elapsedTime;
			//	velocity.z += cosf(angle.y) * 38 * elapsedTime;
			//}
		}
		else TransitionIdleState();
		break;
	case State::AttackSword2:
		Sword.flag1 = false;
		Sword.flag2 = model->IsPlayAnimation();
		if (Sword.flag2)
		{
			UpdateArmPositions(model.get(), Sword);
			CollisionArmsVsEnemies(Sword);

			float animationTime = model->GetCurrentAnimationSeconds();
			// 足を踏ん張る際の前進をここで行う
			if (animationTime < 0.25f)
			{
				HorizontalVelocityByAttack(true, 45, elapsedTime);
			}
			// 任意のアニメーション再生区間でのみ次の攻撃技を出すようにする
			else if (InputSwordButton() && animationTime >= 0.35f && animationTime <= 0.6f)
			{
				attackCount++;
				if (attackCount >= 4) TransitionAttackSword3State();
				else TransitionAttackSword1State();
			}
		}
		else TransitionIdleState();
		break;
	case State::AttackSword3:
		Sword.flag2 = false;
		Sword.flag3 = model->IsPlayAnimation();
		if (Sword.flag3)
		{
			UpdateArmPositions(model.get(), Sword);
			float animationTime = model->GetCurrentAnimationSeconds();
			if (animationTime <= 0.6f)
				CollisionArmsVsEnemies(Sword);

			// 足を踏ん張る際の前進をここで行う
			if (animationTime >= 0.25f && animationTime <= 0.5f)
			{
				HorizontalVelocityByAttack(true, 48, elapsedTime);
				if (animationTime <= 0.4f)
					velocity.y += 150 * elapsedTime;
			}
		}
		else TransitionIdleState();
		break;
	case State::AttackSwordJump:
		Sword.flagJump = model->IsPlayAnimation();
		if (Sword.flagJump)
		{
			UpdateArmPositions(model.get(), Sword);
			CollisionArmsVsEnemies(Sword);
		}

		if (isMoveAttack)
		{
			HorizontalVelocityByAttack(false, 800, elapsedTime);
		}
		if (!model->IsPlayAnimation())
		{
			isMoveAttack = false;
			TransitionIdleState();
		}
		break;
	case State::CliffGrab:
		// アニメーション終了後Idleにとりあえず移行
		if (!model->IsPlayAnimation()) TransitionIdleState();
		break;
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
	//ジャンプ攻撃中は重力を無視する
	if (state == State::AttackHammerJump || state == State::AttackSwordJump)
	{
		velocity.y = 0;
	}
	
	if (state == State::AttackSpearJump)
	{
		if (velocity.y > 0)	velocity.y = 0;
		velocity.y += gravity * 0.25f * elapsedFrame;
	}
	// スピアー攻撃のみ別の下向き処理を使用する
	else velocity.y += gravity * elapsedFrame;
}

// ===========入力処理===========
//移動入力処理
bool Player::InputMove(float elapsedTime)
{
	XMFLOAT3 moveVec = {}; //GetMoveVec();
	XMVECTOR MoveVec = XMLoadFloat3(&moveVec); //進行ベクトルを取得

	// 動いていて且つ最近距離が登録されている
	if (XMVectorGetX(XMVector3Length(MoveVec)) != 0 && nearestDist < FLT_MAX)
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
	if (InputHammerButton())	 TransitionAttackHummer1State();
	else if (InputSwordButton()) TransitionAttackSword1State();
	else if (InputSpearButton()) TransitionAttackSpear1State();
	else return false;

	if (enemySearch[nearestEnemy] > EnemySearch::None)
	{
		//旋回処理
		Turn(elapsedTime, nearestVec.x, nearestVec.z, 1000);
	}

	return true;
}
bool Player::InputAttackFromJump(float elapsedTime)
{
	if (InputHammerButton())	 TransitionAttackHummerJumpState(elapsedTime);
	else if (InputSwordButton()) TransitionAttackSwordJumpState(elapsedTime);
	else if (InputSpearButton()) TransitionAttackSpearJumpState();
	else return false;

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
			//ダメージを与える
			if (enemy->ApplyDamage(1, 0.5f))
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
			}
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


// ========遷移========
// 待機ステート
void Player::TransitionIdleState()
{
	state = State::Idle;
	attackCount = 0;
	// 攻撃後は必ず待機に戻る為ここで攻撃タイプ リセットを行う
	Atype = AttackType::None;
	model->PlayAnimation(Anim_Idle, true);
}
// 移動ステート
void Player::TransitionRunState()
{
	state = State::Run;
	model->PlayAnimation(Anim_Running, true);
}
// 待機->移動切替ステート
void Player::TransitionIdleToRunState()
{
	state = State::IdleToRun;
	model->PlayAnimation(Anim_IdleToRun, false);
}
// 移動->待機切替ステート
void Player::TransitionRunToIdleState()	// 未使用
{
	state = State::RunToIdle;
	model->PlayAnimation(Anim_RunToIdle, false);
}

// ジャンプステートへ遷移
void Player::TransitionJumpStartState()
{
	state = State::JumpStart;
	model->PlayAnimation(Anim_JumpStart, false);
}
void Player::TransitionJumpLoopState()
{
	state = State::JumpLoop;
	model->PlayAnimation(Anim_JumpLoop, false);
}
void Player::TransitionJumpAirState()
{
	state = State::JumpAir;
	model->PlayAnimation(Anim_JumpAir, false);
}
void Player::TransitionJumpEndState()
{
	state = State::JumpEnd;
	model->PlayAnimation(Anim_JumpEnd, false);
}
// ダメージステート
void Player::TransitionDamageState()
{
	state = State::Damage;
	model->PlayAnimation(Anim_Damage, false);
}
// 死亡ステート
void Player::TransitionDeadState()
{
	state = State::Dead;
	model->PlayAnimation(Anim_Death, false);
}
// 攻撃ステート
void Player::TransitionAttackHummer1State()
{
	state = State::AttackHammer1;
	Atype = AttackType::Hammer;
	model->PlayAnimation(Anim_AttackHammer1, false);
}
void Player::TransitionAttackHummer2State()
{
	state = State::AttackHammer2;
	model->PlayAnimation(Anim_AttackHammer2, false);
}
void Player::TransitionAttackHummerJumpState(float elapsedTime)
{
	state = State::AttackHammerJump;
	Atype = AttackType::Hammer;
	if (InputMove(elapsedTime)) isMoveAttack = true;
	model->PlayAnimation(Anim_AttackHammerJump, false);
}
void Player::TransitionAttackSpear1State()
{
	state = State::AttackSpear1;
	Atype = AttackType::Spear;
	model->PlayAnimation(Anim_AttackSpear1, false);
}
void Player::TransitionAttackSpear2State()
{
	state = State::AttackSpear2;
	model->PlayAnimation(Anim_AttackSpear2, false);
}
void Player::TransitionAttackSpear3State()
{
	state = State::AttackSpear3;
	model->PlayAnimation(Anim_AttackSpear3, false);
}
void Player::TransitionAttackSpearJumpState()
{
	state = State::AttackSpearJump;
	Atype = AttackType::Spear;
	model->PlayAnimation(Anim_AttackSpearJump, false);
}
void Player::TransitionAttackSword1State()
{
	state = State::AttackSword1;
	Atype = AttackType::Sword;
	model->PlayAnimation(Anim_AttackSword1, false);
}
void Player::TransitionAttackSword2State()
{
	state = State::AttackSword2;
	model->PlayAnimation(Anim_AttackSword2, false);
}
void Player::TransitionAttackSword3State()
{
	state = State::AttackSword3;
	model->PlayAnimation(Anim_AttackSword3, false);
}
void Player::TransitionAttackSwordJumpState(float elapsedTime)
{
	state = State::AttackSwordJump;
	Atype = AttackType::Sword;
	if (InputMove(elapsedTime)) isMoveAttack = true;
	model->PlayAnimation(Anim_AttackSwordJump, false);
}
void Player::TransitionCliffGrabState()
{
	state = State::CliffGrab;
	model->PlayAnimation(Anim_CliffGrab, false);
}
#include <map>

#include "Player.h"
#include "PlayerManager.h"
#include "State/Player/PlayerEachState.h"
#include "Graphics/Graphics.h"
#include "Graphics/PrimitiveRenderer.h"
#include "Input/Input.h"
#include "EnemyManager.h"
#include "Enemy.h"
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
	model = std::make_unique<Model>(device, "Data/Model/SD-UnityChan/UnityChan.fbx", playerModelSize);

	// 武器登録
	HaveArms.clear();
	for (int i = 0; i < SC_INT(Player::AttackType::MaxCount); i++)
	{
		HaveArms.insert(std::pair<Player::AttackType, bool>(SC_AT(i), false));
	}
	// 初期武器
	CurrentUseArm = InitialArm;
	HaveArms[InitialArm] = true;

	enemySearch.clear();
	enemyDist.clear();
	EnemyManager& enemyManager = EnemyManager::Instance();
	int enemyCount = enemyManager.GetEnemyCount();//全ての敵と総当たりで衝突処理
	for (int i = 0; i < enemyCount; i++)
	{
		enemySearch[enemyManager.GetEnemy(i)] = EnemySearch::None;
		enemyDist[enemyManager.GetEnemy(i)] = FLT_MAX;
	}

	// HP設定
	health = maxHealth = playerMaxHealth;

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
	stateMachine->SetState(SC_INT(State::Idle));
}

void Player::UpdateUtils(float elapsedTime)
{
	// 配列ズラし
	//ShiftTrailPositions();

	// 回復遷移可能か
	enableRecoverTransition = EnableRecoverTransition();

	// ステート毎に中で処理分け
	stateMachine->Update(elapsedTime);

	// 剣の軌跡描画更新処理
	//RenderTrail();

	// ジャンプ処理
	UpdateJumpState(elapsedTime);

	// プレイヤーとエネミーとの衝突処理
	CollisionPlayerVsEnemies();

	// 速力処理更新
	UpdateVelocity(elapsedTime);

	// オブジェクト行列更新
	UpdateTransform();

	// モデルアニメーション更新処理
	model->UpdateAnimation(elapsedTime);

	// モデル行列更新
	model->UpdateTransform(transform);
}

// 敵との距離更新
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
			if (i == (enemyCount - 1) && enemyCount == noneEnemy)
			{
				currentEnemySearch = EnemySearch::None;
			}
			continue;
		}

		/***********************/
		if (dist < nearestDist) 
		{
			// secondEnemyの登録
			secondDist = nearestDist;
			secondDistEnemyVec = nearestVec;

			// 最近エネミーの登録
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
	// 攻撃中の場合はジャンプさせない
	if (isAttacking) return;

	const float firstJumpSpeed = 150.0f;
	const float secondJumpSpeed = 15.0f;
	const float MaxJumpSpeed = 17.5f;

	switch (jumpTrg)
	{
	case JumpState::CanJump:
		// 押している間の処理
		if (InputButton(Player::InputState::Jump))
		{
			velocity.y += firstJumpSpeed * elapsedTime;
			// 指定加速度まであがったら
			if (velocity.y > MaxJumpSpeed)
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
			if (velocity.y > 0) velocity.y += secondJumpSpeed;
			else				velocity.y = secondJumpSpeed;

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

// HP描画
void Player::RenderHPBar(ID3D11DeviceContext* dc, Sprite* gauge, FontSprite* font) const
{
	const float frameExpansion = 6;
	Graphics& graphics = Graphics::Instance();
	const float screenWidth = static_cast<float>(graphics.GetScreenWidth());

	// 名前表示
	font->Textout(dc, characterName, 200, hpGuage_Y - 7.0f, 1.0f, { -10, 10, 0 }, 12, 16, 32, 32, 16, 16, 0, nameColor);
	// Lv表示
	std::string levelStr = "Lv:" + std::to_string(currentLevel);
	font->Textout(dc, levelStr, 800, hpGuage_Y + 12.0f, 1.0f, { -10, 10, 0 }, 12, 16, 32, 32, 16, 16, 0, nameColor);
	// HP表示
	std::string hpStr = std::to_string(GetHealth()) + '/' + std::to_string(GetMaxHealth());
	font->Textout(dc, hpStr, 900, hpGuage_Y + 12.0f, 1.0f, { -10, 10, 0 }, 12, 16, 32, 32, 16, 16, 0, nameColor);

	//ゲージ描画(下地)
	gauge->Render(dc,
		(screenWidth / 2) - (hpGuageWidth / 2),
		hpGuage_Y,
		0,
		hpGuageWidth + frameExpansion,
		hpGuageHeight + frameExpansion,
		0, 0,
		static_cast<float>(gauge->GetTextureWidth()),
		static_cast<float>(gauge->GetTextureHeight()),
		0.0f,
		{ 0.3f, 0.3f, 0.3f, 0.8f }
	);
	//ゲージ描画
	gauge->Render(dc,
		(screenWidth / 2) - (hpGuageWidth / 2) + frameExpansion / 2,
		hpGuage_Y + frameExpansion / 2,
		0,
		hpGuageWidth * (GetHealthRate() / 100.0f),
		hpGuageHeight,
		0, 0,
		static_cast<float>(gauge->GetTextureWidth()),
		static_cast<float>(gauge->GetTextureHeight()),
		0.0f,
		{ GetHpWorning() ? 0.8f : 0.2f, GetHpWorning() ? 0.2f : 0.8f , 0.2f, 1.0f}
	);
}

// キャラクター名前描画
void Player::RenderCharacterOverHead(const RenderContext& rc, FontSprite* font, Sprite* message)
{
	const DirectX::XMFLOAT4X4& view = rc.view;
	const DirectX::XMFLOAT4X4& projection = rc.projection;

	//ビューポート
	D3D11_VIEWPORT viewport;
	UINT numViewports = 1;
	rc.deviceContext->RSGetViewports(&numViewports, &viewport);

	//変換行列
	XMMATRIX View = XMLoadFloat4x4(&view);
	XMMATRIX Projection = XMLoadFloat4x4(&projection);
	XMMATRIX World = XMMatrixIdentity();

	//Player頭上のワールド座標
	XMFLOAT3 worldPosition = GetPosition();
	worldPosition.y += GetHeight() + 0.4f;
	XMVECTOR WorldPosition = XMLoadFloat3(&worldPosition);

	//ワールドからスクリーンへの変換
	XMVECTOR ScreenPosition = XMVector3Project(
		WorldPosition,
		viewport.TopLeftX,
		viewport.TopLeftY,
		viewport.Width,
		viewport.Height,
		viewport.MinDepth,
		viewport.MaxDepth,
		Projection,
		View,
		World
	);

	XMFLOAT3 screenPosition;
	XMStoreFloat3(&screenPosition, ScreenPosition);
	//カメラの背後にいるか、明らかに離れているなら描画しない
	if (screenPosition.z > 0.0f && screenPosition.z < 1.0f)
	{
		// 名前
		font->Textout(rc.deviceContext, characterName,
			0,
			screenPosition.y,
			0,
			{ screenPosition.x - 12 * 5, 0, 0 },
			12, 16,
			32, 32, 16, 16, 0, nameColor);

		// メッセージ
		if (messageNumber == 0 || messageNumber == 1)
		{
			const DirectX::XMFLOAT2 spriteSize = { 600.0f,100.f };
			messageYTimer += 0.2f;
			message->Render(rc.deviceContext, { screenPosition.x - 60, screenPosition.y - 5.0f - messageYTimer, 0 }, { 180, 30 }, { spriteSize.x, spriteSize.y * messageNumber }, spriteSize, 0, { 1, 1, 1, 1 });
			
			if (messageYTimer > 10.0f)
			{
				enableShowMessage[messageNumber] = false;
				messageNumber = -1;
			}
		}
		else if (enableShowMessage[0])
		{
			messageNumber = 0;
			messageYTimer = 0.0;
		}
		else if (enableShowMessage[1])
		{
			messageNumber = 1;
			messageYTimer = 0.0;
		}
		else
		{
			for (int i = SC_INT(MessageNotification::Attack); i < SC_INT(MessageNotification::MaxCount); i++)
			{
				if (enableShowMessage[i])
				{
					const DirectX::XMFLOAT2 spriteSize = { 600.0f,100.f };
					message->Render(rc.deviceContext, { screenPosition.x - 60, screenPosition.y - 5.0f, 0 }, { 180, 30 }, { spriteSize.x, spriteSize.y * i }, spriteSize, 0, { 1, 1, 1, 1 });
					break;
				}
			}
		}
	}
}

void Player::RenderHaveArms(ID3D11DeviceContext* dc, Sprite* frame, Sprite* arm)
{
	const DirectX::XMFLOAT2 spriteSize = { 300.0f,300.f };

	//HaveArmFrame
	for (int i = 0; i < HaveArms.size(); i++)
	{
		float spriteOffset_x = 0;
		if (i == SC_INT(CurrentUseArm)) spriteOffset_x = spriteSize.x;
		else spriteOffset_x = HaveArms[SC_AT(i)] ? spriteSize.x * 2 : 0;

		frame->Render(dc, { 1000.0f + 65 * i, hpGuage_Y - 10.0f, 0.0f }, { 70, 70 }, { spriteOffset_x, spriteSize.y }, spriteSize, 0, { 1, 1, 1, 1 });
	}
	//HaveArm
	for (int i = 0; i < HaveArms.size(); i++)
	{
		float spriteOffset_x = spriteSize.x * (i + 1);
		float spriteOffset_y = 0;
		//if (SC_AT(i) != CurrentUseArm && SC_AT(i) == GetNextArm()) {
		//	spriteOffset_y = spriteSize.y;
		//}

		float color_a = HaveArms[SC_AT(i)] ? 1.0f : 0.3f;
		float color_rb = (SC_AT(i) == CurrentUseArm) ? 0.4f : 1.0f;
		arm->Render(dc, { 1013.0f + 65 * i, hpGuage_Y + 2.0f, 0.0f }, { 45, 45 }, { spriteOffset_x, spriteOffset_y }, spriteSize, 0, { color_rb, 1, color_rb, color_a });
	}
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
			XMFLOAT3 a = {};
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
	if (isAttacking) //攻撃中(主にジャンプ攻撃後)
	{
		// 着地してすぐは何もさせないためここで処理を書かない
		// 各StateUpdateにてアニメーション終了後にIdleStateへ遷移する
	}
	else if (InputMove(elapsedTime))
	{
		ChangeState(State::Run);
	}
	else {
		ChangeState(State::JumpEnd);
	}
}

void Player::OnDamaged()
{
	ChangeState(State::Damage);
}

void Player::OnDead()
{
	ChangeState(State::Dead);
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
void Player::RenderTrail() const
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

			XMFLOAT3 Position[2] = {};
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
		if (velocity.y > 0)
		{
			velocity.y = 0;
		}
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
bool Player::InputAttackFromNoneAttack()
{
	// 押されていない時はreturn
	if (!InputButtonDown(Player::InputState::Attack)) return false;
	enableSpecialAttack = true;

	if (targetPlayer->enableSpecialAttack
		&& targetPlayer->CurrentUseArm == this->CurrentUseArm
		) {
		switch (CurrentUseArm)
		{
		case Player::AttackType::Hammer:
			ChangeState(State::AttackHammerJump);
			break;
		case Player::AttackType::Spear:
			ChangeState(State::AttackSpearJump);
			break;
		case Player::AttackType::Sword:
			ChangeState(State::AttackSwordJump);
			break;
		}
	}
	else
	{
		switch (CurrentUseArm)
		{
		case Player::AttackType::Hammer:
			ChangeState(State::AttackHammer1);
			break;
		case Player::AttackType::Spear:
			ChangeState(State::AttackSpear1);
			break;
		case Player::AttackType::Sword:
			ChangeState(State::AttackSword1);
			break;
		}
	}
	return true;
}
bool Player::InputAttackFromJump(float elapsedTime)
{
	// 押されていない時はreturn
	if (!InputButtonDown(Player::InputState::Attack)) return false;

	switch (CurrentUseArm)
	{
	case Player::AttackType::Hammer:
		ChangeState(State::AttackHammerJump);
		break;
	case Player::AttackType::Spear:
		ChangeState(State::AttackSpearJump);
		break;
	case Player::AttackType::Sword:
		ChangeState(State::AttackSwordJump);
		break;
	}

	return true;
}

// 次の選択武器取得
Player::AttackType Player::GetNextArm()
{
	// 現在の次の番号の武器を選択する
	for (int i = SC_INT(CurrentUseArm);;)
	{
		// 回転
		if (i == SC_INT(AttackType::MaxCount) - 1) i = 0;
		else i++;
		// 全検索したらbreak
		if (i == SC_INT(CurrentUseArm)) break;

		// 未所持ならcontinue
		if (!HaveArms[SC_AT(i)]) continue;

		// 次に所持しているものを選択する
		return SC_AT(i);
	}
	// 現在の武器をそのまま返す
	return CurrentUseArm;
}

// アイテムゲット
void Player::AddHaveArm(Player::AttackType arm/* = AttackType::None*/)
{
	// 指定されていたらそれを設定する
	if (arm != AttackType::None) {
		HaveArms[arm] = true;
		return;
	}

	// 現在持っていない武器リスト
	std::unordered_map<AttackType,bool> remainArm = HaveArms;
	for (const auto& arm : HaveArms)
	{
		if (HaveArms[arm.first])
		{
			remainArm.erase(arm.first);
		}
	}

	// 全て持っている場合はreturn
	if (remainArm.size() == 0) return;

	// 乱数
	int num = rand() % remainArm.size();
	int i = 0;
	// 獲得する武器を確定する
	for (const auto& arm : remainArm)
	{
		if (i == num)
		{
			HaveArms[arm.first] = true;
			return;
		}
		i++;
	}
}

//回復遷移確認処理
bool Player::EnableRecoverTransition()
{
	// 20%以上はfalse
	if (!targetPlayer->GetHpWorning()) return false;

	XMVECTOR posPlayerthis = XMLoadFloat3(&GetPosition());
	XMVECTOR posPlayertarget = XMLoadFloat3(&targetPlayer->GetPosition());
	float distSq = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(posPlayerthis, posPlayertarget)));
	// 近く無ければfalse
	if (distSq > 3.0f * 3.0f) return false;

	// trueで遷移可能
	return true;
}

// 近距離攻撃時の角度矯正
void Player::ForceTurnByAttack(float elapsedTime)
{
	// 敵の発見時に進む方向を矯正する
	if (enemySearch[nearestEnemy] >= EnemySearch::Find)
	{
		//旋回処理
		Turn(elapsedTime, nearestVec.x, nearestVec.z, turnSpeed);
	}
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
	for (int i = 0; i < enemyManager.GetEnemyCount(); i++)
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
			// スペシャル技false
			enableSpecialAttack = false;

			// 攻撃判定しない場合はreturn
			if (attackingEnemyNumber == i && !isAttackJudge) return;

			int damage = arm.damage + static_cast<int>(currentLevel * 0.2f);
			// ダメージを与えない場合はreturn
			if (!enemy->ApplyDamage(damage, 0, this, this == &Player1P::Instance() ? 0 : 1)) return;

			currentAttackEnemy = enemy;
			attackingEnemyNumber = i;
			isAttackJudge = false;
			allDamage += damage;
			// ヒットエフェクト再生
			{
				outPosition.y += enemy->GetHeight() * enemy->GetEffectOffset_Y();
				PlayEffect(EffectNumber::Hit, outPosition);
			}

			// 吹き飛ばしは4回目以上
			if (attackCount >= 4)
			{
				// 吹き飛ばし攻撃
				const float power = 13.0f; //仮の水平の力
				const XMFLOAT3& ep = enemy->GetPosition();

				// ノックバック方向の算出
				float vx = ep.x - enemy->GetCurrentAttacker()->position.x;
				float vz = ep.z - enemy->GetCurrentAttacker()->position.z;
				float lengthXZ = sqrtf(vx * vx + vz * vz);
				vx /= lengthXZ;
				vz /= lengthXZ;

				// ノックバック
				enemy->AddImpulse({ power * vx , power * 0.8f, power * vz });
			}
		}
		else if(attackingEnemyNumber == i)//攻撃中のエネミーと一旦攻撃が外れた時、次回当たった時に判定を行う
		{
			isAttackJudge = true;
		}
	}
}

//デバッグプリミティブ描画
void Player::DrawDebugPrimitive() const
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
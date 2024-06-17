#include <map>
#include <iomanip>
#include <sstream>

#include "Player.h"
#include "Constant/UtilsDefineConst.h"
#include "Constant/UIConst.h"
#include "Character/Manager/PlayerManager.h"
#include "Character/State/Player/PlayerEachState.h"
#include "Character/Manager/EnemyManager.h"
#include "Graphics/Graphics.h"
#include "Graphics/PrimitiveRenderer.h"
#include "Input/Input.h"
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
	model = std::make_unique<Model>(device, "Data/Model/SD-UnityChan/UnityChan.fbx", model_size);

	// 武器登録
	HaveWeapons.clear();
	for (AttackType type = AttackType::Sword; type < AttackType::MaxCount; type = SC_AT(SC_INT(type) + 1))
	{
		HaveWeapons.insert(std::pair<Player::AttackType, bool>(type, false));
	}
	// 初期武器
	CurrentUseWeapon = InitialWeapon;
	HaveWeapons[InitialWeapon] = true;

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
	health = maxHealth = max_health;

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

// 更新処理
void Player::UpdateUtils(float elapsedTime)
{
	// 回復遷移可能か
	enableRecoverTransition = EnableRecoverTransition();

	// ステート毎に中で処理分け
	stateMachine->Update(elapsedTime);

	// ジャンプ処理
	UpdateJumpState(elapsedTime);

	// プレイヤーとエネミーとの衝突処理
	CollisionPlayerVsEnemies();

	// 速力処理更新
	UpdateVelocity(elapsedTime);

	// ステージの端の仮壁処理
	StageSideWall();

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
		if (dist < judge_dist_attack)
		{
			enemySearch[enemy] = EnemySearch::Attack;
		}
		else if (dist < judge_dist_find)
		{
			enemySearch[enemy] = EnemySearch::Find;
		}
		else
		{
			enemySearch[enemy] = EnemySearch::None;
			noneEnemy++;
			// エネミーがいなくなった時、現在のエネミー探索ステートをNoneにする
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

// ジャンプ処理
void Player::UpdateJumpState(float elapsedTime)
{
	// 攻撃中の場合はジャンプさせない
	if (isAttacking) return;

	switch (jumpTrg)
	{
	case JumpState::CanJump:
		// 押している間の処理
		if (InputButton(Player::InputState::Jump))
		{
			velocity.y += first_jump_speed * elapsedTime;
			// 指定加速度まであがったら
			if (velocity.y > max_jump_speed)
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
			if (velocity.y > 0) velocity.y += second_jump_speed;
			else				velocity.y = second_jump_speed;

			jumpTrg = JumpState::CannotJump;
		}
		// 一段目ジャンプ中の攻撃ボタン
		else if (InputAttackFromJump())
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

// シャドウマップ用描画
void Player::ShadowRender(const RenderContext& rc, ShadowMap* shadowMap)
{
	shadowMap->Draw(rc, model.get());
}
// 描画
void Player::Render(const RenderContext& rc, ModelShader* shader)
{
	shader->Draw(rc, model.get());
}

// HP描画
void Player::RenderHPBar(ID3D11DeviceContext* dc, Sprite* gauge, FontSprite* font)
{
	Graphics& graphics = Graphics::Instance();
	static const float screenWidth = static_cast<float>(graphics.GetScreenWidth());

	// 名前表示
	StringRender(dc, font, playerName,
		{ hp_gauge_name_position.x, hpGaugePosition_Y + hp_gauge_name_position.y },
		nameColor);

	// Lv表示
	std::string levelStr = "Lv:" + std::to_string(currentLevel);
	StringRender(dc, font, levelStr,
		{ lv_display_position_x, hpGaugePosition_Y + display_under_text_offset_y },
		nameColor);

	// HP表示;
	std::ostringstream ss;
	ss << std::setw(hp_display_digit) << std::setfill('0') << GetHealth(); // 桁数指定
	std::string hp(ss.str());
	std::string hpStr = hp + '/' + std::to_string(GetMaxHealth());
	StringRender(dc, font, hpStr,
		{ hp_display_position_x, hpGaugePosition_Y + display_under_text_offset_y },
		nameColor);

	//ゲージ描画(下地)
	gauge->Render(dc,
		(screenWidth / 2) - (hp_gauge_size.x / 2), // X_中央に配置するため幅の半分とゲージ長さの半分で求める
		hpGaugePosition_Y, // Y
		SPRITE_position_default_z, // Z
		hp_gauge_size.x + hp_gauge_frame_expansion,
		hp_gauge_size.y + hp_gauge_frame_expansion,
		SPRITE_angle_default,
		hp_gauge_frame_color // 背景カラー
	);
	//ゲージ描画
	gauge->Render(dc,
		(screenWidth / 2) - (hp_gauge_size.x / 2) + hp_gauge_frame_expansion / 2, // X_中央に配置するため幅の半分とゲージ長さの半分で求める
		hpGaugePosition_Y + hp_gauge_frame_expansion / 2, // Y_上下の拡張を合わせたサイズ分で足しているため半分足す
		SPRITE_position_default_z, // Z
		hp_gauge_size.x * (GetHealthRate() / 100.0f), //百分率を小数に変換
		hp_gauge_size.y,
		SPRITE_angle_default,
		GetHpWorning() ? hp_gauge_color_wornimg : hp_gauge_color_normal // ゲージカラー/HP減ると色を変える
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
		viewport.TopLeftX, viewport.TopLeftY,
		viewport.Width, viewport.Height,
		viewport.MinDepth, viewport.MaxDepth,
		Projection, View, World);

	XMFLOAT3 screenPosition;
	XMStoreFloat3(&screenPosition, ScreenPosition);
	//カメラの背後にいるか、明らかに離れているなら描画しない
	if (screenPosition.z > 0.0f && screenPosition.z < 1.0f) //0.0f～1.0fの間
	{
		// 名前
		StringRender(rc.deviceContext, font, playerName,
			{ screenPosition.x + name_offset.x, screenPosition.y + name_offset.y },
			nameColor);
		// メッセージ描画
		RenderCharacterMessage(rc.deviceContext, message, { screenPosition.x, screenPosition.y });
	}
}
// キャラクターメッセージ描画
void Player::RenderCharacterMessage(ID3D11DeviceContext* dc, Sprite* message, DirectX::XMFLOAT2 position)
{
	// メッセージ
	if (messageNumber == PlayerMessage::WeaponGet || messageNumber == PlayerMessage::LevelUp)
	{
		// タイマー増加
		messageYTimer += message_timer_increase;
		// タイマーで自動的にfalseに切り替えるメッセージ描画
		message->Render(dc,
			{ position.x + message_offset.x, position.y + message_offset.y - messageYTimer, SPRITE_position_default_z },
			message_size,
			{ message_sprite_size.x, message_sprite_size.y * SC_INT(messageNumber) },
			message_sprite_size,
			SPRITE_angle_default,
			SPRITE_color_default);

		// タイマーが一定時間まできたら
		if (messageYTimer > message_timer_max)
		{
			enableShowMessage[SC_INT(messageNumber)] = false;
			messageNumber = PlayerMessage::None;
		}
	}
	else if (enableShowMessage[SC_INT(PlayerMessage::WeaponGet)])
	{
		// trueになった時の初期処理
		messageNumber = PlayerMessage::WeaponGet;
		messageYTimer = 0.0f;
	}
	else if (enableShowMessage[SC_INT(PlayerMessage::LevelUp)])
	{
		// trueになった時の初期処理
		messageNumber = PlayerMessage::LevelUp;
		messageYTimer = 0.0f;
	}
	else
	{
		for (PlayerMessage mes = PlayerMessage::Attack; mes < PlayerMessage::MaxCount; mes = SC_PM(SC_INT(mes) + 1))
		{
			if (!enableShowMessage[SC_INT(mes)]) continue;

			// trueの時は常時表示するメッセージの描画
			message->Render(dc,
				{ position.x + message_offset.x, position.y + message_offset.y - messageYTimer, SPRITE_position_default_z },
				message_size,
				{ message_sprite_size.x, message_sprite_size.y * SC_INT(mes) },
				message_sprite_size,
				SPRITE_angle_default,
				SPRITE_color_default);
			break; // 一種類しか表示させないので一つ描画した時点でbreak
		}
	}
}

// 所持武器描画
void Player::RenderHaveWeapons(ID3D11DeviceContext* dc, Sprite* frame, Sprite* weapon)
{
	//HaveWeaponFrame
	for (int i = 0; i < HaveWeapons.size(); i++)
	{
		float textureCutPosition_x = 0;
		// 現在使用中の武器
		if (i == SC_INT(CurrentUseWeapon)) {
			textureCutPosition_x = WeaponIcon_sprite_size.x;
		}
		else {
			// 所持している武器かどうかでセットする
			textureCutPosition_x = HaveWeapons[SC_AT(i)] ? WeaponIcon_sprite_size.x * 2 : 0; // 2倍座標部分・0倍座標部分
		}
		frame->Render(dc,
			{ WeaponFrame_position_x + WeaponFrame_offset.x * i, hpGaugePosition_Y + WeaponFrame_offset.y, SPRITE_position_default_z },
			WeaponFrame_render_size,
			{ textureCutPosition_x, WeaponIcon_sprite_size.y }, 
			WeaponIcon_sprite_size,
			SPRITE_angle_default,
			SPRITE_color_default);
	}
	//HaveWeapon
	for (int i = 0; i < HaveWeapons.size(); i++)
	{
		float textureCutPosition_x = WeaponIcon_sprite_size.x * (i + 1); // i+1倍座標部分
		float textureCutPosition_y = 0;
		float color_a = HaveWeapons[SC_AT(i)] ? SPRITE_color_default.z : WeaponIcon_color_translucent_a; // 未所持武器を半透明にする
		float color_rb = (SC_AT(i) == CurrentUseWeapon) ? WeaponIcon_color_decrease_rb : SPRITE_color_default.y; // 使用中の武器に緑色を付ける
		weapon->Render(dc,
			{ WeaponIcon_position_x + WeaponFrame_offset.x * i, hpGaugePosition_Y + WeaponIcon_offset_y, SPRITE_position_default_z },
			WeaponIcon_size,
			{ textureCutPosition_x, textureCutPosition_y },
			WeaponIcon_sprite_size,
			SPRITE_angle_default,
			{ color_rb, SPRITE_color_default.y, color_rb, color_a });
	}
}
// 文字描画/プレイヤーUI用
void Player::StringRender(ID3D11DeviceContext* dc, FontSprite* font,
	std::string str,			// テキスト
	DirectX::XMFLOAT2 position,	// 位置
	DirectX::XMFLOAT4 color)	// 色
{
	font->Textout(dc, str,
		{ position.x, position.y, TEXT_depth_default },
		TEXT_display_size_default,
		TEXT_cut_position_default,
		TEXT_cut_size_default,
		TEXT_angle_default,
		color);
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

void Player::UpdateWeaponPositions(Model* model, Weapon& weapon)
{
	Model::Node* weaponBone = model->FindNode(weapon.nodeName);
	XMMATRIX W = XMLoadFloat4x4(&weaponBone->worldTransform);
	XMVECTOR V = weapon.tipOffset;
	XMVECTOR P = XMVector3Transform(V, W);
	XMStoreFloat3(&weapon.position, P);
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

//垂直速力更新
void Player::UpdateVerticalVelocity(float elapsedFrame)
{
	auto state = static_cast<Player::State>(stateMachine->GetStateNumber());

	// ジャンプスピアー攻撃のみ別の下向き処理を使用する
	if (state == State::AttackSpearJump)
	{
		if (velocity.y > 0) // 既にY座標上向き加速値があれば0にする
		{
			velocity.y = 0;
		}
		velocity.y += gravity * spear_jamp_attack_velocity_rate * elapsedFrame;
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
	Move(moveVec.x, moveVec.z, move_speed);
	//旋回処理
	Turn(elapsedTime, moveVec.x, moveVec.z, turn_speed);

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
		&& targetPlayer->CurrentUseWeapon == this->CurrentUseWeapon)
	{
		// ジャンプ攻撃をスペシャル技として使用する
		InputAttackFromJump();
		targetPlayer->InputAttackFromJump();
	}
	else
	{
		switch (CurrentUseWeapon)
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
bool Player::InputAttackFromJump()
{
	// 押されていない時はreturn
	if (!InputButtonDown(Player::InputState::Attack)) return false;

	// スペシャル技false
	enableSpecialAttack = false;

	switch (CurrentUseWeapon)
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
Player::AttackType Player::GetNextWeapon()
{
	// 現在の次の番号の武器を選択する
	AttackType type = CurrentUseWeapon;
	while (true)
	{
		// 次の武器を選択
		type = SC_AT(SC_INT(type) + 1);
		// 範囲外になれば初めに戻る
		if (type == AttackType::MaxCount) {
			type = AttackType::Sword;
		}

		// 全検索したらbreak
		if (type == CurrentUseWeapon) break;
		// 未所持ならcontinue
		if (!HaveWeapons[type]) continue;

		// 次に所持しているものを選択する
		return type;
	}
	// 現在の武器をそのまま返す
	return CurrentUseWeapon;
}

// アイテムゲット
void Player::AddHaveWeapon(Player::AttackType weapon/* = AttackType::None*/)
{
	// 指定されていたらそれを設定する
	if (weapon != AttackType::None) {
		HaveWeapons[weapon] = true;
		return;
	}

	// 現在持っていない武器リスト
	std::unordered_map<AttackType,bool> remainWeapon = HaveWeapons;
	for (const auto& weapon : HaveWeapons)
	{
		if (HaveWeapons[weapon.first]) {
			remainWeapon.erase(weapon.first);
		}
	}

	// 全て持っている場合はreturn
	if (remainWeapon.size() == 0) return;

	// 乱数
	int num = rand() % remainWeapon.size();
	int i = 0;
	// 獲得する武器を確定する
	for (const auto& weapon : remainWeapon)
	{
		if (i == num) {
			HaveWeapons[weapon.first] = true;
			return;
		}
		i++;
	}
}

//回復遷移確認処理
bool Player::EnableRecoverTransition()
{
	// 危険でなければfalse
	if (!targetPlayer->GetHpWorning()) return false;

	XMVECTOR thisPlayerPosition = XMLoadFloat3(&GetPosition());
	XMVECTOR targetPlayerPosition = XMLoadFloat3(&targetPlayer->GetPosition());
	// プレイヤー同士の距離
	float distSq = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(thisPlayerPosition, targetPlayerPosition)));

	// 回復可能な距離になれば遷移可能
	return distSq < recover_dist * recover_dist;
}

// 近距離攻撃時の角度矯正
void Player::ForceTurnByAttack(float elapsedTime)
{
	// 敵の発見時に進む方向を矯正する
	if (enemySearch[nearestEnemy] >= EnemySearch::Find)
	{
		//旋回処理
		Turn(elapsedTime, nearestVec.x, nearestVec.z, turn_speed);
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
			XMFLOAT3{}, outPosition))
		{
			SetPosition(outPosition);
		}
	}
}

// ステージの端の仮壁処理
void Player::StageSideWall()
{
	// 強制的な押し返し
	if (position.x > STAGE_side_max) position.x = STAGE_side_max;
	if (position.x < STAGE_side_min) position.x = STAGE_side_min;
	if (position.z > STAGE_side_max) position.z = STAGE_side_max;
	if (position.z < STAGE_side_min) position.z = STAGE_side_min;
}

// 武器とエネミーの衝突処理
void Player::CollisionWeaponsVsEnemies(Weapon weapon)
{
	//指定のノードと全ての敵を総当たりで衝突処理
	EnemyManager& enemyManager = EnemyManager::Instance();
	for (int i = 0; i < enemyManager.GetEnemyCount(); i++)
	{
		Enemy* enemy = enemyManager.GetEnemy(i);

		//衝突処理
		XMFLOAT3 outPosition;
		if (Collision::IntersectSphereVsCylinder(
			weapon.position, weapon.radius,
			enemy->GetPosition(), enemy->GetRadius(), enemy->GetHeight(),
			outPosition
		))
		{
			// スペシャル技false
			enableSpecialAttack = false;

			// 攻撃判定しない場合はreturn
			if (attackingEnemyNumber == i && !isAttackJudge) return;

			// ダメージ計算
			int damage = weapon.damage + SC_INT(currentLevel * damage_increase);
			// ダメージを与えない場合はreturn
			if (!enemy->ApplyDamage(damage, invincible_time, this, serialNumber)) return;

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
			if (attackCount >= impulse_attack_count)
			{
				// 吹き飛ばし攻撃
				const XMFLOAT3& ep = enemy->GetPosition();

				// ノックバック方向の算出
				float vx = ep.x - enemy->GetCurrentAttacker()->position.x;
				float vz = ep.z - enemy->GetCurrentAttacker()->position.z;
				float lengthXZ = sqrtf(vx * vx + vz * vz);
				vx /= lengthXZ;
				vz /= lengthXZ;
				// Y方向の調整
				float vy = impulse_power_adjust_rate_y;

				// ノックバック
				enemy->AddImpulse({ impulse_power * vx , impulse_power * vy, impulse_power * vz });
			}
		}
		else if(attackingEnemyNumber == i)//攻撃中のエネミーと一旦攻撃が外れた時、次回当たった時に判定を行う
		{
			isAttackJudge = true;
		}
	}
}

#pragma region DEBUG_DRAW
//デバッグプリミティブ描画
void Player::DrawDebugPrimitive() const
{
	Gizmos* gizmos = Graphics::Instance().GetGizmos();
	//衝突判定用のデバッグ円柱を描画
	gizmos->DrawCylinder(
		position,		//位置
		radius,			//半径
		height,			//高さ
		GIZMOS_cylinder_angle_default,		//角度
		GIZMOS_color_red);	//色

	if (Hammer.flag1 || Hammer.flag2 || Hammer.flagJump)
	{
		gizmos->DrawSphere(
			Hammer.position,
			Hammer.radius,
			GIZMOS_color_blue
		);
	}
	if (Spear.flag1 || Spear.flag2 || Spear.flag3 || Spear.flagJump)
	{
		gizmos->DrawSphere(
			Spear.position,
			Spear.radius,
			GIZMOS_color_blue
		);
	}
	if (Sword.flag1 || Sword.flag2 || Sword.flag3 || Sword.flagJump)
	{
		gizmos->DrawSphere(
			Sword.position,
			Sword.radius,
			GIZMOS_color_blue
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
#pragma endregion
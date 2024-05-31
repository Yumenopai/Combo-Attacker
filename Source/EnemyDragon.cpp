#include "EnemyDragon.h"
#include "Graphics/Graphics.h"
#include "PlayerManager.h"
#include "Collision.h"
#include "AnimationTimeStruct.h"
#include "SceneManager.h"
#include "SceneLoading.h"
#include "SceneClear.h"
#include "imgui.h"

//コンストラクタ
EnemyDragon::EnemyDragon()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	model = std::make_unique<Model>(device, dragon_model_file, dragon_render_size);

	radius = dragon_radius;
	height = dragon_height;
	health = maxHealth = dragon_max_health;
	attackDamage = dragon_attack_damage;

	//待機ステートへ遷移
	TransitionState(State::Idle);
}

//デストラクタ
EnemyDragon::~EnemyDragon()
{
}

//ステート遷移
void EnemyDragon::TransitionState(State nowState)
{
	state = nowState; //ステート設定

	switch (nowState)
	{
	case State::Wander:
		//目標地点設定
		SetRandomTargetPosition();
	}

	TransitionPlayAnimation(nowState);
}

//遷移時アニメーション再生
void EnemyDragon::TransitionPlayAnimation(State nowState)
{
	//アニメーション設定
	Animation anime = Animation::Idle1;
	bool animeLoop = true;

	switch (nowState)
	{
	case State::Idle:
		anime = Animation::Idle1;
		break;
	case State::Wander:
		anime = Animation::Walk;
		break;
	case State::Pursuit:
		anime = Animation::Run;
		break;
	case State::Attack:
		anime = Animation::Attack01;
		animeLoop = false;
		break;
	case State::IdleBattle:
		anime = Animation::Idle02;
		break;
	case State::Scream:
		anime = Animation::Scream;
		animeLoop = false;
		break;
	case State::AttackClaw:
		anime = Animation::AttackClaw;
		animeLoop = false;
		break;
	case State::GetHit:
		anime = Animation::GetDamage;
		animeLoop = false;
		break;
	case State::Die:
		anime = Animation::Die;
		animeLoop = false;
		break;
	}

	//アニメーション再生
	model->PlayAnimation(static_cast<int>(anime), animeLoop);
}

//更新処理
void EnemyDragon::Update(float elapsedTime)
{
	UpdateTargetPosition();

	//ステート毎の更新処理
	switch (state)
	{
	case State::Wander:
		UpdateWanderState(elapsedTime);
		break;
	case State::Idle:
		UpdateIdleState(elapsedTime);
		break;
	case State::Pursuit:
		UpdatePursuitState(elapsedTime);
		break;
	case State::Attack:
		UpdateAttackState(elapsedTime);
		break;
	case State::IdleBattle:
		UpdateIdleBattleState(elapsedTime);
		break;
	case State::GetHit:
		UpdateGetHitState(elapsedTime);
		break;
	case State::Scream:
		UpdateScreamState(elapsedTime);
		break;
	case State::AttackClaw:
		UpdateAttackClawState(elapsedTime);
		break;
	case State::Die:
		UpdateDieState(elapsedTime);
		break;
	}

	//速力処理更新
	UpdateVelocity(elapsedTime);

	//無敵時間更新
	UpdateInvincibleTimer(elapsedTime);

	//オブジェクト行列を更新
	UpdateTransform();

	//モデルアニメーション更新
	model->UpdateAnimation(elapsedTime);

	//モデル行列更新
	model->UpdateTransform(transform);
}

//描画処理
void EnemyDragon::ShadowRender(const RenderContext& rc, ShadowMap* shadowMap)
{
	shadowMap->Draw(rc, model.get());
}
void EnemyDragon::Render(const RenderContext& rc, ModelShader* shader)
{
	shader->Draw(rc, model.get());
}

//ダメージ時に呼ばれる
void EnemyDragon::OnDamaged()
{
	if (!isHalfHP && health <= maxHealth / 2) // 半分まできたら
	{
		//ダメージステートへ遷移
		TransitionState(State::GetHit);
		isHalfHP = true;
	}
}
//死亡した時に呼ばれる
void EnemyDragon::OnDead()
{
	if (isDead) return;

	int i = 0;
	// 与えたダメージ量が少なすぎるとLevelをあげない
	for (Player* player : PlayerManager::Instance().players)
	{
		if (attackedDamage[i] > (maxHealth / add_level_min_damage_rate)) {
			player->AddLevel(dragon_add_level_up);
		}
		i++;
	}
	// 最もダメージを与えたプレイヤーはさらにLevelUp
	GetMostAttackPlayer()->AddLevel(most_attack_bonus_level_up);

	//死亡ステートへ遷移
	TransitionState(State::Die);
	isDead = true;
}

Player::EnemySearch EnemyDragon::GetNearestPlayer_EnemySearch()
{
	Player::EnemySearch es = Player::EnemySearch::None;

	for (Player* player : PlayerManager::Instance().players)
	{
		if (static_cast<int>(es) < static_cast<int>(player->GetEachEnemySearch(this)))
			es = player->GetEachEnemySearch(this);
	}
	return es;
}

//ターゲット位置を設定
void EnemyDragon::UpdateTargetPosition()
{
	//プレイヤーとの高低差を考慮して3Dでの距離判定をする
	minLen = FLT_MAX;
	for (Player* player : PlayerManager::Instance().players)
	{
		nowLen = player->GetEachEnemyDist(this);
		if (nowLen < minLen)
		{
			minLen = nowLen;
			targetPosition = player->GetPosition();
		}
	}
}

//ターゲット位置をランダム設定
void EnemyDragon::SetRandomTargetPosition()
{
	float theta = 0;
	float range = 0;

	targetPosition.x = territoryOrigin.x + range * sinf(theta);
	targetPosition.y = territoryOrigin.y;
	targetPosition.z = territoryOrigin.z + range * cosf(theta);
}

//目標地点へ移動
void EnemyDragon::MoveToTarget(float elapsedTime, float speedRate)
{
	//ターゲット方向への進行ベクトルを算出
	float vx = targetPosition.x - position.x;
	float vz = targetPosition.z - position.z;
	float dist = sqrtf(vx * vx + vz * vz);
	vx /= dist;
	vz /= dist;

	//移動処理
	Move(vx, vz, dragon_move_speed * speedRate);
	Turn(elapsedTime, vx, vz, dragon_turn_speed * speedRate);
}

//プレイヤー索敵
bool EnemyDragon::SearchPlayer()
{
	Player::EnemySearch es = GetNearestPlayer_EnemySearch();
	if (es >= Player::EnemySearch::Find)
	{
		return true;
	}
	return false;
}

//徘徊ステート更新処理
void EnemyDragon::UpdateWanderState(float elapsedTime)
{
	//目標地点へ移動
	MoveToTarget(elapsedTime, 0.5f);// 速度レート指定：通常の50%速度

	//プレイヤー索敵
	if (SearchPlayer())
	{
		//見つかったら追跡ステートへ遷移
		TransitionState(State::Pursuit);
	}
}

//待機ステート更新処理
void EnemyDragon::UpdateIdleState(float elapsedTime)
{
	//プレイヤー索敵
	if (SearchPlayer())
	{
		//見つかったら追跡ステートへ遷移
		TransitionState(State::Pursuit);
	}
}

//追跡ステート更新処理
void EnemyDragon::UpdatePursuitState(float elapsedTime)
{
	//目標地点へ移動
	MoveToTarget(elapsedTime);

	//プレイヤーが近づくと攻撃ステートへ遷移
	Player::EnemySearch es = GetNearestPlayer_EnemySearch();

	if (es >= Player::EnemySearch::Attack)
	{
		//攻撃ステートへ遷移
		isHalfHP ? TransitionState(State::AttackClaw): TransitionState(State::Attack);
	}
	else if (!SearchPlayer())
	{
		TransitionState(State::Idle);
	}
}

//攻撃ステート更新処理
void EnemyDragon::UpdateAttackState(float elapsedTime)
{
	//任意のアニメーション再生区間でのみ衝突判定処理をする
	float animationTime = model->GetCurrentAnimationSeconds();
	if (IsDuringTime(animationTime, dragon_attack_head_time))
	{
		//頭ノードとプレイヤーの衝突処理
		CollisionNodeVsPlayer(dragon_head_node_name, attack_node_radius);
	}	

	//攻撃アニメーションが終わったら戦闘待機ステートへ遷移
	if (!model->IsPlayAnimation())
	{
		TransitionState(State::IdleBattle);
	}
}

//戦闘待機ステート更新処理
void EnemyDragon::UpdateIdleBattleState(float elapsedTime)
{
	//プレイヤーが攻撃範囲にいた場合は攻撃ステートへ遷移
	Player::EnemySearch es = GetNearestPlayer_EnemySearch();

	if (es >= Player::EnemySearch::Attack)
	{
		//攻撃ステートへ遷移
		isHalfHP ? TransitionState(State::AttackClaw) : TransitionState(State::Attack);
	}
	else
	{
		//待機ステートへ遷移
		TransitionState(State::Idle);
	}
}

//咆哮ステート更新処理
void EnemyDragon::UpdateScreamState(float elapsedTime)
{
	if (model->IsPlayAnimation()) return;
	
	//アニメーション終了後
	{
		//攻撃ステートへ遷移
		TransitionState(State::IdleBattle);
	}
}

//翼攻撃ステート更新処理
void EnemyDragon::UpdateAttackClawState(float elapsedTime)
{
	//任意のアニメーション再生区間でのみ衝突判定処理をする
	float animationTime = model->GetCurrentAnimationSeconds();
	if (IsDuringTime(animationTime, dragon_attack_claw_time))
	{
		//爪ノードとプレイヤーの衝突処理
		CollisionNodeVsPlayer(dragon_clawL_node_name, attack_node_radius);
		CollisionNodeVsPlayer(dragon_clawR_node_name, attack_node_radius);
	}

	//攻撃アニメーションが終わったら戦闘待機ステートへ遷移
	if (!model->IsPlayAnimation())
	{
		TransitionState(State::IdleBattle);
	}
}

//戦闘待機ステート更新処理
void EnemyDragon::UpdateGetHitState(float elapsedTime)
{
	if (model->IsPlayAnimation()) return;
	
	//アニメーション終了後
	{
		//咆哮ステートへ遷移
		TransitionState(State::Scream);
	}
}

//死亡ステート更新処理
void EnemyDragon::UpdateDieState(float elapsedTime)
{
	//アニメーション再生中は何もしない
	if (model->IsPlayAnimation()) return;
	
	//アニメーション終了に合わせエフェクト再生
	PlayEffect(EffectNumber::dead, position);

	//破棄
	Destroy();
	
	// クリア画面へ遷移
	SceneManager::Instance().ChangeScene(new SceneLoading(new SceneClear(), -255));
}

//ノードとプレイヤーの衝突処理
void EnemyDragon::CollisionNodeVsPlayer(const char* nodeName, float nodeRadius)
{
	//ノード取得
	Model::Node* node = model->FindNode(nodeName);
	if (node == nullptr) return;

	//ノード位置取得
	XMFLOAT3 nodePosition = {};
	nodePosition.x = node->worldTransform._41;
	nodePosition.y = node->worldTransform._42;
	nodePosition.z = node->worldTransform._43;

	//衝突処理
	for (Player* player : PlayerManager::Instance().players)
	{
		XMFLOAT3 outPosition;
		// 当たっていない場合リターン
		if (!Collision::IntersectSphereVsCylinder(
			nodePosition, nodeRadius,
			player->GetPosition(), player->GetRadius(), player->GetHeight(),
			outPosition)) return;
		// ダメージを与えない場合リターン
		if (!player->ApplyDamage(attackDamage)) return;

		// 敵を吹っ飛ばすベクトルを算出
		XMFLOAT3 vec = {};
		vec.x = outPosition.x - nodePosition.x;
		vec.z = outPosition.z - nodePosition.z;
		float length = sqrtf(vec.x * vec.x + vec.z * vec.z);
		vec.x /= length;
		vec.z /= length;

		// XZ平面に吹っ飛ばす力をかける
		vec.x *= dragon_impulse_power_rate;
		vec.z *= dragon_impulse_power_rate;
		//Y方向は矯正した上側への力をかける
		vec.y = dragon_impulse_power_y;

		// 吹っ飛ばす
		player->AddImpulse(vec);
	}
}

#pragma region DEBUG_DRAW
void EnemyDragon::DebugMenu()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	// デバッグ文字列表示の変更
	std::string str = "";
	// 現在のステート番号に合わせてデバッグ文字列をstrに格納
	switch (state)
	{
	case State::Wander:
		str = "Wander";
		break;
	case State::Idle:
		str = "Idle";
		break;
	case State::Pursuit:
		str = "Pursuit";
		break;
	case State::Attack:
		str = "Attack";
		break;
	case State::IdleBattle:
		str = "IdleBattle";
		break;
	case State::GetHit:
		str = "GetHit";
		break;
	default:
		break;
	}

	if (ImGui::Begin("Dragon", nullptr, ImGuiWindowFlags_None))
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

			ImGui::Text(u8"State　%s", str.c_str());
			ImGui::Text(u8"nearestPlayer　%s", nearestPlayerStr.c_str());
			ImGui::Text(u8"minLen　%f", minLen);
			ImGui::Text(u8"nowLen　%f", nowLen);
		}
		ImGui::End();
	}
}
#pragma endregion
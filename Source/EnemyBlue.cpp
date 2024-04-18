#include "EnemyBlue.h"
#include "Graphics/Graphics.h"
#include "PlayerManager.h"
#include "Collision.h"
#include "imgui.h"

//コンストラクタ
EnemyBlue::EnemyBlue()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	model = std::make_unique<Model>(device, "Data/Model/Enemy/Blue.fbx", 0.008f);

	height = 3.6f;
	radius = 1.8f;
	health = maxHealth = 100;
	isHalfHP = false;

	//待機ステートへ遷移
	TransitionState(State::Idle);
}

//デストラクタ
EnemyBlue::~EnemyBlue()
{
}

//ステート遷移
void EnemyBlue::TransitionState(State nowState)
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
void EnemyBlue::TransitionPlayAnimation(State nowState)
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
void EnemyBlue::Update(float elapsedTime)
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
void EnemyBlue::ShadowRender(const RenderContext& rc, ShadowMap* shadowMap)
{
	shadowMap->Draw(rc, model.get());
}
void EnemyBlue::Render(const RenderContext& rc, ModelShader* shader)
{
	shader->Draw(rc, model.get());
}

//縄張り設定
void EnemyBlue::SetTerritory(const DirectX::XMFLOAT3& origin, float range)
{
	territoryOrigin = origin;
	territoryRange = range;
}

//ダメージ時に呼ばれる
void EnemyBlue::OnDamaged()
{
	if (health <= 50 && !isHalfHP)
	{
		//ダメージステートへ遷移
		TransitionState(State::GetHit);
		isHalfHP = true;
	}
}
//死亡した時に呼ばれる
void EnemyBlue::OnDead()
{
	if (!isDead)
	{
		//死亡ステートへ遷移
		TransitionState(State::Die);
		isDead = true;
	}
}

Player::EnemySearch EnemyBlue::GetNearestPlayerES()
{
	Player::EnemySearch es = Player::EnemySearch::None;
	//es = Player1P::Instance().GetEachEnemySearch(this);

	for (Player* player : PlayerManager::Instance().players)
	{
		if (static_cast<int>(es) < static_cast<int>(player->GetEachEnemySearch(this)))
			es = player->GetEachEnemySearch(this);
	}
	return es;
}

//ターゲット位置を設定
void EnemyBlue::UpdateTargetPosition()
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

	//targetPosition = Player1P::Instance().GetPosition();
}

//ターゲット位置をランダム設定
void EnemyBlue::SetRandomTargetPosition()
{
	float theta = 0;
	float range = 0;

	targetPosition.x = territoryOrigin.x + range * sinf(theta);
	targetPosition.y = territoryOrigin.y;
	targetPosition.z = territoryOrigin.z + range * cosf(theta);
}

//目標地点へ移動
void EnemyBlue::MoveToTarget(float elapsedTime, float speedRate)
{
	//ターゲット方向への進行ベクトルを算出
	float vx = targetPosition.x - position.x;
	float vz = targetPosition.z - position.z;
	float dist = sqrtf(vx * vx + vz * vz);
	vx /= dist;
	vz /= dist;

	//移動処理
	Move(vx, vz, moveSpeed * speedRate);
	Turn(elapsedTime, vx, vz, turnSpeed * speedRate);
}

//プレイヤー索敵
bool EnemyBlue::SearchPlayer()
{
	Player::EnemySearch es = GetNearestPlayerES();
	if (es >= Player::EnemySearch::Find)
	{
		return true;
	}
	return false;
}

//徘徊ステート更新処理
void EnemyBlue::UpdateWanderState(float elapsedTime)
{
	//目標地点へ移動
	MoveToTarget(elapsedTime, 0.5f);

	//プレイヤー索敵
	if (SearchPlayer())
	{
		//見つかったら追跡ステートへ遷移
		TransitionState(State::Pursuit);
	}
}

//待機ステート更新処理
void EnemyBlue::UpdateIdleState(float elapsedTime)
{
	//プレイヤー索敵
	if (SearchPlayer())
	{
		//見つかったら追跡ステートへ遷移
		TransitionState(State::Pursuit);
	}
}

//追跡ステート更新処理
void EnemyBlue::UpdatePursuitState(float elapsedTime)
{
	//目標地点へ移動
	MoveToTarget(elapsedTime, 1.0f);

	//プレイヤーが近づくと攻撃ステートへ遷移
	Player::EnemySearch es = GetNearestPlayerES();

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
void EnemyBlue::UpdateAttackState(float elapsedTime)
{
	//任意のアニメーション再生区間でのみ衝突判定処理をする
	float animationTime = model->GetCurrentAnimationSeconds();
	if (animationTime >= 0.35f && animationTime <= 0.6f)
	{
		//頭ノードとプレイヤーの衝突処理
		CollisionNodeVsPlayer("Jaw3", 0.2f);

	}	

	//攻撃アニメーションが終わったら戦闘待機ステートへ遷移
	if (!model->IsPlayAnimation())
	{
		TransitionState(State::IdleBattle);
	}
}

//戦闘待機ステート更新処理
void EnemyBlue::UpdateIdleBattleState(float elapsedTime)
{
	//プレイヤーが攻撃範囲にいた場合は攻撃ステートへ遷移
	Player::EnemySearch es = GetNearestPlayerES();

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

	MoveToTarget(elapsedTime, 0.0f);
}

//咆哮ステート更新処理
void EnemyBlue::UpdateScreamState(float elapsedTime)
{
	if (model->IsPlayAnimation()) return;
	
	//アニメーション終了後
	{
		//攻撃ステートへ遷移
		TransitionState(State::IdleBattle);
	}
}

//翼攻撃ステート更新処理
void EnemyBlue::UpdateAttackClawState(float elapsedTime)
{
	//任意のアニメーション再生区間でのみ衝突判定処理をする
	float animationTime = model->GetCurrentAnimationSeconds();
	if (animationTime >= 0.4f && animationTime <= 1.05f)
	{
		//爪ノードとプレイヤーの衝突処理
		CollisionNodeVsPlayer("WingClaw2_L", 1);
		CollisionNodeVsPlayer("WingClaw2_L_1", 1);
	}

	//攻撃アニメーションが終わったら戦闘待機ステートへ遷移
	if (!model->IsPlayAnimation())
	{
		TransitionState(State::IdleBattle);
	}
}

//戦闘待機ステート更新処理
void EnemyBlue::UpdateGetHitState(float elapsedTime)
{
	if (model->IsPlayAnimation()) return;
	
	//アニメーション終了後
	{
		//咆哮ステートへ遷移
		TransitionState(State::Scream);
	}
}

//死亡ステート更新処理
void EnemyBlue::UpdateDieState(float elapsedTime)
{
	//アニメーション再生中は何もしない
	if (model->IsPlayAnimation()) return;
	
	//アニメーション終了に合わせエフェクト再生
	PlayEffect(EffectNumber::dead, position);

	//破棄
	Destroy();
}

//ノードとプレイヤーの衝突処理
void EnemyBlue::CollisionNodeVsPlayer(const char* nodeName, float nodeRadius)
{
	//ノード取得
	Model::Node* node = model->FindNode(nodeName);
	if (node == nullptr) return;

	//ノード位置取得
	XMFLOAT3 nodePosition;
	nodePosition.x = node->worldTransform._41;
	nodePosition.y = node->worldTransform._42;
	nodePosition.z = node->worldTransform._43;

	//衝突処理
	for (Player* player : PlayerManager::Instance().players)
	{
		XMFLOAT3 outPosition;
		if (Collision::IntersectSphereVsCylinder(
			nodePosition, nodeRadius,
			player->GetPosition(), player->GetRadius(), player->GetHeight(),
			outPosition
		))
		{
			//ダメージを与える
			if (player->ApplyDamage(2, 0))
			{
				//敵を吹っ飛ばすベクトルを算出
				XMFLOAT3 vec;
				vec.x = outPosition.x - nodePosition.x;
				vec.z = outPosition.z - nodePosition.z;
				float length = sqrtf(vec.x * vec.x + vec.z * vec.z);
				vec.x /= length;
				vec.z /= length;

				//XZ平面に吹っ飛ばす力をかける
				float power = 10.0f;
				vec.x *= power;
				vec.z *= power;
				//Y方向にも力をかける
				vec.y = 5.0f;

				//吹っ飛ばす
				player->AddImpulse(vec);
			}
		}
	}
}

void EnemyBlue::DebugMenu()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);


	// デバッグ文字列表示の変更
	std::string str = "";
	// 現在のステート番号に合わせてデバッグ文字列をstrに格納
	switch (state)
	{
	case EnemyBlue::State::Wander:
		str = "Wander";
		break;
	case EnemyBlue::State::Idle:
		str = "Idle";
		break;
	case EnemyBlue::State::Pursuit:
		str = "Pursuit";
		break;
	case EnemyBlue::State::Attack:
		str = "Attack";
		break;
	case EnemyBlue::State::IdleBattle:
		str = "IdleBattle";
		break;
	case EnemyBlue::State::GetHit:
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

			ImGui::Text(u8"State　%s", str.c_str());
			ImGui::Text(u8"nearestPlayer　%s", nearestPlayerStr.c_str());
			ImGui::Text(u8"minLen　%f", minLen);
			ImGui::Text(u8"nowLen　%f", nowLen);


		}

		ImGui::End();
	}
}
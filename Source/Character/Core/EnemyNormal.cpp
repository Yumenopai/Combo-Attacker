#include "EnemyNormal.h"
#include "Constant/EnemyConst.h"
#include "Graphics/Graphics.h"
#include "Character/Manager/PlayerManager.h"
#include "Collision.h"
#include "MathScript.h"
#include "AnimationTimeStruct.h"

//コンストラクタ
EnemyNormal::EnemyNormal()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	model = std::make_unique<Model>(device, slime_model_file, slime_render_size);
}

//デストラクタ
EnemyNormal::~EnemyNormal()
{
}

void EnemyNormal::Init()
{
	//待機ステートへ遷移
	TransitionState(State::Idle);
}

//更新処理
void EnemyNormal::Update(float elapsedTime)
{
	//最近Playerを設定
	UpdateTargetPosition();

	// 各ステージごとの更新処理
	UpdateEachState(elapsedTime);

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

// 各ステージごとの更新処理
void EnemyNormal::UpdateEachState(float elapsedTime)
{
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
	case State::HitDamage:
		UpdateHitDamageState(elapsedTime);
		break;
	}
}

//描画処理
void EnemyNormal::ShadowRender(const RenderContext& rc, ShadowMap* shadowMap)
{
	shadowMap->Draw(rc, model.get());
}
void EnemyNormal::Render(const RenderContext& rc, ModelShader* shader)
{
	shader->Draw(rc, model.get());
}

//ダメージ時に呼ばれる
void EnemyNormal::OnDamaged()
{
	//ダメージステートへ遷移
	TransitionState(State::HitDamage);
}
//死亡した時に呼ばれる
void EnemyNormal::OnDead()
{
	int i = 0;
	// 与えたダメージ量が少なすぎるとLevelをあげない
	for (Player* player : PlayerManager::Instance().players)
	{
		if (static_cast<float>(attackedDamage[i]) / maxHealth > add_level_min_damage_rate) {
			player->AddLevel(slime_add_level_up);
		}
		i++;
	}
	// 最もダメージを与えたプレイヤーはさらにLevelUp
	GetMostAttackPlayer()->AddLevel(most_attack_bonus_level_up);

	//自身を破棄
	Destroy();
}

Player::EnemySearch EnemyNormal::GetNearestPlayer_EnemySearch()
{
	Player::EnemySearch es = Player::EnemySearch::None;

	for (Player* player : PlayerManager::Instance().players)
	{
		if (SC_INT(es) < SC_INT(player->GetEachEnemySearch(this)))
			es = player->GetEachEnemySearch(this);
	}
	return es;
}

//ターゲット位置を設定
void EnemyNormal::UpdateTargetPosition()
{
	//プレイヤーとの高低差を考慮して3Dでの距離判定をする
	float minLen = FLT_MAX;
	for (Player* player : PlayerManager::Instance().players)
	{
		float nowLen = player->GetEachEnemyDist(this);
		if (nowLen < minLen)
		{
			minLen = nowLen;
			targetPosition = player->GetPosition();
		}
	}
	//targetPosition = Player1P::Instance().GetPosition();
}

//目標地点へ移動
void EnemyNormal::MoveToTarget(float elapsedTime, float speedRate)
{
	//ターゲット方向への進行ベクトルを算出
	float vx = targetPosition.x - position.x;
	float vz = targetPosition.z - position.z;
	float dist = sqrtf(vx * vx + vz * vz);
	vx /= dist;
	vz /= dist;

	//移動処理
	Move(vx, vz, slime_move_speed * speedRate);
	Turn(elapsedTime, vx, vz, slime_turn_speed * speedRate);
}

//最近Playerへの回転
void EnemyNormal::TurnToTarget(float elapsedTime, float speedRate)
{
	//ターゲット方向への進行ベクトルを算出
	float vx = targetPosition.x - position.x;
	float vz = targetPosition.z - position.z;
	float dist = XMVectorGetX(XMVector3Length(XMVectorSubtract(
		XMLoadFloat3(&targetPosition), XMLoadFloat3(&position)
	)));

	vx /= dist;
	vz /= dist;

	//回転処理
	Turn(elapsedTime, vx, vz, slime_turn_speed * speedRate);
}

//プレイヤー索敵
bool EnemyNormal::SearchPlayer()
{
	float vx = targetPosition.x - position.x;
	float vy = targetPosition.y - position.y;
	float vz = targetPosition.z - position.z;
	float dist = vx * vx + vy * vy + vz * vz;

	if (dist < slime_attack_range * slime_attack_range)//必ず見つける
	{
		return true;
	}

	if (dist < slime_search_range * slime_search_range)
	{
		float distXZ = sqrtf(vx * vx + vz * vz);
		//単位ベクトル化
		vx /= distXZ;
		vz /= distXZ;
		//前方ベクトル
		float frontX = sinf(angle.y);
		float frontZ = cosf(angle.y);
		//2つのベクトル内積値で前後判定
		float dot = (frontX * vx) + (frontZ * vz);
		if (dot > 0.0f)
		{
			return true;
		}
	}
	return false;
}

//ノードとプレイヤーの衝突処理
void EnemyNormal::CollisionNodeVsPlayer(const char* nodeName, float nodeRadius)
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
		if (Collision::IntersectSphereVsCylinder(
			nodePosition, nodeRadius,
			player->GetPosition(), player->GetRadius(), player->GetHeight(),
			outPosition
		))
		{
			//ダメージを与える
			if (player->ApplyDamage(attackDamage))
			{
				//敵を吹っ飛ばすベクトルを算出
				XMFLOAT3 vec = {};
				vec.x = outPosition.x - nodePosition.x;
				vec.z = outPosition.z - nodePosition.z;
				float length = sqrtf(vec.x * vec.x + vec.z * vec.z);
				vec.x /= length;
				vec.z /= length;

				//XZ平面に吹っ飛ばす力をかける
				vec.x *= slime_impulse_power_rate;
				vec.z *= slime_impulse_power_rate;
				//Y方向は矯正した上側への力をかける
				vec.y = slime_impulse_power_y;

				//吹っ飛ばす
				player->AddImpulse(vec);
			}
		}
	}
}

/****************更新処理****************/

//徘徊ステート更新処理
void EnemyNormal::UpdateWanderState(float elapsedTime)
{
	//目標地点へ回転
	TurnToTarget(elapsedTime, slime_turn_speed);

	//プレイヤー索敵
	if (SearchPlayer())
	{
		//見つかったら追跡ステートへ遷移
		TransitionState(State::Pursuit);
	}
}

//待機ステート更新処理
void EnemyNormal::UpdateIdleState(float elapsedTime)
{
	//プレイヤー索敵
	if (SearchPlayer())
	{
		//見つかったら追跡ステートへ遷移
		TransitionState(State::Pursuit);
	}
}

//追跡ステート更新処理
void EnemyNormal::UpdatePursuitState(float elapsedTime)
{
	//目標へ回転
	TurnToTarget(elapsedTime, slime_turn_speed);

	//プレイヤーが攻撃範囲にいた場合は攻撃ステートへ遷移
	Player::EnemySearch es = GetNearestPlayer_EnemySearch();

	if (es >= Player::EnemySearch::Attack)
	{
		//攻撃ステートへ遷移
		TransitionState(State::Attack);
	}
	else if (!SearchPlayer())
	{
		TransitionState(State::Idle);
	}
}

//攻撃ステート更新処理
void EnemyNormal::UpdateAttackState(float elapsedTime)
{
	//目標へ回転
	TurnToTarget(elapsedTime, slime_turn_speed);

	//任意のアニメーション再生区間でのみ衝突判定処理をする
	float animationTime = model->GetCurrentAnimationSeconds();
	if (IsDuringTime(animationTime, slime_attack_time))
	{
		//目玉ノードとプレイヤーの衝突処理
		CollisionNodeVsPlayer(eye_ball_node_name, attack_node_radius);
	}

	//攻撃アニメーションが終わったら戦闘待機ステートへ遷移
	if (!model->IsPlayAnimation())
	{
		TransitionState(State::IdleBattle);
	}
}

//戦闘待機ステート更新処理
void EnemyNormal::UpdateIdleBattleState(float elapsedTime)
{
	//プレイヤーが攻撃範囲にいた場合は攻撃ステートへ遷移
	Player::EnemySearch es = GetNearestPlayer_EnemySearch();

	if (es >= Player::EnemySearch::Attack)
	{
		//攻撃ステートへ遷移
		TransitionState(State::Attack);
	}
	else
	{
		//待機ステートへ遷移
		TransitionState(State::Idle);
	}
	}

//ダメージステート更新処理
void EnemyNormal::UpdateHitDamageState(float elapsedTime)
{
	if (model->IsPlayAnimation()) return;

	//プレイヤーが攻撃範囲にいた場合は攻撃ステートへ遷移
	Player::EnemySearch es = GetNearestPlayer_EnemySearch();

	if (es >= Player::EnemySearch::Attack)
	{
		//攻撃ステートへ遷移
		TransitionState(State::IdleBattle);
	}
	else
	{
		//待機ステートへ遷移
		TransitionState(State::Idle);
	}
}

//ステート遷移
void EnemyNormal::TransitionState(State nowState)
{
	state = nowState; //ステート設定
	TransitionPlayAnimation(nowState);
}
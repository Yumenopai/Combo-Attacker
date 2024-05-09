#include "EnemySlime.h"
#include "Graphics/Graphics.h"
#include "PlayerManager.h"
#include "Collision.h"
#include "MathScript.h"

//コンストラクタ
EnemySlime::EnemySlime()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	model = std::make_unique<Model>(device, "Data/Model/RPG_Slime/SlimePBR.fbx", 0.01f);
	eyeBallNodeName = "EyeBall";

	angle.y = Math::RandomRange(-360, 360);
	//radius = 0.5f;
	height = 1.0f;
	health = maxHealth = 30;

	damage = 2;
	//待機ステートへ遷移
	TransitionState(State::Idle);
}

//デストラクタ
EnemySlime::~EnemySlime()
{
}

//更新処理
void EnemySlime::Update(float elapsedTime) 
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
void EnemySlime::UpdateEachState(float elapsedTime)
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
void EnemySlime::ShadowRender(const RenderContext& rc, ShadowMap* shadowMap)
{
	shadowMap->Draw(rc, model.get());
}
void EnemySlime::Render(const RenderContext& rc, ModelShader* shader)
{
	shader->Draw(rc, model.get());
}

//縄張り設定
void EnemySlime::SetTerritory(const DirectX::XMFLOAT3& origin, float range)
{
	territoryOrigin = origin;
	territoryRange = range;
}

//ダメージ時に呼ばれる
void EnemySlime::OnDamaged()
{
	//ダメージステートへ遷移
	TransitionState(State::HitDamage);
}
//死亡した時に呼ばれる
void EnemySlime::OnDead()
{
	//自身を破棄
	Destroy();
}

Player::EnemySearch EnemySlime::GetNearestPlayerES()
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

//ターゲット位置をランダム設定
void EnemySlime::SetRandomTargetPosition()
{
	float theta = Math::RandomRange(0, DirectX::XM_2PI);
	float range = Math::RandomRange(0, territoryRange);

	targetPosition.x = territoryOrigin.x + range * sinf(theta);
	targetPosition.y = territoryOrigin.y;
	targetPosition.z = territoryOrigin.z + range * cosf(theta);
}

//ターゲット位置を設定
void EnemySlime::UpdateTargetPosition()
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
void EnemySlime::MoveToTarget(float elapsedTime, float speedRate)
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

//最近Playerへの回転
void EnemySlime::TurnToTarget(float elapsedTime, float speedRate)
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
	Turn(elapsedTime, vx, vz, turnSpeed * speedRate);
}

//プレイヤー索敵
bool EnemySlime::SearchPlayer()
{
	float vx = targetPosition.x - position.x;
	float vy = targetPosition.y - position.y;
	float vz = targetPosition.z - position.z;
	float dist = vx * vx + vy * vy + vz * vz;

	if (dist < attackRange * attackRange)//必ず見つける
	{
		return true;
	}

	if (dist < searchRange * searchRange)
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
void EnemySlime::CollisionNodeVsPlayer(const char* nodeName, float nodeRadius)
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
			if (player->ApplyDamage(damage))
			{
				//敵を吹っ飛ばすベクトルを算出
				XMFLOAT3 vec;
				vec.x = outPosition.x - nodePosition.x;
				vec.z = outPosition.z - nodePosition.z;
				float length = sqrtf(vec.x * vec.x + vec.z * vec.z);
				vec.x /= length;
				vec.z /= length;

				//XZ平面に吹っ飛ばす力をかける
				float power = 3.0f;
				vec.x *= power;
				vec.z *= power;
				//Y方向にも力をかける
				vec.y = 4.0f;

				//吹っ飛ばす
				player->AddImpulse(vec);
			}
		}
	}
}

/****************更新処理****************/

//徘徊ステート更新処理
void EnemySlime::UpdateWanderState(float elapsedTime)
{
	//目標地点へ移動
	//MoveToTarget(elapsedTime, 0.5f);
	TurnToTarget(elapsedTime, 0.5f);

	//プレイヤー索敵
	if (SearchPlayer())
	{
		//見つかったら追跡ステートへ遷移
		TransitionState(State::Pursuit);
	}
}

//待機ステート更新処理
void EnemySlime::UpdateIdleState(float elapsedTime)
{
	//プレイヤー索敵
	if (SearchPlayer())
	{
		//見つかったら追跡ステートへ遷移
		TransitionState(State::Pursuit);
	}
}

//追跡ステート更新処理
void EnemySlime::UpdatePursuitState(float elapsedTime)
{
	//目標地点へ移動
	//MoveToTarget(elapsedTime,1.0f);
	//目標へ回転
	TurnToTarget(elapsedTime, 1.0f);

	//プレイヤーが攻撃範囲にいた場合は攻撃ステートへ遷移
	Player::EnemySearch es = GetNearestPlayerES();

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
void EnemySlime::UpdateAttackState(float elapsedTime)
{
	//目標へ回転
	TurnToTarget(elapsedTime, 1.0f);

	//任意のアニメーション再生区間でのみ衝突判定処理をする
	float animationTime = model->GetCurrentAnimationSeconds();
	if (animationTime >= 0.1f && animationTime <= 0.35f)
	{
		//目玉ノードとプレイヤーの衝突処理
		CollisionNodeVsPlayer(eyeBallNodeName, 0.2f);
	}

	//攻撃アニメーションが終わったら戦闘待機ステートへ遷移
	if (!model->IsPlayAnimation())
	{
		TransitionState(State::IdleBattle);
	}
}

//戦闘待機ステート更新処理
void EnemySlime::UpdateIdleBattleState(float elapsedTime)
{
	//プレイヤーが攻撃範囲にいた場合は攻撃ステートへ遷移
	Player::EnemySearch es = GetNearestPlayerES();

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
	
	TurnToTarget(elapsedTime, 0.0f);
}

//ダメージステート更新処理
void EnemySlime::UpdateHitDamageState(float elapsedTime)
{
	if (model->IsPlayAnimation()) return;

	//プレイヤーが攻撃範囲にいた場合は攻撃ステートへ遷移
	Player::EnemySearch es = GetNearestPlayerES();

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
void EnemySlime::TransitionState(State nowState)
{
	state = nowState; //ステート設定

	//switch (nowState)
	//{
	//case State::Idle:
	//	break;
	//case State::Wander:
	//	//目標地点設定
	//	SetRandomTargetPosition();
	//	break;
	//case State::Pursuit:
	//	break;
	//case State::Attack:
	//	break;
	//case State::IdleBattle:
	//	break;
	//case State::HitDamage:
	//	break;
	//}

	TransitionPlayAnimation(nowState);
}

//遷移時アニメーション再生
void EnemySlime::TransitionPlayAnimation(State nowState)
{
	//アニメーション設定
	Animation anime = Animation::IdleNormal;
	bool animeLoop = true;

	switch (nowState)
	{
	case State::Idle:
		anime = Animation::IdleNormal;
		break;
	case State::Wander:
		anime = Animation::WalkFWD;
		break;
	case State::Pursuit:
		anime = Animation::RunFWD;
		break;
	case State::Attack:
		anime = Animation::Attack1;
		animeLoop = false;
		break;
	case State::IdleBattle:
		anime = Animation::IdleBattle;
		break;
	case State::HitDamage:
		anime = Animation::GetHit;
		animeLoop = false;
		break;
	}

	//アニメーション再生
	model->PlayAnimation(static_cast<int>(anime), animeLoop);
}

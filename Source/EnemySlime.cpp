#include "EnemySlime.h"
#include "Graphics/Graphics.h"
#include "Player1P.h"
#include "Collision.h"

//コンストラクタ
EnemySlime::EnemySlime()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	model = std::make_unique<Model>(device, "Data/Model/RPG_Slime/SlimePBR.fbx", 0.01f);

	//radius = 0.5f;
	height = 1.0f;
	//待機ステートへ遷移
	TransitionWanderState();
}

//デストラクタ
EnemySlime::~EnemySlime()
{
}

//更新処理
void EnemySlime::Update(float elapsedTime) 
{
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

//死亡した時に呼ばれる
void EnemySlime::OnDead()
{
	//自身を破棄
	Destroy();
}

//指定範囲のランダム値を計算する
float RandomRange(float min, float max)
{
	return min + (max - min) * (rand() / static_cast<float>(RAND_MAX));
}

//ターゲット位置をランダム設定
void EnemySlime::SetRandomTargetPosition()
{
	float theta = RandomRange(0, DirectX::XM_2PI);
	float range = RandomRange(0, territoryRange);

	targetPosition.x = territoryOrigin.x + range * sinf(theta);
	targetPosition.y = territoryOrigin.y;
	targetPosition.z = territoryOrigin.z + range * cosf(theta);
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
	//Move(vx, vz, moveSpeed * speedRate);
	Turn(elapsedTime, vx, vz, turnSpeed * speedRate);
}

//プレイヤー索敵
bool EnemySlime::SearchPlayer()
{
	//プレイヤーとの高低差を考慮して3Dでの距離判定をする
	const DirectX::XMFLOAT3& playerPosition = Player1P::Instance().GetPosition();
	float vx = playerPosition.x - position.x;
	float vy = playerPosition.y - position.y;
	float vz = playerPosition.z - position.z;
	float dist = sqrtf(vx * vx + vy * vy + vz * vz);

	if (dist < searchRange)
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
	if (node != nullptr)
	{
		//ノード位置取得
		XMFLOAT3 nodePosition;
		nodePosition.x = node->worldTransform._41;
		nodePosition.y = node->worldTransform._42;
		nodePosition.z = node->worldTransform._43;

		//衝突処理
		Player& player = Player1P::Instance();
		XMFLOAT3 outPosition;
		if (Collision::IntersectSphereVsCylinder(
			nodePosition, nodeRadius,
			player.GetPosition(), player.GetRadius(), player.GetHeight(),
			outPosition
		))
		{
			//ダメージを与える
			if (player.ApplyDamage(1, 0.5f))
			{
				if (player.ApplyDamage(1, 0.5f))
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
					player.AddImpulse(vec);
				}
			}
		}
	}
}

//徘徊ステートへ遷移
void EnemySlime::TransitionWanderState()
{
	state = State::Wander;

	//目標地点設定
	SetRandomTargetPosition();

	//歩きアニメーション再生
	model->PlayAnimation(Anim_WalkFWD, true);
}

//徘徊ステート更新処理
void EnemySlime::UpdateWanderState(float elapsedTime)
{
	//目標地点へ移動
	MoveToTarget(elapsedTime, 0.5f);

	//プレイヤー索敵
	if (SearchPlayer())
	{
		//見つかったら追跡ステートへ遷移
		TransitionPursuitState();
	}
}

//待機ステートへ遷移
void EnemySlime::TransitionIdleState()
{
	state = State::Idle;

	//待機アニメーション再生
	model->PlayAnimation(Anim_IdleNormal, true);
}

//待機ステート更新処理
void EnemySlime::UpdateIdleState(float elapsedTime)
{
	//プレイヤー索敵
	if (SearchPlayer())
	{
		//見つかったら追跡ステートへ遷移
		TransitionPursuitState();
	}
}

//追跡ステートへ遷移
void EnemySlime::TransitionPursuitState()
{
	state = State::Pursuit;

	//歩きアニメーション再生
	model->PlayAnimation(Anim_RunFWD, true);
}

//追跡ステート更新処理
void EnemySlime::UpdatePursuitState(float elapsedTime)
{
	//目標地点をプレイヤー位置に設定
	targetPosition = Player1P::Instance().GetPosition();

	//目標地点へ移動
	MoveToTarget(elapsedTime,1.0f);

	//プレイヤーの近づくと攻撃ステートへ遷移
	float vx = targetPosition.x - position.x;
	float vy = targetPosition.y - position.y;
	float vz = targetPosition.z - position.z;
	float dist = sqrtf(vx * vx + vy * vy + vz * vz);
	if (dist < attackRange)
	{
		//攻撃ステートへ遷移
		TransitionAttackState();
	}
	else if (!SearchPlayer())
	{
		TransitionWanderState();
	}
}

//攻撃ステートへ遷移
void EnemySlime::TransitionAttackState()
{
	state = State::Attack;

	//攻撃アニメーション再生
	model->PlayAnimation(Anim_Attack1, false);
}

//攻撃ステート更新処理
void EnemySlime::UpdateAttackState(float elapsedTime)
{
	//任意のアニメーション再生区間でのみ衝突判定処理をする
	float animationTime = model->GetCurrentAnimationSeconds();
	if (animationTime >= 0.1f && animationTime <= 0.35f)
	{
		//目玉ノードとプレイヤーの衝突処理
		CollisionNodeVsPlayer("EyeBall", 0.2f);
	}

	//攻撃アニメーションが終わったら戦闘待機ステートへ遷移
	if (!model->IsPlayAnimation())
	{
		TransitionIdleBattleState();
	}
}

//戦闘待機ステートへ遷移
void EnemySlime::TransitionIdleBattleState()
{
	state = State::IdleBattle;

	//戦闘待機アニメーション再生
	model->PlayAnimation(Anim_IdleBattle, true);
}

//戦闘待機ステート更新処理
void EnemySlime::UpdateIdleBattleState(float elapsedTime)
{
	//目標地点をプレイヤー位置に設定
	targetPosition = Player1P::Instance().GetPosition();

	//プレイヤーが攻撃範囲にいた場合は攻撃ステートへ遷移
	float vx = targetPosition.x - position.x;
	float vy = targetPosition.y - position.y;
	float vz = targetPosition.z - position.z;
	float dist = sqrtf(vx * vx + vy * vy + vz * vz);
	if (dist < attackRange)
	{
		//攻撃ステートへ遷移
		TransitionAttackState();
	}
	else
	{
		//徘徊ステートへ遷移
		TransitionWanderState();
	}
	
	MoveToTarget(elapsedTime, 0.0f);
}

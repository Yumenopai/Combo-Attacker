#include "EnemyBlue.h"
#include "Graphics/Graphics.h"
#include "Player1P.h"
#include "Collision.h"

//コンストラクタ
EnemyBlue::EnemyBlue()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	model = std::make_unique<Model>(device, "Data/Model/Enemy/Blue.fbx", 0.001f);

	height = 1.0f;
	//待機ステートへ遷移
	TransitionWanderState();
}

//デストラクタ
EnemyBlue::~EnemyBlue()
{
}

//更新処理
void EnemyBlue::Update(float elapsedTime)
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

//死亡した時に呼ばれる
void EnemyBlue::OnDead()
{
	//自身を破棄
	Destroy();
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

//徘徊ステートへ遷移
void EnemyBlue::TransitionWanderState()
{
	state = State::Wander;

	//目標地点設定
	SetRandomTargetPosition();

	//歩きアニメーション再生
	//model->PlayAnimation(Anim_WalkFWD, true);
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
		TransitionPursuitState();
	}
}

//待機ステートへ遷移
void EnemyBlue::TransitionIdleState()
{
	state = State::Idle;

	//待機アニメーション再生
	//model->PlayAnimation(Anim_IdleNormal, true);
}

//待機ステート更新処理
void EnemyBlue::UpdateIdleState(float elapsedTime)
{
	//プレイヤー索敵
	if (SearchPlayer())
	{
		//見つかったら追跡ステートへ遷移
		TransitionPursuitState();
	}
}

//追跡ステートへ遷移
void EnemyBlue::TransitionPursuitState()
{
	state = State::Pursuit;

	//歩きアニメーション再生
	//model->PlayAnimation(Anim_RunFWD, true);
}

//追跡ステート更新処理
void EnemyBlue::UpdatePursuitState(float elapsedTime)
{
	//目標地点をプレイヤー位置に設定
	targetPosition = Player1P::Instance().GetPosition();

	//目標地点へ移動
	MoveToTarget(elapsedTime, 1.0f);

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
void EnemyBlue::TransitionAttackState()
{
	state = State::Attack;

	//攻撃アニメーション再生
	//model->PlayAnimation(Anim_Attack1, false);
}

//攻撃ステート更新処理
void EnemyBlue::UpdateAttackState(float elapsedTime)
{
	//任意のアニメーション再生区間でのみ衝突判定処理をする
	float animationTime = model->GetCurrentAnimationSeconds();
	if (animationTime >= 0.1f && animationTime <= 0.35f)
	{
	}

	//攻撃アニメーションが終わったら戦闘待機ステートへ遷移
	if (!model->IsPlayAnimation())
	{
		TransitionIdleBattleState();
	}
}

//戦闘待機ステートへ遷移
void EnemyBlue::TransitionIdleBattleState()
{
	state = State::IdleBattle;

	//戦闘待機アニメーション再生
	//model->PlayAnimation(Anim_IdleBattle, true);
}

//戦闘待機ステート更新処理
void EnemyBlue::UpdateIdleBattleState(float elapsedTime)
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

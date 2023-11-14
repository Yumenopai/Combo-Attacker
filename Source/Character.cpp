#include "Character.h"
#include "Player.h"
#include "Stage.h"
#include "Input/Input.h"

//行列更新処理
void Character::UpdateTransform()
{
	//スケール行列作成
	XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);
	//回転行列作成
	XMMATRIX R = XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z);
	//位置行列作成
	XMMATRIX T = DirectX::XMMatrixTranslation(position.x, position.y, position.z);
	//3つの行列組み合わせ、ワールド行列作成
	XMMATRIX W = S * R * T;
	//計算したワールド行列取り出し
	XMStoreFloat4x4(&transform, W);
}

bool Character::ApplyDamage(int damage, float invincibleTime)
{
	if (damage == 0)		return false;
	if (invincibleTime <= 0) return false;

	//ダメージ処理
	invincibleTimer = invincibleTime;
	health -= damage;

	//死亡通知
	if (health <= 0)
	{
		OnDead();
	}
	//ダメージ通知
	else
	{
		OnDamaged();
	}

	//変更時のみ
	return true;
}

void Character::AddImpulse(const DirectX::XMFLOAT3& impulse)
{
	//速力に力を加える
	velocity.x += impulse.x;
	velocity.y += impulse.y;
	velocity.z += impulse.z;
}

//移動処理
void Character::Move(float vx, float vz, float speed)
{
	//移動方向ベクトルを設定
	moveVecX = vx;
	moveVecZ = vz;

	//最大速度設定
	maxMoveSpeed = speed;
}

//旋回処理
void Character::Turn(float elapsedTime, float vx, float vz, float speed)
{
	speed *= elapsedTime;

	//進行ベクトルがゼロベクトルの場合は処理する必要なし
	float vecLength = vx * vx + vz * vz;
	if (vecLength < 0.00001f) return;

	//進行ベクトルを単位ベクトル化
	vecLength = sqrtf(vecLength);
	vx /= vecLength;
	vz /= vecLength;

	//自身の回転値から前方向を求める
	float frontX = sinf(angle.y);
	float frontZ = cosf(angle.y);

	//回転角を求めるため、2つの単位ベクトルの内積を計算する
	float dot = frontX * vx + frontZ * vz;

	//内積値は-1.0～1.0で表現されており、2つの単位ベクトルの角度が
	//小さいほど1.0に近づくという性質を利用して回転速度を調整する
	float rot = 1.0f - dot;

	//左右判定を行うために2つの単位ベクトルの外積を計算する
	float cross = frontZ * vx - frontX * vz;

	//2Dの外積値が正の場合か負の場合によって左右判定が行える
	//左右判定を行うことによって左右回転を選択する
	if (rot > speed) rot = speed;

	if (cross < 0.0f)
	{
		angle.y -= rot;
	}
	else
	{
		angle.y += rot;
	}
}


//ジャンプ処理
void Character::Jump(float speed)
{
	//上方向の力を設定
	velocity.y = speed;
}

//速力処理
void Character::UpdateVelocity(float elapsedTime)
{
	//経過フレーム
	float elapsedFrame = 60.0f * elapsedTime;

	//垂直速力更新
	UpdateVerticalVelocity(elapsedFrame);
	UpdateVerticalMove(elapsedTime);
	//水平移動更新
	UpdateHorizontalVelocity(elapsedFrame);
	UpdateHorizontalMove(elapsedTime);
}

void Character::UpdateInvincibleTimer(float elapsedTime)
{
	if (invincibleTimer > 0.0f)
	{
		invincibleTimer -= elapsedTime;
	}
}

//垂直速力更新
void Character::UpdateVerticalVelocity(float elapsedFrame)
{
	//重力処理
	velocity.y += gravity * elapsedFrame;
}

float Lerp(float a, float b, float t)
{
	return a * (1.0f - t) + (b * t);
}
//垂直移動更新
void Character::UpdateVerticalMove(float elapsedTime)
{
	//垂直方向の移動量
	float my = velocity.y * elapsedTime;
	slopeRate = 0.0f;

	//キャラクターのY軸方向となる法線ベクトル
	XMFLOAT3 normal = { 0,1,0 };
	
	//落下中
	if (my < 0.0f)
	{
		//レイの開始位置は足元より少し上
		XMFLOAT3 start = { position.x, position.y + stepOffset, position.z };
		//レイの終点位置は移動後の位置
		XMFLOAT3 end = { position.x, position.y + my, position.z };

		//レイキャストによる地面判定
		HitResult hit;
		if (Stage::Instance().RayCast(start, end, hit))
		{
			//法線ベクトル取得
			normal = hit.normal;

			//地面に接地している
			position = hit.position;

			//回転
			angle.y += hit.rotation.y;

			//傾斜率の計算
			{
				float nx = hit.normal.x;
				float ny = hit.normal.y;
				float nz = hit.normal.z;
				float lengthXZ = sqrtf(nx * nx + nz * nz);
				slopeRate = 1.0f - (ny / (lengthXZ + ny));
			}

			//着地した
			if (!isGround)
			{
				OnLanding(elapsedTime);
			}
			isGround = true;
			velocity.y = 0.0f;
		}
		else
		{
			//空中に浮いている
			position.y += my;
			isGround = false;
		}
	}
	//上昇中
	else if (my > 0.0f)
	{
		position.y += my;
		isGround = false;
	}

	//地面の向きに沿うようにXZ軸回転
	{
		//Y軸が法線ベクトル方向に向くオイラー角回転を算出
		//線形補完で滑らかに回転する
		//float ax = atan2f(normal.z, normal.y);
		//angle.x = Lerp(angle.x, ax, 0.2f);

		//float az = -atan2f(normal.x, normal.y);
		//angle.z = Lerp(angle.z, az, 0.2f);
	}
}

//水平速力更新
void Character::UpdateHorizontalVelocity(float elapsedFrame)
{
	//XZ平面の速力を減速
	float length = sqrtf(velocity.x * velocity.x + velocity.z * velocity.z);
	if (length > 0.0f)
	{
		//摩擦力
		float friction = this->friction * elapsedFrame;

		//摩擦による横方向の原則処理
		//横方向速力の大きさの方が摩擦力より大きい時
		if (length > friction)
		{
			//単位ベクトル化
			float vx = velocity.x / length;
			float vz = velocity.z / length;

			//velocity -= |velocity| * friction;
			velocity.x -= vx * friction;
			velocity.z -= vz * friction;
		}
		//横方向の速力が摩擦力以下になったので速力を無効化
		else
		{
			velocity.x = 0.0f;
			velocity.z = 0.0f;
		}
	}

	//XZ平面の速力を加速
	if (length <= maxMoveSpeed)
	{
		//移動ベクトルがゼロでないなら加速
		float moveVecLength = moveVecX * moveVecX + moveVecZ * moveVecZ;
		if (moveVecLength > 0.0f)
		{
			//加速力
			float acceleration = this->acceleration * elapsedFrame;
			//移動ベクトルによる加速処理
			velocity.x += moveVecX * acceleration;
			velocity.z += moveVecZ * acceleration;

			//最大速度制限
			float nowLength = sqrtf(velocity.x * velocity.x + velocity.z * velocity.z);
			if (nowLength > maxMoveSpeed)
			{
				float vx = velocity.x / nowLength;
				float vz = velocity.z / nowLength;

				velocity.x = vx * maxMoveSpeed;
				velocity.z = vz * maxMoveSpeed;
			}

			//下り坂でガタガタしないようにする
			if (isGround && slopeRate > 0.0f)
			{
				velocity.y -= length * elapsedFrame * slopeRate;
			}
		}
	}
	//移動ベクトルをリセット
	moveVecX = 0.0f;
	moveVecZ = 0.0f;
}

//水平移動更新
void Character::UpdateHorizontalMove(float elapsedTime)
{
	//水平速力量計算
	float velocityLengthXZ = sqrtf(velocity.x * velocity.x + velocity.z * velocity.z);
	if (velocityLengthXZ > 0.0f)
	{
		//水平移動地
		float mx = velocity.x * elapsedTime;
		float mz = velocity.z * elapsedTime;

		//レイの開始位置と終点位置
		XMFLOAT3 start = { position.x,		position.y + stepOffset / 2,position.z };
		XMFLOAT3 end = { position.x + mx ,position.y + stepOffset / 2,position.z + mz };

		//レイキャストによる壁判定
		HitResult hit;
		if (Stage::Instance().RayCast(start, end, hit))
		{
			//壁開始から本来進む位置までのベクトル
			XMVECTOR Start = XMLoadFloat3(&hit.position);
			XMVECTOR End = XMLoadFloat3(&end);
			XMVECTOR Vec = XMVectorSubtract(End, Start);

			//壁の法線
			XMVECTOR Normal = XMLoadFloat3(&hit.normal);

			//入射ベクトルを法線に射影
			XMVECTOR Dot = XMVector3Dot(XMVectorNegate(Vec), Normal);
			Dot = XMVectorScale(Dot, 1.1f); //壁のめり込み対策

			//補正位置の計算
			XMVECTOR CorrectPosition = XMVectorMultiplyAdd(Normal, Dot, End);
			XMFLOAT3 correctPosition;
			XMStoreFloat3(&correctPosition, CorrectPosition);

			//壁擦り補正後の方向へレイキャスト
			HitResult hit2;
			if (!Stage::Instance().RayCast(start, correctPosition, hit2))
			{
				//壁擦り方向で壁に当たらなかったら補正位置に移動
				position.x = correctPosition.x;
				position.z = correctPosition.z;
			}
			else
			{
				//壁に当たった点で停止
				position.x = hit2.position.x;
				position.z = hit2.position.z;
			}
		}
		else
		{
			//移動
			position.x += mx;
			position.z += mz;
		}
	}
}
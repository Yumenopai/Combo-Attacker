#pragma once

#include<DirectXMath.h>
using namespace DirectX;

//キャラクター
class Character
{
public:
	Character() {}
	virtual ~Character() {}

	//行列更新
	void UpdateTransform();

	//位置取得
	const XMFLOAT3& GetPosition() const { return position; }
	//位置設定
	void SetPosition(const XMFLOAT3& position) { this->position = position; }
	//回転取得
	const XMFLOAT3& GetAngle() const { return angle; }
	//回転設定
	void SetAngle(const XMFLOAT3& angle) { this->angle = angle; }
	//スケール取得
	const XMFLOAT3& GetScale() const { return scale; }
	//スケール設定
	void SetScale(const XMFLOAT3& scale) { this->scale = scale; }
	//半径取得
	float GetRadius() const { return radius; }

	//地面に接触しているか
	bool IsGround() const { return isGround; }
	//高さ取得
	float GetHeight() const { return height; }
	//ダメージを与える
	bool ApplyDamage(int damage, float invincibleTime);
	//衝撃を与える
	void AddImpulse(const XMFLOAT3& impulse);

	//健康状態を取得
	int GetHealth() const { return health; }
	//最大健康状態を取得
	int GetMaxHealth() const { return maxHealth; }

protected:
	//移動処理
	void Move(float vx, float vz, float speed);
	//旋回処理
	void Turn(float elapsedTime, float vx, float vz, float speed);
	//ジャンプ処理
	void Jump(float speed);
	//速力処理更新
	void UpdateVelocity(float elapsedTime);
	//無敵時間更新
	void UpdateInvincibleTimer(float elapsedTime);

	//着地した時に呼ばれる
	virtual void OnLanding(float elapsedTime) {}
	//ダメージ受けた時に呼ばれる
	virtual void OnDamaged() {}
	//死亡した時に呼ばれる
	virtual void OnDead() {}

	//垂直速力更新
	virtual void UpdateVerticalVelocity(float elapsedFrame);
private:
	//垂直移動更新
	void UpdateVerticalMove(float elapsedTime);
	//水平速力更新
	void UpdateHorizontalVelocity(float elapsedFrame);
	//水平移動更新
	void UpdateHorizontalMove(float elapsedTime);

protected:
	XMFLOAT3	position = { 0,0,0 };
	XMFLOAT3	angle = { 0,0,0 };
	XMFLOAT3	scale = { 1,1,1 };
	XMFLOAT4X4 transform = {
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	};

	float radius = 0.5f;

	float	gravity = -1.0f;
	XMFLOAT3 velocity = { 0,0,0 };
	bool	isGround = false;
	float	height = 2.0f;
	int		health = 40;
	int		maxHealth = 40;
	float	invincibleTimer = 1.0f;

	float	friction = 0.5f;

	float acceleration = 1.0f;
	float maxMoveSpeed = 5.0f;
	float moveVecX = 0.0f;
	float moveVecZ = 0.0f;
	float stepOffset = 1.0f;
	float slopeRate = 1.0f;
};
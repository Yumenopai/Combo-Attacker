#pragma once

#include<DirectXMath.h>
using namespace DirectX;

//キャラクター
class Character
{
protected:
	// 位置
	XMFLOAT3 position = { 0,0,0 };
	// 角度
	XMFLOAT3 angle = { 0,0,0 };
	// スケール
	XMFLOAT3 scale = { 1,1,1 };
	// トランスフォーム
	XMFLOAT4X4 transform = {
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		0,0,0,1
	};
	// ダメージ半径
	float radius = 0.5f;

	// 重力
	const float gravity = -1.0f;
	// 加速度
	XMFLOAT3 velocity = { 0,0,0 };
	// 地面に着地しているか
	bool isGround = false;
	// 高さ
	float height = 2.0f;
	// HP
	int health = 40;
	// 最大HP
	int maxHealth = 40;
	// ダメージ無敵時間タイマー
	float invincibleTimer = 0.0f;
	// 摩擦力
	float	friction = 0.5f;

	// 加速力
	float acceleration = 1.0f;
	// 最大移動スピード
	float maxMoveSpeed = 5.0f;
	// 移動ベクトル_X
	float moveVecX = 0.0f;
	// 移動ベクトル_Y
	float moveVecZ = 0.0f;
	// レイキャスト用足元オフセット
	float stepOffset = 1.0f;
	// 傾斜率
	float slopeRate = 1.0f;

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
	bool ApplyDamage(int damage, float invincibleTime = 0);
	//衝撃を与える
	void AddImpulse(const XMFLOAT3& impulse);

	//健康%状態を取得
	int GetHealthRate() const { return 100 * health / maxHealth; }
	//健康状態設定
	void SetHealth(int h) { health = h; }
	//健康状態を取得
	int GetHealth() const { return health; }
	//最大健康状態を取得
	int GetMaxHealth() const { return maxHealth; }
	// 健康状態を設定
	void AddHealth(const int h) { health += h; }

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
};
#pragma once

#include <DirectXMath.h>

class CameraController
{
public:
	CameraController() {}
	~CameraController() {}

	//更新処理
	void Update(float elapsedTime);

	//ターゲット位置設定
	void SetTarget(const DirectX::XMFLOAT3& target) { this->target = target; }

private:
	DirectX::XMFLOAT3 target = { 0, 0, 0 };
	DirectX::XMFLOAT3 angle = { 0.2f, 3.5f, 0.0f };
	float rollSpeed = DirectX::XMConvertToRadians(90);
	float range = 11.0f;
	float maxAngleX = DirectX::XMConvertToRadians(45);
	float minAngleX = DirectX::XMConvertToRadians(0);
};
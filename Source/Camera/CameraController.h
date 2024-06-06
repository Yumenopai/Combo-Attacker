#pragma once

#include <DirectXMath.h>

class CameraController
{
public:
	CameraController() {}
	~CameraController() {}

	//�X�V����
	void Update(float elapsedTime);

	//�^�[�Q�b�g�ʒu�ݒ�
	void SetTarget(const DirectX::XMFLOAT3& target) { this->target = target; }

private:
	DirectX::XMFLOAT3 target = { 0, 0, 0 };
	DirectX::XMFLOAT3 angle = { 0.2f, 3.5f, 0.0f };
	float rollSpeed = DirectX::XMConvertToRadians(90);
	float range = 11.0f;
	float maxAngleX = DirectX::XMConvertToRadians(45);
	float minAngleX = DirectX::XMConvertToRadians(0);
};
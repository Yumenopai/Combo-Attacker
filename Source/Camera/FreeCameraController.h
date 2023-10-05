#pragma once
#include "Camera.h"

class FreeCameraController
{
public:
	//�J��������R���g���[���[�փp�����[�^�𓯊�
	void SyncCameraToController(const Camera& camera);

	//�R���g���[���[����J�����փp�����[�^�𓯊�
	void SyncControllerToCamera(Camera& camera);

	//�X�V����
	void Update();

private:
	XMFLOAT3	eye;
	XMFLOAT3	focus;
	XMFLOAT3	up;
	XMFLOAT3	right;
	float		distance;

	float		angleX;
	float		angleY;
};
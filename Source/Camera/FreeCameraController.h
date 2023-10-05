#pragma once
#include "Camera.h"

class FreeCameraController
{
public:
	//カメラからコントローラーへパラメータを同期
	void SyncCameraToController(const Camera& camera);

	//コントローラーからカメラへパラメータを同期
	void SyncControllerToCamera(Camera& camera);

	//更新処理
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
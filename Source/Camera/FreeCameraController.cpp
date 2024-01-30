#include <imgui.h>
#include "FreeCameraController.h"

//カメラからコントローラーへパラメータを同期
void FreeCameraController::SyncCameraToController(const Camera& camera)
{
	eye = camera.GetEye();
	focus = camera.GetFocus();
	up = camera.GetUp();
	right = camera.GetRight();

	//視点から注視点までの距離を算出
	XMVECTOR Eye = XMLoadFloat3(&eye);
	XMVECTOR Focus = XMLoadFloat3(&focus);
	XMVECTOR Vec = XMVectorSubtract(Focus, Eye);
	XMVECTOR Distance = XMVector3Length(Vec);
	XMStoreFloat(&distance, Distance);

	//回転角度を算出
	const XMFLOAT3& front = camera.GetFront();
	angleX = ::asinf(-front.y);
	if (up.y < 0)
	{
		if (front.y > 0)
		{
			angleX = -XM_PI - angleX;
		}
		else
		{
			angleX = XM_PI - angleX;
		}
		angleY = atan2f(front.x, front.z);
	}
	else
	{
		angleY = ::atan2f(-front.x, -front.z);
	}
}

//コントローラーからカメラへパラメータを同期
void FreeCameraController::SyncControllerToCamera(Camera& camera)
{
	camera.SetLookAt(eye, focus, up);
}

//更新処理
void FreeCameraController::Update()
{
	//デバッグウィンドウ操作中は処理しない
	//if (ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
	//{
	//	return;
	//}

	//ImGuiのマウス入力値を使ってカメラ操作する
	ImGuiIO io = ImGui::GetIO();

	//マウスカーソルの移動量を求める
	float moveX = io.MouseDelta.x * 0.02f;
	float moveY = io.MouseDelta.y * 0.02f;

	//マウス感度倍率
	float MouseRate = 0.1f;

	//マウス左ボタン押下中
	//if (io.MouseDown[ImGuiMouseButton_Left])
	{
		//Y軸回転
		angleY += moveX * MouseRate;
		if (angleY > XM_PI)
		{
			angleY -= XM_2PI;
		}
		else if (angleY < -XM_PI)
		{
			angleY += XM_2PI;
		}
		//X軸回転
		angleX += moveY * MouseRate;
		if (angleX > XM_PI)
		{
			angleX -= XM_2PI;
		}
		else if (angleX < -XM_PI)
		{
			angleX += XM_2PI;
		}
	}
	////マウス中ボタン押下中
	//else if (io.MouseDown[ImGuiMouseButton_Middle])
	//{
	//	//平行移動
	//	float s = distance * 0.035f;
	//	float x = moveX * s;
	//	float y = moveY * s;

	//	focus.x -= right.x * x;
	//	focus.y -= right.y * x;
	//	focus.z -= right.z * x;

	//	focus.x += up.x * y;
	//	focus.y += up.y * y;
	//	focus.z += up.z * y;
	//}
	////マウス右ボタン押下中
	//else if (io.MouseDown[ImGuiMouseButton_Right])
	//{
	//	//ズーム
	//	distance += (-moveY - moveX) * distance * 0.1f;
	//}
	//マウスホイール
	if (io.MouseWheel != 0)
	{
		//ズーム
		distance -= io.MouseWheel * distance * 0.1f;
	}

	float sx = ::sinf(angleX);
	float cx = ::cosf(angleX);
	float sy = ::sinf(angleY);
	float cy = ::cosf(angleY);

	//カメラの方向を算出
	XMVECTOR Front = XMVectorSet(-cx * sy, -sx, -cx * cy, 0.0f);
	XMVECTOR Right = XMVectorSet(cy, 0, -sy, 0.0f);
	XMVECTOR Up = XMVector3Cross(Right, Front);
	//カメラの視点＆注視点を算出
	XMVECTOR Focus = XMLoadFloat3(&focus);
	XMVECTOR Distance = XMVectorSet(distance, distance, distance, 0.0f);
	XMVECTOR Eye = XMVectorSubtract(Focus, XMVectorMultiply(Front, Distance));
	//ビュー行列からワールド行列を算出
	XMMATRIX View = XMMatrixLookAtLH(Eye, Focus, Up);
	XMMATRIX World = XMMatrixTranspose(View);
	//ワールド行列から方向を算出
	Right = XMVector3TransformNormal(XMVectorSet(1, 0, 0, 0), World);
	Up = XMVector3TransformNormal(XMVectorSet(0, 1, 0, 0), World);
	//結果を格納
	XMStoreFloat3(&eye, Eye);
	XMStoreFloat3(&up, Up);
	XMStoreFloat3(&right, Right);
}

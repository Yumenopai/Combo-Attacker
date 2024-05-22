#pragma once

#include <DirectXMath.h>
using namespace DirectX;

//カメラ
class Camera
{
	//シングルトン化のためprivate
private:
	Camera() {}
	~Camera() {}

public:
	static Camera& Instance()
	{
		//唯一のインスタンス取得
		static Camera camera;
		return camera;
	}

	//指定方向を向く
	void SetLookAt(const DirectX::XMFLOAT3& eye, const DirectX::XMFLOAT3& focus, const DirectX::XMFLOAT3& up);

	//パースペクティブ設定
	void SetPerspectiveFov(float fovY, float aspect, float nearZ, float farZ);

	//ビュー行列取得
	const XMFLOAT4X4& GetView() const { return view; }

	//プロジェクション行列取得
	const XMFLOAT4X4& GetProjection() const { return projection; }

	//視点取得
	const XMFLOAT3& GetEye() const { return eye; }

	//注視点取得
	const XMFLOAT3& GetFocus() const { return focus; }

	//上方向取得
	const XMFLOAT3& GetUp() const { return up; }

	//前方向取得
	const XMFLOAT3& GetFront() const { return front; }

	//右方向取得
	const XMFLOAT3& GetRight() const { return right; }

	//ImGui
	void DebugImGui();

private:
	XMFLOAT4X4 view = {};
	XMFLOAT4X4 projection = {};

	XMFLOAT3 eye = {};
	XMFLOAT3 focus = {};

	XMFLOAT3 up = {};
	XMFLOAT3 front = {};
	XMFLOAT3 right = {};
};
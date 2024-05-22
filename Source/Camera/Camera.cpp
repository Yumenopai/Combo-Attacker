#include "Camera.h"
#include "imgui.h"

//指定方向を向く
void Camera::SetLookAt(const DirectX::XMFLOAT3& eye, const DirectX::XMFLOAT3& focus, const DirectX::XMFLOAT3& up)
{
	//視点、注視点、上方向からビュー行列を作成
	XMVECTOR Eye = XMLoadFloat3(&eye);
	XMVECTOR Focus = XMLoadFloat3(&focus);
	XMVECTOR Up = XMLoadFloat3(&up);
	XMMATRIX View = XMMatrixLookAtLH(Eye, Focus, Up);
	XMStoreFloat4x4(&view, View);

	//ビュー行列を逆行列化し、ワールド行列に戻す
	XMMATRIX World = XMMatrixInverse(nullptr, View);
	XMFLOAT4X4 world;
	XMStoreFloat4x4(&world, World);

	//カメラの方向を取り出す
	this->right.x = world._11;
	this->right.y = world._12;
	this->right.z = world._13;

	this->up.x = world._21;
	this->up.y = world._22;
	this->up.z = world._23;

	this->front.x = world._31;
	this->front.y = world._32;
	this->front.z = world._33;

	//視点、注視点を保存
	this->eye = eye;
	this->focus = focus;
}

//パースペクティブ設定
void Camera::SetPerspectiveFov(float fovY, float aspect, float nearZ, float farZ)
{
	//画角、画面比率、クリップ距離からプロジェクション行列を作成
	XMMATRIX Projection = XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ);
	XMStoreFloat4x4(&projection, Projection);
}

#pragma region DEBUG_DRAW
//デバッグメニュー描画
void Camera::DebugImGui()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Camera", nullptr, ImGuiWindowFlags_None))
	{
		if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::DragFloat3("eye", &this->eye.x, 0.1f);
			ImGui::DragFloat3("focus", &this->focus.x, 0.1f);
			ImGui::DragFloat3("front", &this->front.x, 0.1f);
			ImGui::DragFloat3("up", &this->up.x, 0.1f);
			ImGui::DragFloat3("right", &this->right.x, 0.1f);
		}

		ImGui::End();
	}
}
#pragma endregion
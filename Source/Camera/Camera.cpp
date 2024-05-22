#include "Camera.h"
#include "imgui.h"

//�w�����������
void Camera::SetLookAt(const DirectX::XMFLOAT3& eye, const DirectX::XMFLOAT3& focus, const DirectX::XMFLOAT3& up)
{
	//���_�A�����_�A���������r���[�s����쐬
	XMVECTOR Eye = XMLoadFloat3(&eye);
	XMVECTOR Focus = XMLoadFloat3(&focus);
	XMVECTOR Up = XMLoadFloat3(&up);
	XMMATRIX View = XMMatrixLookAtLH(Eye, Focus, Up);
	XMStoreFloat4x4(&view, View);

	//�r���[�s����t�s�񉻂��A���[���h�s��ɖ߂�
	XMMATRIX World = XMMatrixInverse(nullptr, View);
	XMFLOAT4X4 world;
	XMStoreFloat4x4(&world, World);

	//�J�����̕��������o��
	this->right.x = world._11;
	this->right.y = world._12;
	this->right.z = world._13;

	this->up.x = world._21;
	this->up.y = world._22;
	this->up.z = world._23;

	this->front.x = world._31;
	this->front.y = world._32;
	this->front.z = world._33;

	//���_�A�����_��ۑ�
	this->eye = eye;
	this->focus = focus;
}

//�p�[�X�y�N�e�B�u�ݒ�
void Camera::SetPerspectiveFov(float fovY, float aspect, float nearZ, float farZ)
{
	//��p�A��ʔ䗦�A�N���b�v��������v���W�F�N�V�����s����쐬
	XMMATRIX Projection = XMMatrixPerspectiveFovLH(fovY, aspect, nearZ, farZ);
	XMStoreFloat4x4(&projection, Projection);
}

#pragma region DEBUG_DRAW
//�f�o�b�O���j���[�`��
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
#include <imgui.h>
#include "FreeCameraController.h"

//�J��������R���g���[���[�փp�����[�^�𓯊�
void FreeCameraController::SyncCameraToController(const Camera& camera)
{
	eye = camera.GetEye();
	focus = camera.GetFocus();
	up = camera.GetUp();
	right = camera.GetRight();

	//���_���璍���_�܂ł̋������Z�o
	XMVECTOR Eye = XMLoadFloat3(&eye);
	XMVECTOR Focus = XMLoadFloat3(&focus);
	XMVECTOR Vec = XMVectorSubtract(Focus, Eye);
	XMVECTOR Distance = XMVector3Length(Vec);
	XMStoreFloat(&distance, Distance);

	//��]�p�x���Z�o
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

//�R���g���[���[����J�����փp�����[�^�𓯊�
void FreeCameraController::SyncControllerToCamera(Camera& camera)
{
	camera.SetLookAt(eye, focus, up);
}

//�X�V����
void FreeCameraController::Update()
{
	//�f�o�b�O�E�B���h�E���쒆�͏������Ȃ�
	//if (ImGui::IsWindowFocused(ImGuiFocusedFlags_AnyWindow))
	//{
	//	return;
	//}

	//ImGui�̃}�E�X���͒l���g���ăJ�������삷��
	ImGuiIO io = ImGui::GetIO();

	//�}�E�X�J�[�\���̈ړ��ʂ����߂�
	float moveX = io.MouseDelta.x * 0.02f;
	float moveY = io.MouseDelta.y * 0.02f;

	//�}�E�X���x�{��
	float MouseRate = 0.1f;

	//�}�E�X���{�^��������
	//if (io.MouseDown[ImGuiMouseButton_Left])
	{
		//Y����]
		angleY += moveX * MouseRate;
		if (angleY > XM_PI)
		{
			angleY -= XM_2PI;
		}
		else if (angleY < -XM_PI)
		{
			angleY += XM_2PI;
		}
		//X����]
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
	////�}�E�X���{�^��������
	//else if (io.MouseDown[ImGuiMouseButton_Middle])
	//{
	//	//���s�ړ�
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
	////�}�E�X�E�{�^��������
	//else if (io.MouseDown[ImGuiMouseButton_Right])
	//{
	//	//�Y�[��
	//	distance += (-moveY - moveX) * distance * 0.1f;
	//}
	//�}�E�X�z�C�[��
	if (io.MouseWheel != 0)
	{
		//�Y�[��
		distance -= io.MouseWheel * distance * 0.1f;
	}

	float sx = ::sinf(angleX);
	float cx = ::cosf(angleX);
	float sy = ::sinf(angleY);
	float cy = ::cosf(angleY);

	//�J�����̕������Z�o
	XMVECTOR Front = XMVectorSet(-cx * sy, -sx, -cx * cy, 0.0f);
	XMVECTOR Right = XMVectorSet(cy, 0, -sy, 0.0f);
	XMVECTOR Up = XMVector3Cross(Right, Front);
	//�J�����̎��_�������_���Z�o
	XMVECTOR Focus = XMLoadFloat3(&focus);
	XMVECTOR Distance = XMVectorSet(distance, distance, distance, 0.0f);
	XMVECTOR Eye = XMVectorSubtract(Focus, XMVectorMultiply(Front, Distance));
	//�r���[�s�񂩂烏�[���h�s����Z�o
	XMMATRIX View = XMMatrixLookAtLH(Eye, Focus, Up);
	XMMATRIX World = XMMatrixTranspose(View);
	//���[���h�s�񂩂�������Z�o
	Right = XMVector3TransformNormal(XMVectorSet(1, 0, 0, 0), World);
	Up = XMVector3TransformNormal(XMVectorSet(0, 1, 0, 0), World);
	//���ʂ��i�[
	XMStoreFloat3(&eye, Eye);
	XMStoreFloat3(&up, Up);
	XMStoreFloat3(&right, Right);
}

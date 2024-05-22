#pragma once

#include <DirectXMath.h>
using namespace DirectX;

//�J����
class Camera
{
	//�V���O���g�����̂���private
private:
	Camera() {}
	~Camera() {}

public:
	static Camera& Instance()
	{
		//�B��̃C���X�^���X�擾
		static Camera camera;
		return camera;
	}

	//�w�����������
	void SetLookAt(const DirectX::XMFLOAT3& eye, const DirectX::XMFLOAT3& focus, const DirectX::XMFLOAT3& up);

	//�p�[�X�y�N�e�B�u�ݒ�
	void SetPerspectiveFov(float fovY, float aspect, float nearZ, float farZ);

	//�r���[�s��擾
	const XMFLOAT4X4& GetView() const { return view; }

	//�v���W�F�N�V�����s��擾
	const XMFLOAT4X4& GetProjection() const { return projection; }

	//���_�擾
	const XMFLOAT3& GetEye() const { return eye; }

	//�����_�擾
	const XMFLOAT3& GetFocus() const { return focus; }

	//������擾
	const XMFLOAT3& GetUp() const { return up; }

	//�O�����擾
	const XMFLOAT3& GetFront() const { return front; }

	//�E�����擾
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
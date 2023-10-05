#include "TransformUtils.h"

//�s�񂩂烈�[�s�b�`���[���s��v�Z
bool TransformUtils::MatrixToRollPitchYaw(const XMFLOAT4X4& m, float& pitch, float& yaw, float& roll)
{
	float xRadian = asinf(-m._32);
	pitch = xRadian;
	if (xRadian < XM_PI / 2.0f)
	{
		if (xRadian > -XM_PI / 2.0f)
		{
			roll = atan2f(m._12, m._22);
			yaw = atan2f(m._31, m._33);
			return true;
		}
		else
		{
			roll = (float)-atan2f(m._13, m._11);
			yaw = 0.0f;
			return false;
		}
	}
	else
	{
		roll = (float)atan2f(m._13, m._11);
		yaw = 0.0f;
		return false;
	}
}

//�N�H�[�^�j�I�����烈�[�s�b�`���[���s��v�Z
bool TransformUtils::QuaternionToRollPitchYaw(const XMFLOAT4& q, float& pitch, float& yaw, float& roll)
{
	XMVECTOR Q = XMLoadFloat4(&q);
	XMMATRIX M = XMMatrixRotationQuaternion(Q);
	XMFLOAT4X4 m;
	XMStoreFloat4x4(&m, M);
	return MatrixToRollPitchYaw(m, pitch, yaw, roll);
}
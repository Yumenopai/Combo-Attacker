#pragma once
#include <DirectXMath.h>
using namespace DirectX;

class TransformUtils
{
public:
	//�s�񂩂烈�[�s�b�`���[���s��v�Z
	static bool MatrixToRollPitchYaw(const XMFLOAT4X4& m, float& pitch, float& yaw, float& roll);

	//�N�H�[�^�j�I�����烈�[�s�b�`���[���s��v�Z
	static bool QuaternionToRollPitchYaw(const XMFLOAT4& q, float& pitch, float& yaw, float& roll);
};
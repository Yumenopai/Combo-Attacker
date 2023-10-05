#pragma once
#include <DirectXMath.h>
using namespace DirectX;

class TransformUtils
{
public:
	//行列からヨーピッチロール行列計算
	static bool MatrixToRollPitchYaw(const XMFLOAT4X4& m, float& pitch, float& yaw, float& roll);

	//クォータニオンからヨーピッチロール行列計算
	static bool QuaternionToRollPitchYaw(const XMFLOAT4& q, float& pitch, float& yaw, float& roll);
};
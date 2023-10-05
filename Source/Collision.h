#pragma once
#include <DirectXMath.h>
#include "Graphics/Model.h"

using namespace DirectX;

struct HitResult
{
	XMFLOAT3	position = { 0,0,0 };
	XMFLOAT3	normal = { 0,0,0 };
	float		distance = 0.0f;
	int			materialIndex = -1;
	XMFLOAT3	rotation = { 0,0,0 };//��]��
};

//�R���W����
class Collision
{
public:
	//���Ƌ��̌�������
	static bool IntersectSphereVsSphere(
		const XMFLOAT3& positionA,
		float radiusA,
		const XMFLOAT3& positionB,
		float radiusB,
		XMFLOAT3& outPositionB
	);

	//�~���Ɖ~���̌�������
	static bool IntersectCylinderVsCylinder(
		const XMFLOAT3& positionA,
		float radiusA,
		float heightA,
		const XMFLOAT3& positionB,
		float radiusB,
		float heightB,
		XMFLOAT3& outPositionA,
		XMFLOAT3& outPositionB
	);

	//���Ɖ~���̌�������
	static bool IntersectSphereVsCylinder(
		const XMFLOAT3& spherePosition,
		float sphereRadius,
		const XMFLOAT3& cylinderPosition,
		float cylinderRadius,
		float cylinderHeight,
		XMFLOAT3& outCylinderPosition
	);

	//���C�ƃ��f���̌�������
	static bool IntersectRayVsModel(
		const XMFLOAT3& start,
		const XMFLOAT3& end,
		const Model* model,
		HitResult& result);
};
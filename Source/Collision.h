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
	XMFLOAT3	rotation = { 0,0,0 };//‰ñ“]—Ê
};

//ƒRƒŠƒWƒ‡ƒ“
class Collision
{
public:
	//‹…‚Æ‹…‚ÌŒğ·”»’è
	static bool IntersectSphereVsSphere(
		const XMFLOAT3& positionA,
		float radiusA,
		const XMFLOAT3& positionB,
		float radiusB,
		XMFLOAT3& outPositionB
	);

	//‰~’Œ‚Æ‰~’Œ‚ÌŒğ·”»’è
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

	//‹…‚Æ‰~’Œ‚ÌŒğ·”»’è
	static bool IntersectSphereVsCylinder(
		const XMFLOAT3& spherePosition,
		float sphereRadius,
		const XMFLOAT3& cylinderPosition,
		float cylinderRadius,
		float cylinderHeight,
		XMFLOAT3& outCylinderPosition
	);

	//ƒŒƒC‚Æƒ‚ƒfƒ‹‚ÌŒğ·”»’è
	static bool IntersectRayVsModel(
		const XMFLOAT3& start,
		const XMFLOAT3& end,
		const Model* model,
		HitResult& result);
};
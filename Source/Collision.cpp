#include "Collision.h"

//球と球の交差判定
bool Collision::IntersectSphereVsSphere(
	const XMFLOAT3& positionA,
	float radiusA,
	const XMFLOAT3& positionB,
	float radiusB,
	XMFLOAT3& outPositionB)
{
	//A→Bの単位ベクトルを算出
	XMVECTOR PositionA = XMLoadFloat3(&positionA);
	XMVECTOR PositionB = XMLoadFloat3(&positionB);
	XMVECTOR Vec = XMVectorSubtract(PositionB, PositionA);
	XMVECTOR LengthSq = XMVector3Length(Vec);
	float lengthSq;
	XMStoreFloat(&lengthSq, LengthSq);

	//距離判定
	float range = radiusA + radiusB;
	if (lengthSq > range * range)
	{
		return false;
	}

	//AがBを押し出す
	Vec = XMVector3Normalize(Vec);	//正規化
	Vec = XMVectorScale(Vec, range);
	PositionB = XMVectorAdd(PositionA, Vec);

	XMStoreFloat3(&outPositionB, PositionB);

	return true;
}

//円柱と円柱の交差判定
bool Collision::IntersectCylinderVsCylinder(
	const XMFLOAT3& positionA,
	float radiusA,
	float heightA,
	const XMFLOAT3& positionB,
	float radiusB,
	float heightB,
	XMFLOAT3& outPositionA,
	XMFLOAT3& outPositionB)
{
	//Aの足元がBの頭より上なら当たっていない
	if (positionA.y > positionB.y + heightB)
	{
		return false;
	}
	//Aの頭がBの足元より下なら当たっていない
	if (positionA.y + heightA < positionB.y)
	{
		return false;
	}
	//XZ平面での範囲チェック
	float vx = positionB.x - positionA.x;
	float vz = positionB.z - positionA.z;
	float distXZ = vx * vx + vz * vz;//2乗のまま
	float range = radiusA + radiusB;
	if (distXZ > range * range)
	{
		return false;
	}
	//AがBを押し出す
	distXZ = sqrtf(distXZ);
	vx /= distXZ;
	vz /= distXZ;
	outPositionB.x = positionA.x + vx * range;
	outPositionB.z = positionA.z + vz * range;
	outPositionB.y = positionB.y;

	outPositionA.x = positionB.x + vx * range;
	outPositionA.z = positionB.z + vz * range;
	outPositionA.y = positionA.y;
	return true;
}

//球と円柱の交差判定
bool Collision::IntersectSphereVsCylinder(
	const XMFLOAT3& _spherePosition,
	float sphereRadius,
	const XMFLOAT3& cylinderPosition,
	float cylinderRadius,
	float cylinderHeight,
	XMFLOAT3& outCylinderPosition
)
{
	XMFLOAT3 spherePosition = _spherePosition;
	spherePosition.y -= sphereRadius;

	float sphereHeight = sphereRadius * 2.0f;

	return IntersectCylinderVsCylinder(
		spherePosition, sphereRadius, sphereHeight,
		cylinderPosition, cylinderRadius, cylinderHeight,
		XMFLOAT3{},outCylinderPosition
	);
}

//レイとモデルの交差判定
bool Collision::IntersectRayVsModel(const XMFLOAT3& start, const XMFLOAT3& end, const Model* model, HitResult& result)
{
	XMVECTOR WorldStart = XMLoadFloat3(&start);
	XMVECTOR WorldEnd = XMLoadFloat3(&end);
	XMVECTOR WorldRayVec = XMVectorSubtract(WorldEnd, WorldStart);
	XMVECTOR WorldRayLength = XMVector3Length(WorldRayVec);

	//ワールド空間のレイの長さ
	XMStoreFloat(&result.distance, WorldRayLength);

	bool hit = false;
	for (const Model::Mesh& mesh : model->GetMeshes())
	{
		//メッシュノード取得
		const Model::Node& node = model->GetNodes().at(mesh.nodeIndex);

		//レイをワールド空間からローカル空間へ変換
		XMMATRIX WorldTransform = XMLoadFloat4x4(&node.worldTransform);
		XMMATRIX InverseWorldTransform = XMMatrixInverse(nullptr, WorldTransform);

		XMVECTOR S = XMVector3TransformCoord(WorldStart, InverseWorldTransform);
		XMVECTOR E = XMVector3TransformCoord(WorldEnd, InverseWorldTransform);
		XMVECTOR SE = XMVectorSubtract(E, S);
		XMVECTOR V = XMVector3Normalize(SE);
		XMVECTOR Length = XMVector3Length(SE);

		//レイの長さ
		float neart;
		XMStoreFloat(&neart, Length);

		//三角形(面)との交差判定
		const std::vector<Model::Vertex>& vertices = mesh.vertices;
		const std::vector<UINT> indices = mesh.indices;

		int materialIndex = -1;
		XMVECTOR HitPosition;
		XMVECTOR HitNormal;
		for (UINT i = 0; i < indices.size(); i += 3)
		{
			//三角形の頂点を抽出
			const Model::Vertex& a = vertices.at(indices.at(i + 2));
			const Model::Vertex& b = vertices.at(indices.at(i + 1));
			const Model::Vertex& c = vertices.at(indices.at(i));

			XMVECTOR A = XMLoadFloat3(&a.position);
			XMVECTOR B = XMLoadFloat3(&b.position);
			XMVECTOR C = XMLoadFloat3(&c.position);

			//三角形の三辺ベクトルを算出
			XMVECTOR AB = XMVectorSubtract(B, A);
			XMVECTOR BC = XMVectorSubtract(C, B);
			XMVECTOR CA = XMVectorSubtract(A, C);

			//三角形の法線ベクトル
			XMVECTOR N = XMVector3Cross(AB, BC);
			//内積の結果がプラスならば裏向き
			XMVECTOR Dot = XMVector3Dot(V, N);
			float d;
			XMStoreFloat(&d, Dot);
			if (d >= 0)	continue;

			//レイと平面の交点を算出
			XMVECTOR SA = XMVectorSubtract(A, S);
			XMVECTOR X = XMVectorDivide(XMVector3Dot(N, SA), Dot);
			float x;
			XMStoreFloat(&x, X);
			if (x < .0f || x>neart) continue;//交点までの距離が今までに計算した最近距離より大きい時はスキップ

			XMVECTOR P = XMVectorAdd(XMVectorMultiply(V, X), S);

			//交点が三角形の内側にあるか判定
			// 1
			XMVECTOR PA = XMVectorSubtract(A, P);
			XMVECTOR Cross1 = XMVector3Cross(PA, AB);
			XMVECTOR Dot1 = XMVector3Dot(Cross1, N);
			float dot1;
			XMStoreFloat(&dot1, Dot1);
			if (dot1 < .0f) continue;
			// 2
			XMVECTOR PB = XMVectorSubtract(B, P);
			XMVECTOR Cross2 = XMVector3Cross(PB, BC);
			XMVECTOR Dot2 = XMVector3Dot(Cross2, N);
			float dot2;
			XMStoreFloat(&dot2, Dot2);
			if (dot2 < .0f) continue;
			// 3
			XMVECTOR PC = XMVectorSubtract(C, P);
			XMVECTOR Cross3 = XMVector3Cross(PC, CA);
			XMVECTOR Dot3 = XMVector3Dot(Cross3, N);
			float dot3;
			XMStoreFloat(&dot3, Dot3);
			if (dot3 < .0f) continue;

			//最近距離を更新
			neart = x;

			//交点と法線を更新
			HitPosition = P;
			HitNormal = N;
			materialIndex = mesh.materialIndex;
		}
		if (materialIndex >= 0)
		{
			//ローカル空間からワールド空間へ変換
			XMVECTOR WorldPosition = XMVector3TransformCoord(HitPosition, WorldTransform);
			XMVECTOR WorldCrossVec = XMVectorSubtract(WorldPosition, WorldStart);
			XMVECTOR WorldCrossLength = XMVector3Length(WorldCrossVec);
			float distance;
			XMStoreFloat(&distance, WorldCrossLength);

			//ヒット情報保存
			if (result.distance > distance)
			{
				XMVECTOR WorldNormal = XMVector3TransformNormal(HitNormal, WorldTransform);

				result.distance = distance;
				result.materialIndex = materialIndex;
				XMStoreFloat3(&result.position, WorldPosition);
				XMStoreFloat3(&result.normal, XMVector3Normalize(WorldNormal));
				hit = true;
			}
		}
	}
	return hit;
}

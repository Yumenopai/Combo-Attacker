#pragma once

#include <vector>
#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>

#include "RenderContext.h"
using namespace DirectX;

class Gizmos
{
private:
	struct Mesh
	{
		Microsoft::WRL::ComPtr<ID3D11Buffer>	vertexBuffer;
		UINT									vertexCount;
	};

	struct Instance
	{
		Mesh* mesh;
		XMFLOAT4X4	worldTransform;
		XMFLOAT4	color;
	};

	struct CbMesh
	{
		XMFLOAT4X4	worldViewProjection;
		XMFLOAT4	color;
	};

public:
	Gizmos(ID3D11Device* device);
	~Gizmos(){}

	//箱描画
	void DrawBox(const XMFLOAT3& position, const XMFLOAT3& angle, const XMFLOAT3& size,	const XMFLOAT4& color);
	//球描画
	void DrawSphere(const XMFLOAT3& position, float radius, const XMFLOAT4& color);
	// 円柱描画
	void DrawCylinder(const XMFLOAT3& position, float radius, float height, const XMFLOAT3& angle, const XMFLOAT4& color);
	
	//描画実行
	void Render(const RenderContext& rc);

private:
	//メッシュ生成
	void CreateMesh(ID3D11Device* device, const std::vector<XMFLOAT3>& vertices, Mesh& mesh);

	//箱メッシュ作成
	void CreateBoxMesh(ID3D11Device* device, float width, float height, float depth);
	//球メッシュ作成
	void CreateSphereMesh(ID3D11Device* device, float radius, int subdivisions);
	// 円柱メッシュ作成
	void CreateCylinderMesh(ID3D11Device* device, float radius1, float radius2, float start, float height, int slices, int stacks);

private:

	Mesh					boxMesh;
	std::vector<Instance>	instances;

	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		constantBuffer;

	Mesh					sphereMesh;
	Mesh					cylinderMesh;
};
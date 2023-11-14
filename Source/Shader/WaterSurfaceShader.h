#pragma once

#include "Shader.h"

class WaterSurfaceShader : public ModelShader
{
public:
	WaterSurfaceShader(ID3D11Device* device);
	~WaterSurfaceShader() override = default;

	//ï`âÊäJén
	void Begin(const RenderContext& rc) override;

	//ÉÇÉfÉãï`âÊ
	void Draw(const RenderContext& rc, const Model* model) override;
	void DrawByMesh(const RenderContext& rc, const Model* model, const Model::Mesh& mesh) override;

	//ï`âÊèIóπ
	void End(const RenderContext& rc) override;

private:
	struct CbScene
	{
		XMFLOAT4X4 viewProjection;
		XMFLOAT4 lightDirection;
		XMFLOAT4 lightColor;
		XMFLOAT4 cameraPosition;
		XMFLOAT4X4 lightViewProjection;
		XMFLOAT4 shadowColor;
		float shadowTexelSize;
		float padding[3];
	};

	struct CbMesh
	{
		XMFLOAT4	materialColor;

		DirectX::XMFLOAT2 wave1;
		DirectX::XMFLOAT2 wave2;
	};

	struct CbSkeleton
	{
		XMFLOAT4X4 boneTransforms[256];
	};

	Microsoft::WRL::ComPtr<ID3D11Buffer>		sceneConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		meshConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		skeletonConstantBuffer;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	shaderResourceView;
};
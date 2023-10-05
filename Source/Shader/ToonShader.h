#pragma once

#include "Shader.h"

class ToonShader : public ModelShader
{
public:
	ToonShader(ID3D11Device* device);
	~ToonShader() override = default;

	//描画開始
	void Begin(const RenderContext& rc) override;

	//モデル描画
	void Draw(const RenderContext& rc, const Model* model) override;

	//描画終了
	void End(const RenderContext& rc) override;

	//シェーダーリソースビュー取得
	ID3D11ShaderResourceView* GetShaderResourceView() const { return shaderResourceView.Get(); }

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
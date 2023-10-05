#pragma once

#include <wrl.h>
#include <d3d11.h>
#include "Graphics/Model.h"

// 前方宣言
struct RenderContext;

class ShadowMap
{
public:
	ShadowMap(ID3D11Device* device);
	~ShadowMap() = default;

	//開始処理
	void Begin(const RenderContext& rc, const XMFLOAT3& position);

	//描画実行
	void Draw(const RenderContext& rc, const Model* model);

	//終了処理
	void End(const RenderContext& rc);

	//シェーダーリソースビュー取得
	ID3D11ShaderResourceView* GetShaderResourceView() const { return shaderResourceView.Get(); }

	//サンプラーステート取得
	ID3D11SamplerState* GetSamplerState() const { return samplerState.Get(); }

	//ライトビュープロジェクション行列取得
	const XMFLOAT4X4& GetLightViewProjection() const { return lightViewProjection; }

	//テクセルサイズ取得
	float GetTexelSize() const { return 1.0f / textureSize; }

private:
	struct CbScene
	{
		XMFLOAT4X4	lightViewProjection;
	};
	struct CbSkeleton
	{
		XMFLOAT4X4	boneTransforms[256];
	};

	const UINT	textureSize = 4096;

	D3D11_VIEWPORT		cachedViewport;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	cachedRenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	cachedDepthStencilView;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	shaderResourceView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>		depthStencilView;
	
	Microsoft::WRL::ComPtr<ID3D11Buffer>			sceneConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>			skeletonConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>		vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>		pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>		inputLayout;

	Microsoft::WRL::ComPtr<ID3D11SamplerState>		samplerState;
	XMFLOAT4X4										lightViewProjection;
};
#pragma once

#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include "Shader.h"

// スプライト
class SkyBox
{
public:
	SkyBox(const char* filename);
	~SkyBox() {}

	struct Vertex
	{
		DirectX::XMFLOAT3	position;
	};

	struct CbScene
	{
		DirectX::XMFLOAT4X4 inverseViewProjection;
		DirectX::XMFLOAT4 viewPosition;
	};

	// 描画実行
	void Render(const RenderContext& rc);

	// テクスチャ幅取得
	int GetTextureWidth() const { return textureWidth; }
	// テクスチャ高さ取得
	int GetTextureHeight() const { return textureHeight; }

	ID3D11ShaderResourceView* GetShaderResouceView() { return shaderResourceView.Get(); }

private:
	Microsoft::WRL::ComPtr<ID3D11Buffer>				sceneConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>			vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>			pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>			inputLayout;

	Microsoft::WRL::ComPtr<ID3D11Buffer>				vertexBuffer;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	shaderResourceView;

	int textureWidth = 0;
	int textureHeight = 0;
};
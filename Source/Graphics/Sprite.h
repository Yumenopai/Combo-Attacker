#pragma once

#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>

//スプライト
class Sprite
{
public:
	Sprite(ID3D11Device* device);
	Sprite(ID3D11Device* device, const char* filename);

	//頂点データ
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT2 texcoord;
	};

	//描画実行
	void Render(ID3D11DeviceContext* dc,
		float dx, float dy,					//左上位置
		float dz,							//奥行
		float dw, float dh,					//幅 高さ
		float sx, float sy,					//画像切り抜き位置
		float sw, float sh,					//画像切り抜きサイズ
		float angle,						//角度
		float r, float g, float b, float a	//色
	) const;

	//描画実行(テクスチャ切り抜き指定なし)
	void Render(ID3D11DeviceContext* dc,
		float dx, float dy,					//左上位置
		float dz,							//奥行
		float dw, float dh,					//幅 高さ
		float angle,						//角度
		float r, float g, float b, float a	//色
	) const;

	// テクスチャ幅取得
	float GetTextureWidth() const { return textureWidth; }
	// テクスチャ高さ取得
	float GetTextureHeight() const { return textureHeight; }

	// 頂点バッファの取得
	const Microsoft::WRL::ComPtr<ID3D11Buffer>& GetVertexBuffer() const { return vertexBuffer; }

	// シェーダーリソースビューの取得
	const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& GetShaderResourceView() const { return shaderResourceView; }

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	inputLayout;

	Microsoft::WRL::ComPtr<ID3D11Buffer>				vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState>		rasterizerState;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	shaderResourceView;

	float textureWidth = 0;
	float textureHeight = 0;
};
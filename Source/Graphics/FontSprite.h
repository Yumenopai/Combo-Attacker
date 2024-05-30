#pragma once

#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include "RenderContext.h"

//スプライト
class FontSprite
{
private:
	//頂点データ
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT2 texcoord;
	};

private:
	const size_t max_vertices;
	std::vector<Vertex> vertices;


	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>  inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;

	float textureWidth = 0;
	float textureHeight = 0;

public:
	FontSprite(ID3D11Device* device, const char* filename, size_t max_sprites);

	void begin(ID3D11DeviceContext* dc);
	void End(ID3D11DeviceContext* dc);

	//描画実行
	void Render(ID3D11DeviceContext* dc,
		float dx, float dy,					 //左上位置
		float dz,							 //奥行
		float dw, float dh,					 //幅、高さ
		float sx, float sy,					 //画像切り抜き位置
		float sw, float sh,					 //画像切り抜きサイズ
		float angle,						 //角度
		DirectX::XMFLOAT4 color);			 //色

	// テキスト表示
	void Textout(ID3D11DeviceContext* dc, std::string str,
		float dx, float dy,					 //左上位置
		float dz,							 //奥行
		float dw, float dh,					 //幅、高さ
		float sx, float sy,					 //画像切り抜き位置
		float sw, float sh,					 //画像切り抜きサイズ
		float angle,						 //角度
		DirectX::XMFLOAT4 color);			 //色

	// テキスト表示/オーバーロード
	void Textout(ID3D11DeviceContext* dc, std::string str,
		DirectX::XMFLOAT3 position,			 //位置
		DirectX::XMFLOAT2 size,				 //幅、高さ
		DirectX::XMFLOAT2 cutPosition,		 //画像切り抜き位置
		DirectX::XMFLOAT2 cutSize,			 //画像切り抜きサイズ
		float angle,						 //角度
		DirectX::XMFLOAT4 color);			 //色

	//スプライトの幅を取得
	float GetTextureWidth() { return textureWidth; }

	//スプライトの高さを取得
	float GetTextureHeight() { return textureHeight; }

	//頂点バッファの取得
	const Microsoft::WRL::ComPtr<ID3D11Buffer>& GetVertexBuffer() const { return vertexBuffer; }

	// シェーダーリソースビューの取得
	const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& GetShaderResourceView() const { return shaderResourceView; }
};
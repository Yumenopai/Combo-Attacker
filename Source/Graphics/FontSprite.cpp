#include <fstream>
#include "FontSprite.h"
#include "Misc.h"
#include "GpuResourceUtils.h"

//コンストラクタ
FontSprite::FontSprite(ID3D11Device* device, const char* filename, size_t max_sprites) : max_vertices(max_sprites * 6)
{
	HRESULT hr = S_OK;

	std::unique_ptr<Vertex[]> vertices{ std::make_unique<Vertex[]>(max_vertices) };

	//頂点バッファの生成 (頂点データをGPUで扱うための格納用バッファ)
	{
		//頂点バッファを作成するための設定オプション
		D3D11_BUFFER_DESC buffer_desc = {};
		buffer_desc.ByteWidth = static_cast<int>(sizeof(Vertex) * max_vertices);
		//USAGE_DYNAMIC,CPU_ACCESS_WRITEを指定することで毎フレーム頂点を編集できるようになる
		buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		buffer_desc.MiscFlags = 0;
		buffer_desc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA subresource_data{};
		subresource_data.pSysMem = vertices.get();
		subresource_data.SysMemPitch = 0;
		subresource_data.SysMemSlicePitch = 0;
		//頂点バッファオブジェクトの生成
		hr = device->CreateBuffer(&buffer_desc, &subresource_data, vertexBuffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	//頂点シェーダー
	{
		//入力レイアウト  GPUが送られてきた頂点データの内容を理解するためのもの
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
		{
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		};

		hr = GpuResourceUtils::LoadVertexShader(
			device,
			"Data/Shader/SpriteVS.cso",
			inputElementDesc,
			ARRAYSIZE(inputElementDesc),
			inputLayout.GetAddressOf(),
			vertexShader.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	//ピクセルシェーダー
	{
		hr = GpuResourceUtils::LoadPixelShader(
			device,
			"Data/Shader/SpritePS.cso",
			pixelShader.GetAddressOf());

		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	//テクスチャの生成
	{
		if (filename != nullptr)
		{
			//テクスチャファイル読み込み
			D3D11_TEXTURE2D_DESC desc;
			hr = GpuResourceUtils::LoadTexture(device, filename, shaderResourceView.GetAddressOf(), &desc); //テクスチャ読み込み時にテクスチャサイズを取得
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

			textureWidth = static_cast<float>(desc.Width);
			textureHeight = static_cast<float>(desc.Height);
		}
		else
		{
			//ダミーテクスチャを生成
			D3D11_TEXTURE2D_DESC desc;
			hr = GpuResourceUtils::CreateDummyTexture(device, 0xFFFFFFFF, shaderResourceView.GetAddressOf(), &desc);
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

			textureWidth = static_cast<float>(desc.Width);
			textureHeight = static_cast<float>(desc.Height);
		}
	}
}

//描画実行
void FontSprite::Render(ID3D11DeviceContext * dc,
	float dx, float dy,					 //左上位置
	float dz,							 //奥行
	DirectX::XMFLOAT3 offset,
	float dw, float dh,					 //幅、高さ
	float sx, float sy,					 //画像切り抜き位置
	float sw, float sh,					 //画像切り抜きサイズ
	float angle,                         //角度
	DirectX::XMFLOAT4 color				 //色
)
{

	dx += offset.x;
	dy += offset.y;
	dz += offset.z;

	//画面に描画するときは -1.0 ～ 1.0の範囲で指定する必要がある。
	//この空間をNDC空間と呼ぶ。

	//頂点座標
	DirectX::XMFLOAT2 positions[] =
	{
		DirectX::XMFLOAT2(dx, dy), //左上
		DirectX::XMFLOAT2(dx + dw, dy), //右上
		DirectX::XMFLOAT2(dx, dy + dh), //左下
		DirectX::XMFLOAT2(dx + dw, dy + dh), //右下
	};

	//テクスチャ座標
	DirectX::XMFLOAT2 texcoords[]{
		DirectX::XMFLOAT2(sx, sy), //左上
		DirectX::XMFLOAT2(sx + sw, sy), //右上
		DirectX::XMFLOAT2(sx, sy + sh), //左下
		DirectX::XMFLOAT2(sx + sw, sy + sh), //右下
	};

	//スプライトの中心で回転させるために４頂点の中心位置が
	//原点(0、0)になるようにいったん頂点を移動させる
	float mx = dx + dw * 0.5f;
	float my = dy + dh * 0.5f;
	for (auto& p : positions)
	{
		p.x -= mx;
		p.y -= my;
	}

	//頂点を回転させる
	float theta = DirectX::XMConvertToRadians(angle); //角度をラジアン単位の角度(弧度法)に変換
	float c = cosf(theta);
	float s = sinf(theta);
	for (auto& p : positions)
	{
		DirectX::XMFLOAT2 r = p;
		//アフィン変換(拡大縮小と回転を組み合わせた変換)
		p.x = c * r.x + -s * r.y;
		p.y = s * r.x + c * r.y;
	}

	//回転のために移動させた頂点を元の位置に戻す
	for (auto& p : positions)
	{
		p.x += mx;
		p.y += my;
	}

	//現在設定されているビューポートからスクリーンサイズを取得する
	D3D11_VIEWPORT viewport;
	UINT numViewports = 1;
	dc->RSGetViewports(&numViewports, &viewport);
	float screenWidth = viewport.Width;
	float screenHeight = viewport.Height;

	//スクリーン座標系からNDC座標系に変換する
	for (DirectX::XMFLOAT2& p : positions)
	{
		//　　計算式(-1.0～1.0の範囲に正規化)
		// x = (2 * x_screen / screen_width) - 1    正規化するためにx座標を幅で割って値を0.0～1.0の範囲になる、
		// y = 1 - (2 * y_screen / screen_height)	NDC(-1.0～1.0)の範囲に変換するためにx座標に2をかけて幅で割る(0.0～2.0)
		//											そこに1を引いて(-1.0～1.0)の範囲にする
		p.x = 2.0f * p.x / screenWidth - 1.0f;
		p.y = 1.0f - 2.0f * p.y / screenHeight;
	}

	float u0{ sx / textureWidth };
	float v0{ sy / textureHeight };
	float u1{ (sx + sw) / textureWidth };
	float v1{ (sy + sh) / textureHeight };

	vertices.push_back({ {positions[0].x, positions[0].y, 0, }, color,{u0, v0} });
	vertices.push_back({ {positions[1].x, positions[1].y, 0, }, color,{u1, v0} });
	vertices.push_back({ {positions[2].x, positions[2].y, 0, }, color,{u0, v1} });
	vertices.push_back({ {positions[2].x, positions[2].y, 0, }, color,{u0, v1} });
	vertices.push_back({ {positions[1].x, positions[1].y, 0, }, color,{u1, v0} });
	vertices.push_back({ {positions[3].x, positions[3].y, 0, }, color,{u1, v1} });
}

void FontSprite::begin(ID3D11DeviceContext* dc)
{
	vertices.clear();

	dc->VSSetShader(vertexShader.Get(), nullptr, 0);
	dc->PSSetShader(pixelShader.Get(), nullptr, 0);

	dc->PSSetShaderResources(0, 1, shaderResourceView.GetAddressOf());
}

void FontSprite::End(ID3D11DeviceContext* dc)
{
	//頂点バッファの内容の編集を開始する。
	D3D11_MAPPED_SUBRESOURCE mappedSubresouse;
	HRESULT hr = dc->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresouse);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	//頂点バッファの内容を編集
	size_t vertex_count = vertices.size();
	_ASSERT_EXPR(max_vertices >= vertex_count, "Buffer overflow");

	Vertex* data{ reinterpret_cast<Vertex*>(mappedSubresouse.pData) };
	if (data != nullptr)
	{
		const Vertex* p = vertices.data();
		memcpy_s(data, max_vertices * sizeof(Vertex), p, vertex_count * sizeof(Vertex));
	}

	//頂点バッファの内容の編集を終了する
	dc->Unmap(vertexBuffer.Get(), 0);

	{
		//GPUに描画するためのデータを渡す
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		dc->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
		dc->IASetInputLayout(inputLayout.Get());
		dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//描画
		dc->Draw(static_cast<UINT>(vertex_count), 0);
	}
}

void FontSprite::Textout(ID3D11DeviceContext* dc, std::string str,
	float dx, float dy,					 //左上位置
	float dz,							 //奥行
	DirectX::XMFLOAT3 offset,
	float dw, float dh,					 //幅、高さ
	float sx, float sy,					 //画像切り抜き位置
	float sw, float sh,					 //画像切り抜きサイズ
	float angle,						 //角度
	DirectX::XMFLOAT4 color)			 //色
{
	//文字列描画
	int i = 2;
	begin(dc);

	for (char c : str)
	{
		int x = c % 16;
		int y = c / 16;
		Render(dc, dx + i * dw, dy, dz, offset, dw, dh, x * sx, y * sy, textureWidth / sw, textureHeight / sh, angle, color);
		++i;
	}
	End(dc);
}
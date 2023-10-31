#include "Misc.h"
#include "Graphics/GpuResourceUtils.h"
#include "Graphics/RenderContext.h"
#include "SkyBox.h"
#include "Graphics/Graphics.h"

// コンストラクタ
SkyBox::SkyBox(const char* filename)
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	HRESULT hr = S_OK;

	// 入力レイアウト
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
		D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	//頂点シェーダ
	GpuResourceUtils::LoadVertexShader(
		device,
		"Data/Shader/SkyboxVS.cso",
		inputElementDesc,
		_countof(inputElementDesc),
		inputLayout.GetAddressOf(),
		vertexShader.GetAddressOf());

	//ピクセルシェーダ
	GpuResourceUtils::LoadPixelShader(
		device,
		"Data/Shader/SkyboxPS.cso",
		pixelShader.GetAddressOf());

	//シーン用定数バッファ
	GpuResourceUtils::CreateConstantBuffer(
		device,
		sizeof(CbScene),
		sceneConstantBuffer.GetAddressOf());

	// テクスチャ読み込み
	GpuResourceUtils::LoadTexture(
		device,
		filename,
		shaderResourceView.GetAddressOf());

	// 頂点データの定義
	Vertex vertices[] = {
		{ DirectX::XMFLOAT3(-1.0, +1.0, 0) },
		{ DirectX::XMFLOAT3(+1.0, +1.0, 0) },
		{ DirectX::XMFLOAT3(-1.0, -1.0, 0) },
		{ DirectX::XMFLOAT3(+1.0, -1.0, 0) },
	};

	// 頂点バッファの生成
	{
		// 頂点バッファを作成するための設定オプション
		D3D11_BUFFER_DESC buffer_desc = {};
		buffer_desc.ByteWidth = sizeof(vertices);	// バッファ（データを格納する入れ物）のサイズ
		buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;	// 頂点バッファとしてバッファを作成する。
		buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		buffer_desc.MiscFlags = 0;
		buffer_desc.StructureByteStride = 0;
		// 頂点バッファに頂点データを入れるための設定
		D3D11_SUBRESOURCE_DATA subresource_data = {};
		subresource_data.pSysMem = vertices;	// ここに格納したい頂点データのアドレスを渡すことでCreateBuffer()時にデータを入れることができる。
		subresource_data.SysMemPitch = 0; //Not use for vertex buffers.
		subresource_data.SysMemSlicePitch = 0; //Not use for vertex buffers.
		// 頂点バッファオブジェクトの生成
		hr = device->CreateBuffer(&buffer_desc, &subresource_data, &vertexBuffer);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}
}

// 描画実行
void SkyBox::Render(const RenderContext& rc)
{
	ID3D11DeviceContext* dc = rc.deviceContext;
	// シェーダー設定
	dc->IASetInputLayout(inputLayout.Get());
	dc->VSSetShader(vertexShader.Get(), nullptr, 0);
	dc->PSSetShader(pixelShader.Get(), nullptr, 0);

	// シーン用定数バッファ更新
	CbScene cbScene;
	DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&rc.view);
	DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&rc.projection);
	DirectX::XMStoreFloat4x4(&cbScene.inverseViewProjection, DirectX::XMMatrixInverse(nullptr, V * P));

	Camera& camera = Camera::Instance();
	cbScene.viewPosition.x = camera.GetEye().x;
	cbScene.viewPosition.y = camera.GetEye().y;
	cbScene.viewPosition.z = camera.GetEye().z;
	cbScene.viewPosition.w = 1.0f;

	dc->UpdateSubresource(sceneConstantBuffer.Get(), 0, 0, &cbScene, 0, 0);
	dc->VSSetConstantBuffers(1, 1, sceneConstantBuffer.GetAddressOf());
	
	//サンプラーステート設定
	ID3D11SamplerState* samplerStates[] =
	{
		rc.renderState->GetSamplerState(SamplerState::LinearWrap),
	};
	dc->PSSetSamplers(0, _countof(samplerStates), samplerStates);

	//レンダーステート設定
	const float blend_factor[4] = { 1.0f,1.0f,1.0f,1.0f };
	dc->OMSetBlendState(rc.renderState->GetBlendState(BlendState::Transparency), blend_factor, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(rc.renderState->GetDepthStencilState(DepthState::WriteOnly), 0);
	dc->RSSetState(rc.renderState->GetRasterizerState(RasterizerState::SolidCullNone));

	// 頂点バッファ設定
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	dc->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	// プリミティブ設定
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// シェーダーリソースビュー設定
	dc->PSSetShaderResources(0, 1, shaderResourceView.GetAddressOf());

	// 描画
	dc->Draw(4, 0);
}
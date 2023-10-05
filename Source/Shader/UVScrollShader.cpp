#include "Misc.h"
#include "./Graphics/GpuResourceUtils.h"
#include "UVScrollShader.h"

UVScrollShader::UVScrollShader(ID3D11Device* device)
{
	// 入力レイアウト
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
			D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	//頂点シェーダ
	GpuResourceUtils::LoadVertexShader(
		device,
		"Data/Shader/UVScrollVS.cso",
		inputElementDesc,
		_countof(inputElementDesc),
		inputLayout.GetAddressOf(),
		vertexShader.GetAddressOf());

	//ピクセルシェーダ
	GpuResourceUtils::LoadPixelShader(
		device,
		"Data/Shader/UVScrollPS.cso",
		pixelShader.GetAddressOf());

	//シーン用定数バッファ
	GpuResourceUtils::CreateConstantBuffer(
		device,
		sizeof(CbScroll),
		scrollConstantBuffer.GetAddressOf());
}

// 描画開始
void UVScrollShader::Begin(const RenderContext& rc)
{
	rc.deviceContext->VSSetShader(vertexShader.Get(), nullptr, 0);
	rc.deviceContext->PSSetShader(pixelShader.Get(), nullptr, 0);
	rc.deviceContext->IASetInputLayout(inputLayout.Get());

	rc.deviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_UNKNOWN, 0);
	rc.deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	ID3D11Buffer* constantBuffers[] =
	{
		scrollConstantBuffer.Get(),
	};
	rc.deviceContext->VSSetConstantBuffers(0, ARRAYSIZE(constantBuffers), constantBuffers);
	rc.deviceContext->PSSetConstantBuffers(0, ARRAYSIZE(constantBuffers), constantBuffers);
	
	const float blend_factor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	rc.deviceContext->OMSetBlendState(blendState.Get(), blend_factor, 0xFFFFFFFF);
	rc.deviceContext->OMSetDepthStencilState(depthStencilState.Get(), 0);
	rc.deviceContext->RSSetState(rasterizerState.Get());
	rc.deviceContext->PSSetSamplers(0, 1, samplerState.GetAddressOf());
}

// 描画
void UVScrollShader::Draw(const RenderContext& rc, const Sprite* sprite)
{
	CbScroll cbScroll;
	cbScroll.uvScrollValue = rc.uvScrollData.uvScrollValue;
	rc.deviceContext->UpdateSubresource(scrollConstantBuffer.Get(), 0, 0, &cbScroll, 0, 0);
	
	UINT stride = sizeof(Sprite::Vertex);
	UINT offset = 0;
	rc.deviceContext->IASetVertexBuffers(0, 1, sprite->GetVertexBuffer().GetAddressOf(), &stride, &offset);
	rc.deviceContext->PSSetShaderResources(0, 1, sprite->GetShaderResourceView().GetAddressOf());
	rc.deviceContext->Draw(4, 0);
}

// 描画終了
void UVScrollShader::End(const RenderContext& rc)
{
	rc.deviceContext->VSSetShader(nullptr, nullptr, 0);
	rc.deviceContext->PSSetShader(nullptr, nullptr, 0);
	rc.deviceContext->IASetInputLayout(nullptr);
}
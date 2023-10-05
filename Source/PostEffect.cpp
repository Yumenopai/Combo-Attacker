#include <imgui.h>
#include "PostEffect.h"
#include "Graphics/GpuResourceUtils.h"

PostEffect::PostEffect(ID3D11Device* device)
{
	//�t���X�N���[���N�A�b�h���_�V�F�[�_�ǂݍ���
	GpuResourceUtils::LoadVertexShader(
		device,
		"Data/Shader/FullScreenQuadVS.cso",
		nullptr, 0, nullptr, fullscreenQuadVS
		.GetAddressOf());

	//�P�x���o�s�N�Z���V�F�[�_�ǂݍ���
	GpuResourceUtils::LoadPixelShader(
		device,
		"Data/Shader/LuminanceExtractionPS.cso",
		luminanceExtractionPS.GetAddressOf());

	//�萔�o�b�t�@�쐬
	GpuResourceUtils::CreateConstantBuffer(
		device,
		sizeof(CbPostEffect),
		constantBuffer.GetAddressOf());

	//�u���[���s�N�Z���V�F�[�_�ǂݍ���
	GpuResourceUtils::LoadPixelShader(
		device,
		"Data/Shader/BloomPS.cso",
		bloomPS.GetAddressOf());
}

//�J�n����
void PostEffect::Begin(const RenderContext& rc)
{
	ID3D11DeviceContext* dc = rc.deviceContext;
	const RenderState* renderState = rc.renderState;

	//�u�����h�X�e�[�g�ݒ�
	FLOAT blendFactor[4] = { 1.0f,1.0f,1.0f,1.0f };
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), blendFactor, 0xFFFFFFFF);

	//�[�x�X�e���V���X�e�[�g�ݒ�
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::NoTestNoWrite), 0);

	//���X�^���C�U�X�e�[�g�ݒ�
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullNone));

	//���_�o�b�t�@�ݒ�(�g�p���Ȃ�)
	dc->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	dc->IASetInputLayout(nullptr);

	//�T���v���[�X�e�[�g�ݒ�
	ID3D11SamplerState* samplers[] =
	{
		renderState->GetSamplerState(SamplerState::LinearWrap)
	};
	dc->PSSetSamplers(0, _countof(samplers), samplers);

	//�萔�o�b�t�@�ݒ�
	dc->PSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

	//�萔�o�b�t�@�X�V
	dc->UpdateSubresource(constantBuffer.Get(), 0, 0, &cbPostEffect, 0, 0);
}

//�P�x���o����
void PostEffect::LuminanceExtraction(const RenderContext& rc, ID3D11ShaderResourceView* colorMap)
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	//�V�F�[�_�ݒ�
	dc->VSSetShader(fullscreenQuadVS.Get(), 0, 0);
	dc->PSSetShader(luminanceExtractionPS.Get(), 0, 0);

	//�V�F�[�_�[���\�[�X�ݒ�
	ID3D11ShaderResourceView* srvs[] = { colorMap };
	dc->PSSetShaderResources(0, _countof(srvs), srvs);

	//�`��
	dc->Draw(4, 0);
}

//�u���[������
void PostEffect::Bloom(const RenderContext& rc, ID3D11ShaderResourceView* colorMap, ID3D11ShaderResourceView* luminanceMap)
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	//�V�F�[�_�ݒ�
	dc->VSSetShader(fullscreenQuadVS.Get(), 0, 0);
	dc->PSSetShader(bloomPS.Get(), 0, 0);

	//�V�F�[�_�[���\�[�X�ݒ�
	ID3D11ShaderResourceView* srvs[] = { colorMap,luminanceMap };
	dc->PSSetShaderResources(0, _countof(srvs), srvs);

	//�`��
	dc->Draw(4, 0);
}

//�f�o�b�OGUI�`��
void PostEffect::DrawDebugGUI()
{
	ImGui::DragFloat("LuminanceLowerEdge", &cbPostEffect.luminanceExtractionLowerEdge, 0.01f, 0, 1.0f);
	ImGui::DragFloat("LuminanceHigherEdge", &cbPostEffect.luminanceExtractionHigherEdge, 0.01f, 0, 1.0f);
	ImGui::DragFloat("GaussianSigma", &cbPostEffect.gaussianSigma, 0.01f, 0, 10.0f);
	ImGui::DragFloat("BloomIntensity", &cbPostEffect.bloomIntensity, 0.1f, 0, 10.0f);
}

//�I������
void PostEffect::End(const RenderContext& rc)
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	//�ݒ肳��Ă���V�F�[�_�[���\�[�X������
	ID3D11ShaderResourceView* srvs[] = { nullptr ,nullptr };
	dc->PSSetShaderResources(0, _countof(srvs), srvs);
}
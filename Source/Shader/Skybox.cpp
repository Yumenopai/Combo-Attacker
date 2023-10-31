#include "Misc.h"
#include "Graphics/GpuResourceUtils.h"
#include "Graphics/RenderContext.h"
#include "SkyBox.h"
#include "Graphics/Graphics.h"

// �R���X�g���N�^
SkyBox::SkyBox(const char* filename)
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	HRESULT hr = S_OK;

	// ���̓��C�A�E�g
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT,
		D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	//���_�V�F�[�_
	GpuResourceUtils::LoadVertexShader(
		device,
		"Data/Shader/SkyboxVS.cso",
		inputElementDesc,
		_countof(inputElementDesc),
		inputLayout.GetAddressOf(),
		vertexShader.GetAddressOf());

	//�s�N�Z���V�F�[�_
	GpuResourceUtils::LoadPixelShader(
		device,
		"Data/Shader/SkyboxPS.cso",
		pixelShader.GetAddressOf());

	//�V�[���p�萔�o�b�t�@
	GpuResourceUtils::CreateConstantBuffer(
		device,
		sizeof(CbScene),
		sceneConstantBuffer.GetAddressOf());

	// �e�N�X�`���ǂݍ���
	GpuResourceUtils::LoadTexture(
		device,
		filename,
		shaderResourceView.GetAddressOf());

	// ���_�f�[�^�̒�`
	Vertex vertices[] = {
		{ DirectX::XMFLOAT3(-1.0, +1.0, 0) },
		{ DirectX::XMFLOAT3(+1.0, +1.0, 0) },
		{ DirectX::XMFLOAT3(-1.0, -1.0, 0) },
		{ DirectX::XMFLOAT3(+1.0, -1.0, 0) },
	};

	// ���_�o�b�t�@�̐���
	{
		// ���_�o�b�t�@���쐬���邽�߂̐ݒ�I�v�V����
		D3D11_BUFFER_DESC buffer_desc = {};
		buffer_desc.ByteWidth = sizeof(vertices);	// �o�b�t�@�i�f�[�^���i�[������ꕨ�j�̃T�C�Y
		buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;	// ���_�o�b�t�@�Ƃ��ăo�b�t�@���쐬����B
		buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		buffer_desc.MiscFlags = 0;
		buffer_desc.StructureByteStride = 0;
		// ���_�o�b�t�@�ɒ��_�f�[�^�����邽�߂̐ݒ�
		D3D11_SUBRESOURCE_DATA subresource_data = {};
		subresource_data.pSysMem = vertices;	// �����Ɋi�[���������_�f�[�^�̃A�h���X��n�����Ƃ�CreateBuffer()���Ƀf�[�^�����邱�Ƃ��ł���B
		subresource_data.SysMemPitch = 0; //Not use for vertex buffers.
		subresource_data.SysMemSlicePitch = 0; //Not use for vertex buffers.
		// ���_�o�b�t�@�I�u�W�F�N�g�̐���
		hr = device->CreateBuffer(&buffer_desc, &subresource_data, &vertexBuffer);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}
}

// �`����s
void SkyBox::Render(const RenderContext& rc)
{
	ID3D11DeviceContext* dc = rc.deviceContext;
	// �V�F�[�_�[�ݒ�
	dc->IASetInputLayout(inputLayout.Get());
	dc->VSSetShader(vertexShader.Get(), nullptr, 0);
	dc->PSSetShader(pixelShader.Get(), nullptr, 0);

	// �V�[���p�萔�o�b�t�@�X�V
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
	
	//�T���v���[�X�e�[�g�ݒ�
	ID3D11SamplerState* samplerStates[] =
	{
		rc.renderState->GetSamplerState(SamplerState::LinearWrap),
	};
	dc->PSSetSamplers(0, _countof(samplerStates), samplerStates);

	//�����_�[�X�e�[�g�ݒ�
	const float blend_factor[4] = { 1.0f,1.0f,1.0f,1.0f };
	dc->OMSetBlendState(rc.renderState->GetBlendState(BlendState::Transparency), blend_factor, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(rc.renderState->GetDepthStencilState(DepthState::WriteOnly), 0);
	dc->RSSetState(rc.renderState->GetRasterizerState(RasterizerState::SolidCullNone));

	// ���_�o�b�t�@�ݒ�
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	dc->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	// �v���~�e�B�u�ݒ�
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

	// �V�F�[�_�[���\�[�X�r���[�ݒ�
	dc->PSSetShaderResources(0, 1, shaderResourceView.GetAddressOf());

	// �`��
	dc->Draw(4, 0);
}
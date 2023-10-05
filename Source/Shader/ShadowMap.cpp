#include "Misc.h"
#include "Graphics/GpuResourceUtils.h"
#include "Graphics/RenderContext.h"
#include "ShadowMap.h"
#include "Stage.h"

ShadowMap::ShadowMap(ID3D11Device* device)
{
	//���̓��C�A�E�g
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{"POSITION",	0,DXGI_FORMAT_R32G32B32_FLOAT, 0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BONE_WEIGHTS",0,DXGI_FORMAT_R32G32B32A32_FLOAT, 0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BONE_INDICES",0,DXGI_FORMAT_R32G32B32A32_UINT, 0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
	};

	//���_�V�F�[�_
	GpuResourceUtils::LoadVertexShader(
		device,
		"Data/Shader/ShadowMapVS.cso",
		inputElementDesc,
		_countof(inputElementDesc),
		inputLayout.GetAddressOf(),
		vertexShader.GetAddressOf());

	//�V�[���p�萔�o�b�t�@
	GpuResourceUtils::CreateConstantBuffer(
		device,
		sizeof(CbScene),
		sceneConstantBuffer.GetAddressOf());

	//�X�P���g���p�萔�o�b�t�@
	GpuResourceUtils::CreateConstantBuffer(
		device,
		sizeof(CbSkeleton),
		skeletonConstantBuffer.GetAddressOf());

	//�[�x�X�e���V���r���[���V�F�[�_�[���\�[�X�r���[����
	{
		//�e�N�X�`������
		D3D11_TEXTURE2D_DESC texture2dDesc{};
		texture2dDesc.Width = textureSize;
		texture2dDesc.Height = textureSize;
		texture2dDesc.MipLevels = 1;
		texture2dDesc.ArraySize = 1;
		texture2dDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		texture2dDesc.SampleDesc.Count = 1;
		texture2dDesc.SampleDesc.Quality = 0;
		texture2dDesc.Usage = D3D11_USAGE_DEFAULT;
		texture2dDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
		texture2dDesc.CPUAccessFlags = 0;
		texture2dDesc.MiscFlags = 0;

		Microsoft::WRL::ComPtr<ID3D11Texture2D> texture2d;
		HRESULT hr = device->CreateTexture2D(&texture2dDesc, nullptr, texture2d.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	
		//�[�x�X�e���V���r���[�̐���
		D3D11_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
		depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilViewDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		depthStencilViewDesc.Texture2D.MipSlice = 0;
		hr = device->CreateDepthStencilView(texture2d.Get(), &depthStencilViewDesc, depthStencilView.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		//�V�F�[�_�[���\�[�X�r���[����
		D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc{};
		shaderResourceViewDesc.Format = DXGI_FORMAT_R32_FLOAT;
		shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;
		hr = device->CreateShaderResourceView(texture2d.Get(), &shaderResourceViewDesc, shaderResourceView.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}
	//�T���v���[�X�e�[�g
	{
		D3D11_SAMPLER_DESC desc{};
		desc.MipLODBias = 0.0f;
		desc.MaxAnisotropy = 1;
		desc.ComparisonFunc = D3D11_COMPARISON_LESS;
		desc.MinLOD = 0;
		desc.MaxLOD = 0;
		desc.BorderColor[0] = D3D11_FLOAT32_MAX;
		desc.BorderColor[1] = D3D11_FLOAT32_MAX;
		desc.BorderColor[2] = D3D11_FLOAT32_MAX;
		desc.BorderColor[3] = D3D11_FLOAT32_MAX;
		desc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
		desc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
		desc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
		desc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT;

		HRESULT hr = device->CreateSamplerState(&desc, samplerState.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}
}

void ShadowMap::Begin(const RenderContext& rc, const XMFLOAT3& position)
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	//�ݒ蒆�̃����_�[�^�[�Q�b�g���ꎞ�I�ɕێ�����
	UINT numViewports = 1;
	dc->RSGetViewports(&numViewports, &cachedViewport);
	dc->OMGetRenderTargets(1, cachedRenderTargetView.ReleaseAndGetAddressOf(), cachedDepthStencilView.ReleaseAndGetAddressOf());

	//�����_�[�^�[�Q�b�g���[�x�X�e���V���ݒ�
	dc->OMSetRenderTargets(0, nullptr, depthStencilView.Get());
	dc->ClearDepthStencilView(depthStencilView.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

	//�r���[�|�[�g�ݒ�
	D3D11_VIEWPORT viewport;
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = static_cast<float>(textureSize);
	viewport.Height = static_cast<float>(textureSize);
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	dc->RSSetViewports(1, &viewport);

	//�V�F�[�_�ݒ�
	dc->VSSetShader(vertexShader.Get(), nullptr, 0);
	dc->PSSetShader(nullptr, nullptr, 0);
	dc->IASetInputLayout(inputLayout.Get());

	//�����_�[�X�e�[�g�ݒ�
	const float blendFactor[4] = { 1.0f,1.0f,1.0f,1.0f };
	dc->OMSetBlendState(rc.renderState->GetBlendState(BlendState::Opaque), blendFactor, 0xFFFFFFFF);
	dc->RSSetState(rc.renderState->GetRasterizerState(RasterizerState::SolidCullBack));

	//�萔�o�b�t�@�ݒ�
	ID3D11Buffer* constantBuffers[] =
	{
		sceneConstantBuffer.Get(),
		nullptr,
		skeletonConstantBuffer.Get(),
	};
	dc->VSSetConstantBuffers(0, _countof(constantBuffers), constantBuffers);

	//���C�g�r���[�v���W�F�N�V�����s��쐬
	const DirectionalLight& directionalLight = rc.lightManager->GetDirectionalLight();
	XMVECTOR LightDirection = XMLoadFloat3(&directionalLight.direction);
	LightDirection = XMVector3Normalize(LightDirection);
	XMVECTOR Up = XMVectorSet(0, 1, 0, 0);
	XMVECTOR Focus = XMLoadFloat3(&position);
	XMVECTOR Eye = XMVectorSubtract(Focus, XMVectorScale(LightDirection, 50.0f));
	XMMATRIX View = XMMatrixLookAtLH(Eye, Focus, Up);
	XMMATRIX Projection = XMMatrixOrthographicLH(120, 120, 0.1f, 200.0f);
	XMMATRIX ViewProjection = XMMatrixMultiply(View, Projection);
	XMStoreFloat4x4(&lightViewProjection, ViewProjection);

	//�V�[���p�萔�o�b�t�@�X�V
	CbScene cbScene{};
	XMStoreFloat4x4(&cbScene.lightViewProjection, ViewProjection);
	dc->UpdateSubresource(sceneConstantBuffer.Get(), 0, 0, &cbScene, 0, 0);
}

//�`��
void ShadowMap::Draw(const RenderContext& rc, const Model* model)
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	for (const Model::Mesh& mesh : model->GetMeshes())
	{
		//���_�o�b�t�@�ݒ�
		UINT stride = sizeof(Model::Vertex);
		UINT offset = 0;
		dc->IASetVertexBuffers(0, 1, mesh.vertexBuffer.GetAddressOf(), &stride, &offset);
		dc->IASetIndexBuffer(mesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
		dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		//�X�P���g���p�萔�o�b�t�@�X�V
		CbSkeleton cbSkeleton{};
		if (mesh.bones.size() > 0)
		{
			for (size_t i = 0; i < mesh.bones.size(); ++i)
			{
				const Model::Bone& bone = mesh.bones.at(i);
				XMMATRIX WorldTransform = XMLoadFloat4x4(&bone.node->worldTransform);
				XMMATRIX OffsetTransform = XMLoadFloat4x4(&bone.offsetTransform);
				XMMATRIX BoneTransform = OffsetTransform * WorldTransform;
				XMStoreFloat4x4(&cbSkeleton.boneTransforms[i], BoneTransform);
			}
		}
		else
		{
			cbSkeleton.boneTransforms[0] = mesh.node->worldTransform;
		}
		dc->UpdateSubresource(skeletonConstantBuffer.Get(), 0, 0, &cbSkeleton, 0, 0);

		//�`��
		dc->DrawIndexed(static_cast<UINT>(mesh.indices.size()), 0, 0);
	}
}

//�`��I��
void ShadowMap::End(const RenderContext& rc)
{
	ID3D11DeviceContext* dc = rc.deviceContext;
	
	//�����_�[�^�[�Q�b�g�����̏�Ԃɖ߂�
	dc->OMSetRenderTargets(1, cachedRenderTargetView.GetAddressOf(), cachedDepthStencilView.Get());
	dc->RSSetViewports(1, &cachedViewport);
}
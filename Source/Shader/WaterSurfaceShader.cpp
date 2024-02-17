#include "Misc.h"
#include "Graphics/GpuResourceUtils.h"
#include "WaterSurfaceShader.h"

WaterSurfaceShader::WaterSurfaceShader(ID3D11Device* device)
{
	//���̓��C�A�E�g
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT, 0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BONE_WEIGHTS",0,DXGI_FORMAT_R32G32B32A32_FLOAT, 0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BONE_INDICES",0,DXGI_FORMAT_R32G32B32A32_UINT, 0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"TANGENT",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
	};

	//���_�V�F�[�_
	GpuResourceUtils::LoadVertexShader(
		device,
		"Data/Shader/WaterSurfaceVS.cso",
		inputElementDesc,
		_countof(inputElementDesc),
		inputLayout.GetAddressOf(),
		vertexShader.GetAddressOf());

	//�s�N�Z���V�F�[�_
	GpuResourceUtils::LoadPixelShader(
		device,
		"Data/Shader/WaterSurfacePS.cso",
		pixelShader.GetAddressOf());

	//�V�[���p�萔�o�b�t�@
	GpuResourceUtils::CreateConstantBuffer(
		device,
		sizeof(CbScene),
		sceneConstantBuffer.GetAddressOf());

	//���b�V���p�萔�o�b�t�@
	GpuResourceUtils::CreateConstantBuffer(
		device,
		sizeof(CbMesh),
		meshConstantBuffer.GetAddressOf());

	//�X�P���g���p�萔�o�b�t�@
	GpuResourceUtils::CreateConstantBuffer(
		device,
		sizeof(CbSkeleton),
		skeletonConstantBuffer.GetAddressOf());
}

//�`��J�n
void WaterSurfaceShader::Begin(const RenderContext& rc)
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	//�V�F�[�_�ݒ�
	dc->IASetInputLayout(inputLayout.Get());
	dc->VSSetShader(vertexShader.Get(), nullptr, 0);
	dc->PSSetShader(pixelShader.Get(), nullptr, 0);

	//�萔�o�b�t�@�ݒ�
	ID3D11Buffer* constantBuffers[] =
	{
		sceneConstantBuffer.Get(),
		meshConstantBuffer.Get(),
		skeletonConstantBuffer.Get(),
	};
	dc->VSSetConstantBuffers(0, _countof(constantBuffers), constantBuffers);
	dc->PSSetConstantBuffers(0, _countof(constantBuffers), constantBuffers);

	//�T���v���[�X�e�[�g�ݒ�
	ID3D11SamplerState* samplerStates[] =
	{
		rc.renderState->GetSamplerState(SamplerState::LinearWrap),
		rc.shadowMap->GetSamplerState(),
	};
	dc->PSSetSamplers(0, _countof(samplerStates), samplerStates);

	//�����_�[�X�e�[�g�ݒ�
	const float blend_factor[4] = { 1.0f,1.0f,1.0f,1.0f };
	dc->OMSetBlendState(rc.renderState->GetBlendState(BlendState::Transparency), blend_factor, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(rc.renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
	dc->RSSetState(rc.renderState->GetRasterizerState(RasterizerState::SolidCullBack));

	//�V�[���p�萔�o�b�t�@�X�V
	CbScene cbScene{};
	XMMATRIX V = XMLoadFloat4x4(&rc.camera->GetView());
	XMMATRIX P = XMLoadFloat4x4(&rc.camera->GetProjection());
	XMStoreFloat4x4(&cbScene.viewProjection, V * P);

	cbScene.lightViewProjection = rc.shadowMap->GetLightViewProjection();
	cbScene.shadowColor.x = rc.shadowColor.x;
	cbScene.shadowColor.y = rc.shadowColor.y;
	cbScene.shadowColor.z = rc.shadowColor.z;
	cbScene.shadowTexelSize = rc.shadowMap->GetTexelSize();

	cbScene.lightDirection = rc.directionalLightData.direction;
	cbScene.lightColor = rc.directionalLightData.color;

	const XMFLOAT3& eye = rc.camera->GetEye();
	cbScene.cameraPosition.x = eye.x;
	cbScene.cameraPosition.y = eye.y;
	cbScene.cameraPosition.z = eye.z;
	dc->UpdateSubresource(sceneConstantBuffer.Get(), 0, 0, &cbScene, 0, 0);
}

//�`��
void WaterSurfaceShader::Draw(const RenderContext& rc, const Model* model)
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	for (const Model::Mesh& mesh : model->GetMeshes())
	{
		DrawByMesh(rc, model, mesh);
	}
}

void WaterSurfaceShader::DrawByMesh(const RenderContext& rc, const Model* model, const Model::Mesh& mesh)
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	//���_�o�b�t�@�ݒ�
	UINT stride = sizeof(Model::Vertex);
	UINT offset = 0;
	dc->IASetVertexBuffers(0, 1, mesh.vertexBuffer.GetAddressOf(), &stride, &offset);
	dc->IASetIndexBuffer(mesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//���b�V���p�萔�o�b�t�@�X�V
	CbMesh cbMesh{};
	cbMesh.materialColor = mesh.material->color;
	cbMesh.wave1 = mesh.material->waterSurface.waveScroll1;
	cbMesh.wave2 = mesh.material->waterSurface.waveScroll2;
	cbMesh.wave1.x *= rc.timer;
	cbMesh.wave1.y *= rc.timer;
	cbMesh.wave2.x *= rc.timer;
	cbMesh.wave2.y *= rc.timer;

	dc->UpdateSubresource(meshConstantBuffer.Get(), 0, 0, &cbMesh, 0, 0);

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

	//�V�F�[�_�[���\�[�X�r���[�ݒ�
	ID3D11ShaderResourceView* srvs[] =
	{
		mesh.material->diffuseMap.Get(),
		mesh.material->normalMap.Get(),
		rc.shadowMap->GetShaderResourceView(),
	};
	dc->PSSetShaderResources(0, _countof(srvs), srvs);

	//�`��
	dc->DrawIndexed(static_cast<UINT>(mesh.indices.size()), 0, 0);
}

//�`��I��
void WaterSurfaceShader::End(const RenderContext& rc)
{
	ID3D11DeviceContext* dc = rc.deviceContext;
	dc->VSSetShader(nullptr, nullptr, 0);
	dc->PSSetShader(nullptr, nullptr, 0);
	dc->IASetInputLayout(nullptr);

	//�ݒ肳��Ă���V�F�[�_�[���\�[�X���폜
	ID3D11ShaderResourceView* srvs[] = { nullptr,nullptr,nullptr };
	dc->PSSetShaderResources(0, _countof(srvs), srvs);
}
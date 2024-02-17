#include "Misc.h"
#include "Graphics/GpuResourceUtils.h"
#include "WaterSurfaceShader.h"

WaterSurfaceShader::WaterSurfaceShader(ID3D11Device* device)
{
	//入力レイアウト
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT, 0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BONE_WEIGHTS",0,DXGI_FORMAT_R32G32B32A32_FLOAT, 0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"BONE_INDICES",0,DXGI_FORMAT_R32G32B32A32_UINT, 0,D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
		{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"NORMAL",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
		{"TANGENT",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
	};

	//頂点シェーダ
	GpuResourceUtils::LoadVertexShader(
		device,
		"Data/Shader/WaterSurfaceVS.cso",
		inputElementDesc,
		_countof(inputElementDesc),
		inputLayout.GetAddressOf(),
		vertexShader.GetAddressOf());

	//ピクセルシェーダ
	GpuResourceUtils::LoadPixelShader(
		device,
		"Data/Shader/WaterSurfacePS.cso",
		pixelShader.GetAddressOf());

	//シーン用定数バッファ
	GpuResourceUtils::CreateConstantBuffer(
		device,
		sizeof(CbScene),
		sceneConstantBuffer.GetAddressOf());

	//メッシュ用定数バッファ
	GpuResourceUtils::CreateConstantBuffer(
		device,
		sizeof(CbMesh),
		meshConstantBuffer.GetAddressOf());

	//スケルトン用定数バッファ
	GpuResourceUtils::CreateConstantBuffer(
		device,
		sizeof(CbSkeleton),
		skeletonConstantBuffer.GetAddressOf());
}

//描画開始
void WaterSurfaceShader::Begin(const RenderContext& rc)
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	//シェーダ設定
	dc->IASetInputLayout(inputLayout.Get());
	dc->VSSetShader(vertexShader.Get(), nullptr, 0);
	dc->PSSetShader(pixelShader.Get(), nullptr, 0);

	//定数バッファ設定
	ID3D11Buffer* constantBuffers[] =
	{
		sceneConstantBuffer.Get(),
		meshConstantBuffer.Get(),
		skeletonConstantBuffer.Get(),
	};
	dc->VSSetConstantBuffers(0, _countof(constantBuffers), constantBuffers);
	dc->PSSetConstantBuffers(0, _countof(constantBuffers), constantBuffers);

	//サンプラーステート設定
	ID3D11SamplerState* samplerStates[] =
	{
		rc.renderState->GetSamplerState(SamplerState::LinearWrap),
		rc.shadowMap->GetSamplerState(),
	};
	dc->PSSetSamplers(0, _countof(samplerStates), samplerStates);

	//レンダーステート設定
	const float blend_factor[4] = { 1.0f,1.0f,1.0f,1.0f };
	dc->OMSetBlendState(rc.renderState->GetBlendState(BlendState::Transparency), blend_factor, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(rc.renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
	dc->RSSetState(rc.renderState->GetRasterizerState(RasterizerState::SolidCullBack));

	//シーン用定数バッファ更新
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

//描画
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

	//頂点バッファ設定
	UINT stride = sizeof(Model::Vertex);
	UINT offset = 0;
	dc->IASetVertexBuffers(0, 1, mesh.vertexBuffer.GetAddressOf(), &stride, &offset);
	dc->IASetIndexBuffer(mesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//メッシュ用定数バッファ更新
	CbMesh cbMesh{};
	cbMesh.materialColor = mesh.material->color;
	cbMesh.wave1 = mesh.material->waterSurface.waveScroll1;
	cbMesh.wave2 = mesh.material->waterSurface.waveScroll2;
	cbMesh.wave1.x *= rc.timer;
	cbMesh.wave1.y *= rc.timer;
	cbMesh.wave2.x *= rc.timer;
	cbMesh.wave2.y *= rc.timer;

	dc->UpdateSubresource(meshConstantBuffer.Get(), 0, 0, &cbMesh, 0, 0);

	//スケルトン用定数バッファ更新
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

	//シェーダーリソースビュー設定
	ID3D11ShaderResourceView* srvs[] =
	{
		mesh.material->diffuseMap.Get(),
		mesh.material->normalMap.Get(),
		rc.shadowMap->GetShaderResourceView(),
	};
	dc->PSSetShaderResources(0, _countof(srvs), srvs);

	//描画
	dc->DrawIndexed(static_cast<UINT>(mesh.indices.size()), 0, 0);
}

//描画終了
void WaterSurfaceShader::End(const RenderContext& rc)
{
	ID3D11DeviceContext* dc = rc.deviceContext;
	dc->VSSetShader(nullptr, nullptr, 0);
	dc->PSSetShader(nullptr, nullptr, 0);
	dc->IASetInputLayout(nullptr);

	//設定されているシェーダーリソースを削除
	ID3D11ShaderResourceView* srvs[] = { nullptr,nullptr,nullptr };
	dc->PSSetShaderResources(0, _countof(srvs), srvs);
}
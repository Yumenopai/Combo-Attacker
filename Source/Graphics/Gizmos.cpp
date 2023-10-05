#include "Misc.h"
#include "GpuResourceUtils.h"
#include "Gizmos.h"

//�R���X�g���N�^
Gizmos::Gizmos(ID3D11Device* device)
{
	//���̓��C�A�E�g
	D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
	{
		{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
	};

	//���_�V�F�[�_
	GpuResourceUtils::LoadVertexShader(
		device,
		"Data/Shader/GizmosVS.cso",
		inputElementDesc,
		_countof(inputElementDesc),
		inputLayout.GetAddressOf(),
		vertexShader.GetAddressOf());

	//�s�N�Z���V�F�[�_
	GpuResourceUtils::LoadPixelShader(
		device,
		"Data/Shader/GizmosPS.cso",
		pixelShader.GetAddressOf());
	
	//�萔�o�b�t�@
	GpuResourceUtils::CreateConstantBuffer(
		device,
		sizeof(CbMesh),
		constantBuffer.GetAddressOf());

	//�����b�V������
	CreateBoxMesh(device, 0.5f, 0.5f, 0.5f);

	//�����b�V������
	CreateSphereMesh(device, 1.0f, 32);

	//�~�����b�V������
	CreateCylinderMesh(device, 1.0f, 1.0f, 0, 1.0f, 8, 1);
}

//���`��
void Gizmos::DrawBox(const XMFLOAT3& position, const XMFLOAT3& angle, const XMFLOAT3& size, const XMFLOAT4& color)
{
	Instance& instance = instances.emplace_back();
	instance.mesh = &boxMesh;
	instance.color = color;

	XMMATRIX S = XMMatrixScaling(size.x, size.y, size.z);
	XMMATRIX R = XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z);
	XMMATRIX T = XMMatrixTranslation(position.x, position.y, position.z);
	XMStoreFloat4x4(&instance.worldTransform, S * R * T);
}
// ���`��
void Gizmos::DrawSphere(const XMFLOAT3& position, float radius, const XMFLOAT4& color)
{
	Instance& instance = instances.emplace_back();
	instance.mesh = &sphereMesh;
	instance.color = color;

	XMMATRIX S = XMMatrixScaling(radius, radius, radius);
	XMMATRIX T = XMMatrixTranslation(position.x, position.y, position.z);
	XMStoreFloat4x4(&instance.worldTransform, S * T);
}
// �~���`��
void Gizmos::DrawCylinder(const XMFLOAT3& position, float radius, float height, const XMFLOAT3& angle, const XMFLOAT4& color)
{
	Instance& instance = instances.emplace_back();
	instance.mesh = &cylinderMesh;
	instance.color = color;

	XMMATRIX S = XMMatrixScaling(radius, height, radius);
	XMMATRIX R = XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z);
	XMMATRIX T = XMMatrixTranslation(position.x, position.y, position.z);
	XMStoreFloat4x4(&instance.worldTransform, S * R * T);
}

// ���b�V������
void Gizmos::CreateMesh(ID3D11Device* device, const std::vector<XMFLOAT3>& vertices, Mesh& mesh)
{
	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = static_cast<UINT>(sizeof(XMFLOAT3) * vertices.size());
	desc.Usage = D3D11_USAGE_IMMUTABLE;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = 0;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	D3D11_SUBRESOURCE_DATA subresourceData = {};
	subresourceData.pSysMem = vertices.data();
	subresourceData.SysMemPitch = 0;
	subresourceData.SysMemSlicePitch = 0;

	HRESULT hr = device->CreateBuffer(&desc, &subresourceData, mesh.vertexBuffer.GetAddressOf());
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	mesh.vertexCount = static_cast<UINT>(vertices.size());
}

// �����b�V���쐬
void Gizmos::CreateBoxMesh(ID3D11Device* device, float width, float height, float depth)
{
	XMFLOAT3 positions[8] =
	{
		//top
		{-width,height,-depth},
		{ width,height,-depth},
		{ width,height, depth},
		{-width,height, depth},
		//bottom
		{-width,-height,-depth},
		{ width,-height,-depth},
		{ width,-height, depth},
		{-width,-height, depth},
	};

	std::vector<XMFLOAT3> vertices;
	vertices.reserve(32);

	//top
	vertices.emplace_back(positions[0]);
	vertices.emplace_back(positions[1]);
	vertices.emplace_back(positions[1]);
	vertices.emplace_back(positions[2]);
	vertices.emplace_back(positions[2]);
	vertices.emplace_back(positions[3]);
	vertices.emplace_back(positions[3]);
	vertices.emplace_back(positions[0]);
	//bottom
	vertices.emplace_back(positions[4]);
	vertices.emplace_back(positions[5]);
	vertices.emplace_back(positions[5]);
	vertices.emplace_back(positions[6]);
	vertices.emplace_back(positions[6]);
	vertices.emplace_back(positions[7]);
	vertices.emplace_back(positions[7]);
	vertices.emplace_back(positions[4]);
	//side
	vertices.emplace_back(positions[0]);
	vertices.emplace_back(positions[4]);
	vertices.emplace_back(positions[1]);
	vertices.emplace_back(positions[5]);
	vertices.emplace_back(positions[2]);
	vertices.emplace_back(positions[6]);
	vertices.emplace_back(positions[3]);
	vertices.emplace_back(positions[7]);

	//���b�V������
	CreateMesh(device, vertices, boxMesh);
}

// �����b�V���쐬
void Gizmos::CreateSphereMesh(ID3D11Device* device, float radius, int subdivisions)
{
	float step = XM_2PI / subdivisions;

	std::vector<XMFLOAT3> vertices;

	//XZ����
	for (int i = 0; i < subdivisions; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			float theta = step * ((i + j) % subdivisions);

			XMFLOAT3& p = vertices.emplace_back();
			p.x = sinf(theta) * radius;
			p.y = 0.0f;
			p.z = cosf(theta) * radius;
		}
	}

	//XY����
	for (int i = 0; i < subdivisions; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			float theta = step * ((i + j) % subdivisions);

			XMFLOAT3& p = vertices.emplace_back();
			p.x = sinf(theta) * radius;
			p.y = cosf(theta) * radius;
			p.z = 0.0f;
		}
	}

	//YZ����
	for (int i = 0; i < subdivisions; ++i)
	{
		for (int j = 0; j < 2; ++j)
		{
			float theta = step * ((i + j) % subdivisions);

			XMFLOAT3& p = vertices.emplace_back();
			p.x = 0.0f;
			p.y = sinf(theta) * radius;
			p.z = cosf(theta) * radius;
		}
	}

	//���b�V������
	CreateMesh(device, vertices, sphereMesh);
}

// �~�����b�V���쐬
void Gizmos::CreateCylinderMesh(ID3D11Device* device, float radius1, float radius2, float start, float height, int slices, int stacks)
{
	std::vector<XMFLOAT3> vertices;

	float stackHeight = height / stacks;
	float radiusStep = (radius2 - radius1) / stacks;

	// vertices of ring
	float dTheta = XM_2PI / slices;

	for (int i = 0; i < slices; ++i)
	{
		int n = (i + 1) % slices;

		float c1 = cosf(i * dTheta);
		float s1 = sinf(i * dTheta);

		float c2 = cosf(n * dTheta);
		float s2 = sinf(n * dTheta);

		for (int j = 0; j <= stacks; ++j)
		{
			float y = start + j * stackHeight;
			float r = radius1 + j * radiusStep;

			XMFLOAT3& p01 = vertices.emplace_back();
			p01.x = r * c1;
			p01.y = y;
			p01.z = r * s1;
			
			XMFLOAT3& p02 = vertices.emplace_back();
			p02.x = r * c2;
			p02.y = y;
			p02.z = r * s2;
		}

		XMFLOAT3& p11 = vertices.emplace_back();
		p11.x = radius1 * c1;
		p11.y = start;
		p11.z = radius1 * s1;

		XMFLOAT3& p12 = vertices.emplace_back();
		p12.x = radius2 * c1;
		p12.y = start + height;
		p12.z = radius2 * s1;
	}

	//���b�V������
	CreateMesh(device, vertices, cylinderMesh);
}

//�`����s
void Gizmos::Render(const RenderContext& rc)
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	//�V�F�[�_�ݒ�
	dc->VSSetShader(vertexShader.Get(), nullptr, 0);
	dc->PSSetShader(pixelShader.Get(), nullptr, 0);
	dc->IASetInputLayout(inputLayout.Get());

	//�萔�o�b�t�@
	dc->VSSetConstantBuffers(0, 1, constantBuffer.GetAddressOf());

	//�����_�[�X�e�[�g�ݒ�
	const float blendFactor[4] = { 1.0f,1.0f,1.0f,1.0f };
	dc->OMSetBlendState(rc.renderState->GetBlendState(BlendState::Opaque), blendFactor, 0xFFFFFFFF);
	dc->OMSetDepthStencilState(rc.renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
	dc->RSSetState(rc.renderState->GetRasterizerState(RasterizerState::SolidCullNone));

	//�r���[�v���W�F�N�V�����s��쐬
	XMMATRIX V = XMLoadFloat4x4(&rc.camera->GetView());
	XMMATRIX P = XMLoadFloat4x4(&rc.camera->GetProjection());
	XMMATRIX VP = V * P;

	//�v���~�e�B�u�ݒ�
	UINT stride = sizeof(XMFLOAT3);
	UINT offset = 0;
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);

	for (const Instance& instance : instances)
	{
		//���_�o�b�t�@�ݒ�
		dc->IASetVertexBuffers(0, 1, instance.mesh->vertexBuffer.GetAddressOf(), &stride, &offset);

		//���[���h�r���[�v���W�F�N�V�����s��쐬
		XMMATRIX W = XMLoadFloat4x4(&instance.worldTransform);
		XMMATRIX WVP = W * VP;

		//�萔�o�b�t�@�X�V
		CbMesh cbMesh;
		XMStoreFloat4x4(&cbMesh.worldViewProjection, WVP);
		cbMesh.color = instance.color;

		dc->UpdateSubresource(constantBuffer.Get(), 0, 0, &cbMesh, 0, 0);

		//�`��
		dc->Draw(instance.mesh->vertexCount, 0);
	}
	instances.clear();
}

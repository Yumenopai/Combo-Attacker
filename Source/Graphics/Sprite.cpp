#include <fstream>

#include "Sprite.h"
#include "Misc.h"
#include "GpuResourceUtils.h"

//�R���X�g���N�^
Sprite::Sprite(ID3D11Device* device):Sprite(device,nullptr)
{
}

//�R���X�g���N�^
Sprite::Sprite(ID3D11Device* device, const char* filename)
{
	HRESULT hr = S_OK;

	// ���_�f�[�^�̒�`
	// 0           1
	// +-----------+
	// |           |
	// |           |
	// +-----------+
	// 2           3
	Vertex vertices[] = {
		{ DirectX::XMFLOAT3(-0.5, +0.5, 0), DirectX::XMFLOAT4(1, 1, 1, 1) },
		{ DirectX::XMFLOAT3(+0.5, +0.5, 0), DirectX::XMFLOAT4(1, 0, 0, 1) },
		{ DirectX::XMFLOAT3(-0.5, -0.5, 0), DirectX::XMFLOAT4(0, 1, 0, 1) },
		{ DirectX::XMFLOAT3(+0.5, -0.5, 0), DirectX::XMFLOAT4(0, 0, 1, 1) },
	};

	//���_�o�b�t�@����
	{
		//���_�o�b�t�@�쐬�p�̐ݒ�I�v�V����
		D3D11_BUFFER_DESC buffer_desc = {};
		buffer_desc.ByteWidth = sizeof(vertices);
		buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		buffer_desc.MiscFlags = 0;
		buffer_desc.StructureByteStride = 0;
		// ���_�o�b�t�@�ɒ��_�f�[�^�����邽�߂̐ݒ�
		D3D11_SUBRESOURCE_DATA subresource_data = {};
		subresource_data.pSysMem = vertices;	// �����Ɋi�[���������_�f�[�^�̃A�h���X��n�����Ƃ�CreateBuffer()���Ƀf�[�^�����邱�Ƃ��ł���B
		subresource_data.SysMemPitch = 0; //Not use for vertex buffers.
		subresource_data.SysMemSlicePitch = 0; //Not use for vertex buffers.

		//���_�o�b�t�@�I�u�W�F�N�g�̐���
		hr = device->CreateBuffer(&buffer_desc, &subresource_data, vertexBuffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	//���_�V�F�[�_�[
	{
		//���̓��C�A�E�g
		D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
		{
			{"POSITION",0,DXGI_FORMAT_R32G32B32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
			{"COLOR",0,DXGI_FORMAT_R32G32B32A32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
			{"TEXCOORD",0,DXGI_FORMAT_R32G32_FLOAT,0,D3D11_APPEND_ALIGNED_ELEMENT,D3D11_INPUT_PER_VERTEX_DATA,0},
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
	//�s�N�Z���V�F�[�_�[
	{
		hr = GpuResourceUtils::LoadPixelShader(
			device,
			"Data/Shader/SpritePS.cso",
			pixelShader.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	//�e�N�X�`���̐���
	if(filename != nullptr)
	{
		//�e�N�X�`���t�@�C���ǂݍ���
		D3D11_TEXTURE2D_DESC desc;
		hr = GpuResourceUtils::LoadTexture(device, filename, shaderResourceView.GetAddressOf(), &desc);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		textureWidth = static_cast<float>(desc.Width);
		textureHeight = static_cast<float>(desc.Height);
	}
	else
	{
		//�_�~�[�e�N�X�`������
		D3D11_TEXTURE2D_DESC desc;
		hr = GpuResourceUtils::CreateDummyTexture(device, 0xFFFFFFFF, shaderResourceView.GetAddressOf(), &desc);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		textureWidth = static_cast<float>(desc.Width);
		textureHeight = static_cast<float>(desc.Height);
	}
}

//�`����s
void Sprite::Render(ID3D11DeviceContext* dc,
	float dx, float dy,					//����ʒu
	float dz,							//���s
	float dw, float dh,					//�� ����
	float sx, float sy,					//�摜�؂蔲���ʒu
	float sw, float sh,					//�摜�؂蔲���T�C�Y
	float angle,						//�p�x
	DirectX::XMFLOAT4 color				//�F
) const
{
	using namespace DirectX;
	//���_���W
	XMFLOAT2 positions[] =
	{
		XMFLOAT2(dx,		dy),		//����
		XMFLOAT2(dx + dw,	dy),		//�E��
		XMFLOAT2(dx,		dy + dh),	//����
		XMFLOAT2(dx + dw,	dy + dh),	//�E��
	};

	//�e�N�X�`�����W
	XMFLOAT2 texcoords[] =
	{
		XMFLOAT2(sx,		sy),		//����
		XMFLOAT2(sx + sw,	sy),		//�E��
		XMFLOAT2(sx,		sy + sh),	//����
		XMFLOAT2(sx + sw,	sy + sh),	//�E��
	};

	//�X�v���C�g���S�ŉ�]������ׂS���_�̒��S�ʒu�����_(0,0)�ɂȂ�悤��U���_�ړ�
	float mx = dx + dw * 0.5f;
	float my = dy + dh * 0.5f;
	for (auto& p : positions)
	{
		p.x -= mx;
		p.y -= my;
	}

	//���_��]
	float theta = XMConvertToRadians(angle);
	float c = cosf(theta);
	float s = sinf(theta);
	for (auto& p : positions)
	{
		XMFLOAT2 r = p;
		p.x = c * r.x + -s * r.y;
		p.y = s * r.x + c * r.y;
	}

	//��]�p�Ɉړ����������_�����̈ʒu�ɖ߂�
	for (auto& p : positions)
	{
		p.x += mx;
		p.y += my;
	}

	//���ݐݒ蒆�̃r���[�|�[�g����X�N���[���T�C�Y�擾
	D3D11_VIEWPORT viewport;
	UINT numViewports = 1;
	dc->RSGetViewports(&numViewports, &viewport);
	float screenWidth = viewport.Width;
	float screenHeight = viewport.Height;

	//�X�N���[�����W�n����NDC���W�n�֕ϊ�
	for (XMFLOAT2& p : positions)
	{
		p.x = 2.0f * p.x / screenWidth - 1.0f;
		p.y = 1.0f - 2.0f * p.y / screenHeight;
	}

	//���_�o�b�t�@���e�ҏW�J�n
	D3D11_MAPPED_SUBRESOURCE mappedSubresource;
	HRESULT hr = dc->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresource);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	//���_�o�b�t�@���e�ҏW
	Vertex* v = static_cast<Vertex*>(mappedSubresource.pData);
	for (int i = 0; i < 4; ++i)
	{
		v[i].position.x = positions[i].x;
		v[i].position.y = positions[i].y;
		v[i].position.z = dz;

		v[i].color = color;

		v[i].texcoord.x = texcoords[i].x / textureWidth;
		v[i].texcoord.y = texcoords[i].y / textureHeight;
	}

	//���_�o�b�t�@���e�ҏW�I��
	dc->Unmap(vertexBuffer.Get(), 0);

	//GPU�ɕ`��f�[�^���M
	UINT stride = sizeof(Vertex);
	UINT offset = 0;
	dc->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
	dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	dc->IASetInputLayout(inputLayout.Get());
	dc->RSSetState(rasterizerState.Get());
	dc->VSSetShader(vertexShader.Get(), nullptr, 0);
	dc->PSSetShader(pixelShader.Get(), nullptr, 0);
	dc->PSSetShaderResources(0, 1, shaderResourceView.GetAddressOf());

	//�`��
	dc->Draw(4, 0);
}

//�`����s(�e�N�X�`���؂蔲���w��Ȃ�)
void Sprite::Render(ID3D11DeviceContext* dc,
	float dx, float dy,					//����ʒu
	float dz,							//���s
	float dw, float dh,					//�� ����
	float angle,						//�p�x
	DirectX::XMFLOAT4 color				//�F
) const
{
	Render(dc, dx, dy, dz, dw, dh, 0, 0, static_cast<float>(textureWidth), static_cast<float>(textureHeight), angle, color);
}
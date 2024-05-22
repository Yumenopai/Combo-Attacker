#include <fstream>
#include "FontSprite.h"
#include "Misc.h"
#include "GpuResourceUtils.h"

//�R���X�g���N�^
FontSprite::FontSprite(ID3D11Device* device, const char* filename, size_t max_sprites) : max_vertices(max_sprites * 6)
{
	HRESULT hr = S_OK;

	std::unique_ptr<Vertex[]> vertices{ std::make_unique<Vertex[]>(max_vertices) };

	//���_�o�b�t�@�̐��� (���_�f�[�^��GPU�ň������߂̊i�[�p�o�b�t�@)
	{
		//���_�o�b�t�@���쐬���邽�߂̐ݒ�I�v�V����
		D3D11_BUFFER_DESC buffer_desc = {};
		buffer_desc.ByteWidth = static_cast<int>(sizeof(Vertex) * max_vertices);
		//USAGE_DYNAMIC,CPU_ACCESS_WRITE���w�肷�邱�ƂŖ��t���[�����_��ҏW�ł���悤�ɂȂ�
		buffer_desc.Usage = D3D11_USAGE_DYNAMIC;
		buffer_desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		buffer_desc.MiscFlags = 0;
		buffer_desc.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA subresource_data{};
		subresource_data.pSysMem = vertices.get();
		subresource_data.SysMemPitch = 0;
		subresource_data.SysMemSlicePitch = 0;
		//���_�o�b�t�@�I�u�W�F�N�g�̐���
		hr = device->CreateBuffer(&buffer_desc, &subresource_data, vertexBuffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	//���_�V�F�[�_�[
	{
		//���̓��C�A�E�g  GPU�������Ă������_�f�[�^�̓��e�𗝉����邽�߂̂���
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

	//�s�N�Z���V�F�[�_�[
	{
		hr = GpuResourceUtils::LoadPixelShader(
			device,
			"Data/Shader/SpritePS.cso",
			pixelShader.GetAddressOf());

		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	//�e�N�X�`���̐���
	{
		if (filename != nullptr)
		{
			//�e�N�X�`���t�@�C���ǂݍ���
			D3D11_TEXTURE2D_DESC desc;
			hr = GpuResourceUtils::LoadTexture(device, filename, shaderResourceView.GetAddressOf(), &desc); //�e�N�X�`���ǂݍ��ݎ��Ƀe�N�X�`���T�C�Y���擾
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

			textureWidth = static_cast<float>(desc.Width);
			textureHeight = static_cast<float>(desc.Height);
		}
		else
		{
			//�_�~�[�e�N�X�`���𐶐�
			D3D11_TEXTURE2D_DESC desc;
			hr = GpuResourceUtils::CreateDummyTexture(device, 0xFFFFFFFF, shaderResourceView.GetAddressOf(), &desc);
			_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

			textureWidth = static_cast<float>(desc.Width);
			textureHeight = static_cast<float>(desc.Height);
		}
	}
}

//�`����s
void FontSprite::Render(ID3D11DeviceContext * dc,
	float dx, float dy,					 //����ʒu
	float dz,							 //���s
	DirectX::XMFLOAT3 offset,
	float dw, float dh,					 //���A����
	float sx, float sy,					 //�摜�؂蔲���ʒu
	float sw, float sh,					 //�摜�؂蔲���T�C�Y
	float angle,                         //�p�x
	DirectX::XMFLOAT4 color				 //�F
)
{

	dx += offset.x;
	dy += offset.y;
	dz += offset.z;

	//��ʂɕ`�悷��Ƃ��� -1.0 �` 1.0�͈̔͂Ŏw�肷��K�v������B
	//���̋�Ԃ�NDC��ԂƌĂԁB

	//���_���W
	DirectX::XMFLOAT2 positions[] =
	{
		DirectX::XMFLOAT2(dx, dy), //����
		DirectX::XMFLOAT2(dx + dw, dy), //�E��
		DirectX::XMFLOAT2(dx, dy + dh), //����
		DirectX::XMFLOAT2(dx + dw, dy + dh), //�E��
	};

	//�e�N�X�`�����W
	DirectX::XMFLOAT2 texcoords[]{
		DirectX::XMFLOAT2(sx, sy), //����
		DirectX::XMFLOAT2(sx + sw, sy), //�E��
		DirectX::XMFLOAT2(sx, sy + sh), //����
		DirectX::XMFLOAT2(sx + sw, sy + sh), //�E��
	};

	//�X�v���C�g�̒��S�ŉ�]�����邽�߂ɂS���_�̒��S�ʒu��
	//���_(0�A0)�ɂȂ�悤�ɂ������񒸓_���ړ�������
	float mx = dx + dw * 0.5f;
	float my = dy + dh * 0.5f;
	for (auto& p : positions)
	{
		p.x -= mx;
		p.y -= my;
	}

	//���_����]������
	float theta = DirectX::XMConvertToRadians(angle); //�p�x�����W�A���P�ʂ̊p�x(�ʓx�@)�ɕϊ�
	float c = cosf(theta);
	float s = sinf(theta);
	for (auto& p : positions)
	{
		DirectX::XMFLOAT2 r = p;
		//�A�t�B���ϊ�(�g��k���Ɖ�]��g�ݍ��킹���ϊ�)
		p.x = c * r.x + -s * r.y;
		p.y = s * r.x + c * r.y;
	}

	//��]�̂��߂Ɉړ����������_�����̈ʒu�ɖ߂�
	for (auto& p : positions)
	{
		p.x += mx;
		p.y += my;
	}

	//���ݐݒ肳��Ă���r���[�|�[�g����X�N���[���T�C�Y���擾����
	D3D11_VIEWPORT viewport;
	UINT numViewports = 1;
	dc->RSGetViewports(&numViewports, &viewport);
	float screenWidth = viewport.Width;
	float screenHeight = viewport.Height;

	//�X�N���[�����W�n����NDC���W�n�ɕϊ�����
	for (DirectX::XMFLOAT2& p : positions)
	{
		//�@�@�v�Z��(-1.0�`1.0�͈̔͂ɐ��K��)
		// x = (2 * x_screen / screen_width) - 1    ���K�����邽�߂�x���W�𕝂Ŋ����Ēl��0.0�`1.0�͈̔͂ɂȂ�A
		// y = 1 - (2 * y_screen / screen_height)	NDC(-1.0�`1.0)�͈̔͂ɕϊ����邽�߂�x���W��2�������ĕ��Ŋ���(0.0�`2.0)
		//											������1��������(-1.0�`1.0)�͈̔͂ɂ���
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
	//���_�o�b�t�@�̓��e�̕ҏW���J�n����B
	D3D11_MAPPED_SUBRESOURCE mappedSubresouse;
	HRESULT hr = dc->Map(vertexBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedSubresouse);
	_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

	//���_�o�b�t�@�̓��e��ҏW
	size_t vertex_count = vertices.size();
	_ASSERT_EXPR(max_vertices >= vertex_count, "Buffer overflow");

	Vertex* data{ reinterpret_cast<Vertex*>(mappedSubresouse.pData) };
	if (data != nullptr)
	{
		const Vertex* p = vertices.data();
		memcpy_s(data, max_vertices * sizeof(Vertex), p, vertex_count * sizeof(Vertex));
	}

	//���_�o�b�t�@�̓��e�̕ҏW���I������
	dc->Unmap(vertexBuffer.Get(), 0);

	{
		//GPU�ɕ`�悷�邽�߂̃f�[�^��n��
		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		dc->IASetVertexBuffers(0, 1, vertexBuffer.GetAddressOf(), &stride, &offset);
		dc->IASetInputLayout(inputLayout.Get());
		dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//�`��
		dc->Draw(static_cast<UINT>(vertex_count), 0);
	}
}

void FontSprite::Textout(ID3D11DeviceContext* dc, std::string str,
	float dx, float dy,					 //����ʒu
	float dz,							 //���s
	DirectX::XMFLOAT3 offset,
	float dw, float dh,					 //���A����
	float sx, float sy,					 //�摜�؂蔲���ʒu
	float sw, float sh,					 //�摜�؂蔲���T�C�Y
	float angle,						 //�p�x
	DirectX::XMFLOAT4 color)			 //�F
{
	//������`��
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
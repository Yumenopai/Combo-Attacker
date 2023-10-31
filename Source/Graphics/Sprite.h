#pragma once

#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>

//�X�v���C�g
class Sprite
{
public:
	Sprite(ID3D11Device* device);
	Sprite(ID3D11Device* device, const char* filename);

	//���_�f�[�^
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT2 texcoord;
	};

	//�`����s
	void Render(ID3D11DeviceContext* dc,
		float dx, float dy,					//����ʒu
		float dz,							//���s
		float dw, float dh,					//�� ����
		float sx, float sy,					//�摜�؂蔲���ʒu
		float sw, float sh,					//�摜�؂蔲���T�C�Y
		float angle,						//�p�x
		float r, float g, float b, float a	//�F
	) const;

	//�`����s(�e�N�X�`���؂蔲���w��Ȃ�)
	void Render(ID3D11DeviceContext* dc,
		float dx, float dy,					//����ʒu
		float dz,							//���s
		float dw, float dh,					//�� ����
		float angle,						//�p�x
		float r, float g, float b, float a	//�F
	) const;

	// �e�N�X�`�����擾
	float GetTextureWidth() const { return textureWidth; }
	// �e�N�X�`�������擾
	float GetTextureHeight() const { return textureHeight; }

	// ���_�o�b�t�@�̎擾
	const Microsoft::WRL::ComPtr<ID3D11Buffer>& GetVertexBuffer() const { return vertexBuffer; }

	// �V�F�[�_�[���\�[�X�r���[�̎擾
	const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& GetShaderResourceView() const { return shaderResourceView; }

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	inputLayout;

	Microsoft::WRL::ComPtr<ID3D11Buffer>				vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState>		rasterizerState;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	shaderResourceView;

	float textureWidth = 0;
	float textureHeight = 0;
};
#pragma once

#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>
#include "RenderContext.h"

//�X�v���C�g
class FontSprite
{
public:
	FontSprite(ID3D11Device* device, const char* filename, size_t max_sprites);

	//���_�f�[�^
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT2 texcoord;
	};

	const size_t max_vertices;
	std::vector<Vertex> vertices;

	//�`����s
	//void Render(ID3D11DeviceContext* dc) const;
	void Render(ID3D11DeviceContext* dc,
		float dx, float dy,					 //����ʒu
		float dz,							 //���s
		DirectX::XMFLOAT3 offset,
		float dw, float dh,					 //���A����
		float sx, float sy,					 //�摜�؂蔲���ʒu
		float sw, float sh,					 //�摜�؂蔲���T�C�Y
		float angle,						 //�p�x
		DirectX::XMFLOAT4 color				 //�F
	);

	void begin(ID3D11DeviceContext* dc);

	void Textout(ID3D11DeviceContext* dc, std::string str,
		float dx, float dy,					 //����ʒu
		float dz,							 //���s
		DirectX::XMFLOAT3 offset,
		float dw, float dh,					 //���A����
		float sx, float sy,					 //�摜�؂蔲���ʒu
		float sw, float sh,					 //�摜�؂蔲���T�C�Y
		float angle,						 //�p�x
		DirectX::XMFLOAT4 color);			 //�F
	void End(ID3D11DeviceContext* dc);

	//�X�v���C�g�̕����擾
	float GetTextureWidth() { return textureWidth; }

	//�X�v���C�g�̍������擾
	float GetTextureHeight() { return textureHeight; }

	//���_�o�b�t�@�̎擾
	const Microsoft::WRL::ComPtr<ID3D11Buffer>& GetVertexBuffer() const { return vertexBuffer; }

	// �V�F�[�_�[���\�[�X�r���[�̎擾
	const Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& GetShaderResourceView() const { return shaderResourceView; }

private:
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>  inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shaderResourceView;

	float textureWidth = 0;
	float textureHeight = 0;
};
#pragma once

#include <vector>
#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>

class PrimitiveRenderer
{
public:
	PrimitiveRenderer(ID3D11Device* device, const char* filename);

	// ���_�ǉ�
	void AddVertex(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT4& color);

	// �N���A���Z�b�g
	void Clear();

	// ���`��(D3D11_PRIMITIVE_TOPOLOGY_LINELIST)
	void DrawAxis(const DirectX::XMFLOAT4X4& transform, const DirectX::XMFLOAT4& color);

	// �O���b�h�`��(D3D11_PRIMITIVE_TOPOLOGY_LINELIST)
	void DrawGrid(int subdivisions, float scale);

	// �`����s
	void Render(
		ID3D11DeviceContext* dc,
		const DirectX::XMFLOAT4X4& view,
		const DirectX::XMFLOAT4X4& projection,
		D3D11_PRIMITIVE_TOPOLOGY primitiveTopology);

private:
	static const UINT VertexCapacity = 1024;

	struct CbScene
	{
		DirectX::XMFLOAT4X4		viewProjection;
		DirectX::XMFLOAT4		color;
	};

	struct Vertex
	{
		DirectX::XMFLOAT3	position;
		DirectX::XMFLOAT4	color;
		DirectX::XMFLOAT2	texcoord;
	};
	std::vector<Vertex>		vertices;

	Microsoft::WRL::ComPtr<ID3D11VertexShader>	vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>	pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>	inputLayout;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>		constantBuffer;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	shaderResourceView;

	float textureWidth = 0;
	float textureHeight = 0;
};

#pragma once

#include <wrl.h>
#include <d3d11.h>
#include "Graphics/Model.h"

// �O���錾
struct RenderContext;

class ShadowMap
{
public:
	ShadowMap(ID3D11Device* device);
	~ShadowMap() = default;

	//�J�n����
	void Begin(const RenderContext& rc, const XMFLOAT3& position);

	//�`����s
	void Draw(const RenderContext& rc, const Model* model);

	//�I������
	void End(const RenderContext& rc);

	//�V�F�[�_�[���\�[�X�r���[�擾
	ID3D11ShaderResourceView* GetShaderResourceView() const { return shaderResourceView.Get(); }

	//�T���v���[�X�e�[�g�擾
	ID3D11SamplerState* GetSamplerState() const { return samplerState.Get(); }

	//���C�g�r���[�v���W�F�N�V�����s��擾
	const XMFLOAT4X4& GetLightViewProjection() const { return lightViewProjection; }

	//�e�N�Z���T�C�Y�擾
	float GetTexelSize() const { return 1.0f / textureSize; }

private:
	struct CbScene
	{
		XMFLOAT4X4	lightViewProjection;
	};
	struct CbSkeleton
	{
		XMFLOAT4X4	boneTransforms[256];
	};

	const UINT	textureSize = 4096;

	D3D11_VIEWPORT		cachedViewport;

	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	cachedRenderTargetView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	cachedDepthStencilView;

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>	shaderResourceView;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>		depthStencilView;
	
	Microsoft::WRL::ComPtr<ID3D11Buffer>			sceneConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer>			skeletonConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11VertexShader>		vertexShader;
	Microsoft::WRL::ComPtr<ID3D11PixelShader>		pixelShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>		inputLayout;

	Microsoft::WRL::ComPtr<ID3D11SamplerState>		samplerState;
	XMFLOAT4X4										lightViewProjection;
};
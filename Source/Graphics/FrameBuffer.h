#pragma once

#include <wrl.h>
#include <d3d11.h>

class FrameBuffer
{
public:
	FrameBuffer(ID3D11Device* device, IDXGISwapChain* swapchain);
	FrameBuffer(ID3D11Device* device, UINT width, UINT height);

	//�J���[�}�b�v�擾
	ID3D11ShaderResourceView* GetColorMap() const { return colorMap.Get(); }

	//�N���A
	void Clear(ID3D11DeviceContext* dc, float r, float g, float b, float a);

	//�����_�[�^�[�Q�b�g�ݒ�
	void SetRenderTarget(ID3D11DeviceContext* dc);

private:
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	renderTargetView;
	D3D11_VIEWPORT									viewport;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	depthStencilView;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>colorMap;

};
#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <memory>
#include <mutex>

#include "FrameBuffer.h"
#include "RenderState.h"
#include "Gizmos.h"
#include "PrimitiveRenderer.h"
#include "Shader/Shader.h"
#include "Shader/ShadowMap.h"

enum class ShaderId
{
	Phong,
	Toon,
	WaterSurface,

	EnumCount
};
enum class SShaderId
{

	EnumCount
};

enum class FrameBufferId
{
	Display,
	Scene,
	Luminance,

	EnumCount
};


//�O���t�B�b�N�X
class Graphics
{
private:
	Graphics() = default;
	~Graphics() = default;

public:
	//�C���X�^���X�擾
	static Graphics& Instance()
	{
		static Graphics instance;
		return instance;
	}

	//������
	void Initialize(HWND hWnd);

	//��ʕ\��
	void Present(UINT syncInterval);

	//�f�o�C�X�擾
	ID3D11Device* GetDevice() { return device.Get(); }

	//�f�o�C�X�R���e�L�X�g�擾
	ID3D11DeviceContext* GetDeviceContext() { return immediateContext.Get(); }

	// �����_�[�^�[�Q�b�g�r���[�擾
	ID3D11RenderTargetView* GetRenderTargetView() const { return renderTargetView.Get(); }

	// �f�v�X�X�e���V���r���[�擾
	ID3D11DepthStencilView* GetDepthStencilView() const { return depthStencilView.Get(); }

	//�X�N���[�����擾
	float GetScreenWidth() const { return screenWidth; }

	//�X�N���[�������擾
	float GetScreenHeight() const { return screenHeight; }

	//�t���[���o�b�t�@�擾
	FrameBuffer* GetFrameBuffer(FrameBufferId frameBufferId)
	{
		return frameBuffers[static_cast<int>(frameBufferId)].get();
	}

	//�����_�[�X�e�[�g�擾
	RenderState* GetRenderState() { return renderState.get(); }

	//�M�Y���擾
	Gizmos* GetGizmos() { return gizmos.get(); }

	//�V�F�[�_�擾
	ModelShader* GetShader(ShaderId shaderId) { return shaders[static_cast<int>(shaderId)].get(); }

	//�V���h�E�}�b�v�擾
	ShadowMap* GetShadowMap() { return shadowMap.get(); }

	// �v���~�e�B�u�����_���擾
	PrimitiveRenderer* GetPrimitiveRenderer() const { return primitiveRenderer.get(); }

	//�~���[�e�b�N�X�擾
	std::mutex& GetMutex() { return mutex; }

private:
	Microsoft::WRL::ComPtr<ID3D11Device>		device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> immediateContext;
	Microsoft::WRL::ComPtr<IDXGISwapChain>		swapchain;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView>	renderTargetView;
	Microsoft::WRL::ComPtr<ID3D11Texture2D>			depthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView>	depthStencilView;

	float screenWidth;
	float screenHeight;

	std::unique_ptr<FrameBuffer> frameBuffers[static_cast<int>(FrameBufferId::EnumCount)];
	std::unique_ptr<RenderState> renderState;
	std::unique_ptr<Gizmos>		 gizmos;
	std::unique_ptr<ModelShader>		 shaders[static_cast<int>(ShaderId::EnumCount)];
	std::unique_ptr<ShadowMap>	 shadowMap;
	std::unique_ptr<PrimitiveRenderer>				primitiveRenderer;

	std::mutex mutex;
};
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


//グラフィックス
class Graphics
{
private:
	Graphics() = default;
	~Graphics() = default;

public:
	//インスタンス取得
	static Graphics& Instance()
	{
		static Graphics instance;
		return instance;
	}

	//初期化
	void Initialize(HWND hWnd);

	//画面表示
	void Present(UINT syncInterval);

	//デバイス取得
	ID3D11Device* GetDevice() { return device.Get(); }

	//デバイスコンテキスト取得
	ID3D11DeviceContext* GetDeviceContext() { return immediateContext.Get(); }

	// レンダーターゲットビュー取得
	ID3D11RenderTargetView* GetRenderTargetView() const { return renderTargetView.Get(); }

	// デプスステンシルビュー取得
	ID3D11DepthStencilView* GetDepthStencilView() const { return depthStencilView.Get(); }

	//スクリーン幅取得
	float GetScreenWidth() const { return screenWidth; }

	//スクリーン高さ取得
	float GetScreenHeight() const { return screenHeight; }

	//フレームバッファ取得
	FrameBuffer* GetFrameBuffer(FrameBufferId frameBufferId)
	{
		return frameBuffers[static_cast<int>(frameBufferId)].get();
	}

	//レンダーステート取得
	RenderState* GetRenderState() { return renderState.get(); }

	//ギズモ取得
	Gizmos* GetGizmos() { return gizmos.get(); }

	//シェーダ取得
	ModelShader* GetShader(ShaderId shaderId) { return shaders[static_cast<int>(shaderId)].get(); }

	//シャドウマップ取得
	ShadowMap* GetShadowMap() { return shadowMap.get(); }

	// プリミティブレンダラ取得
	PrimitiveRenderer* GetPrimitiveRenderer() const { return primitiveRenderer.get(); }

	//ミューテックス取得
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
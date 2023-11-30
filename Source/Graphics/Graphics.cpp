#include "Misc.h"
#include "Graphics.h"
#include "Shader/PhongShader.h"
#include "Shader/ToonShader.h"
#include "Shader/WaterSurfaceShader.h"

//初期化
void Graphics::Initialize(HWND hWnd)
{
	// ADAPTER
	IDXGIFactory* factory;
	CreateDXGIFactory(IID_PPV_ARGS(&factory));
	IDXGIAdapter* adapter;
	for (UINT adapter_index = 0; S_OK == factory->EnumAdapters(adapter_index, &adapter); ++adapter_index) {
		DXGI_ADAPTER_DESC adapter_desc;
		adapter->GetDesc(&adapter_desc);
		if (adapter_desc.VendorId == 0x1002/*AMD*/ || adapter_desc.VendorId == 0x10DE/*NVIDIA*/)
		{
			break;
		}
		adapter->Release();
	}
	if (adapter == nullptr)
	{
		factory->EnumAdapters(0, &adapter);
		DXGI_ADAPTER_DESC adapter_desc;
		adapter->GetDesc(&adapter_desc);
	}

	//画面サイズ取得
	RECT rc;
	GetClientRect(hWnd, &rc);
	UINT screenWidth = rc.right - rc.left;
	UINT screenHeight = rc.bottom - rc.top;

	this->screenWidth = static_cast<float>(screenWidth);
	this->screenHeight = static_cast<float>(screenHeight);

	HRESULT hr = S_OK;

	//デバイス＆スワップチェーン生成
	{
		UINT createDeviceFlags = 0;
#if defined(DEBUG) || defined(_DEBUG)
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1,
		};

		//スワップチェーン作成のための設定オプション
		DXGI_SWAP_CHAIN_DESC swapchainDesc;
		{
			swapchainDesc.BufferDesc.Width = screenWidth;
			swapchainDesc.BufferDesc.Height = screenHeight;
			swapchainDesc.BufferDesc.RefreshRate.Numerator = 60;
			swapchainDesc.BufferDesc.RefreshRate.Denominator = 1;
			swapchainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			swapchainDesc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			swapchainDesc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
			swapchainDesc.SampleDesc.Count = 1;
			swapchainDesc.SampleDesc.Quality = 0;
			swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			swapchainDesc.BufferCount = 1;
			swapchainDesc.OutputWindow = hWnd;
			swapchainDesc.Windowed = TRUE;
			swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
			swapchainDesc.Flags = 0;
		}

		D3D_FEATURE_LEVEL featureLevel;

		//デバイス＆スワップチェーン生成
		hr = D3D11CreateDeviceAndSwapChain(
			adapter/*ADAPTER*/,
			D3D_DRIVER_TYPE_UNKNOWN/*ADAPTER*/,
			nullptr,
			createDeviceFlags,
			featureLevels,
			ARRAYSIZE(featureLevels),
			D3D11_SDK_VERSION,
			&swapchainDesc,
			swapchain.GetAddressOf(),
			device.GetAddressOf(),
			&featureLevel,
			immediateContext.GetAddressOf()
		);
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	adapter->Release();

	// レンダーターゲットビューの生成
	{
		// スワップチェーンからバックバッファテクスチャを取得する。
		// ※スワップチェーンに内包されているバックバッファテクスチャは'色'を書き込むテクスチャ。
		Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
		hr = swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		// バックバッファテクスチャへの書き込みの窓口となるレンダーターゲットビューを生成する。
		hr = device->CreateRenderTargetView(backBuffer.Get(), nullptr, renderTargetView.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}
	// 深度ステンシルビューの生成
	{
		// 深度ステンシル情報を書き込むためのテクスチャを作成する。
		D3D11_TEXTURE2D_DESC depthStencilBufferDesc;
		depthStencilBufferDesc.Width = screenWidth;
		depthStencilBufferDesc.Height = screenHeight;
		depthStencilBufferDesc.MipLevels = 1;
		depthStencilBufferDesc.ArraySize = 1;
		depthStencilBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;	// 1ピクセルあたり、深度情報を24Bit / ステンシル情報を8bitのテクスチャを作成する。
		depthStencilBufferDesc.SampleDesc.Count = 1;
		depthStencilBufferDesc.SampleDesc.Quality = 0;
		depthStencilBufferDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;		// 深度ステンシル用のテクスチャを作成する。
		depthStencilBufferDesc.CPUAccessFlags = 0;
		depthStencilBufferDesc.MiscFlags = 0;
		hr = device->CreateTexture2D(&depthStencilBufferDesc, nullptr, depthStencilBuffer.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));

		// 深度ステンシルテクスチャへの書き込みに窓口になる深度ステンシルビューを作成する。
		hr = device->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr, depthStencilView.GetAddressOf());
		_ASSERT_EXPR(SUCCEEDED(hr), HRTrace(hr));
	}

	//フレームバッファ作成
	frameBuffers[static_cast<int>(FrameBufferId::Display)] = std::make_unique<FrameBuffer>(device.Get(), swapchain.Get());
	frameBuffers[static_cast<int>(FrameBufferId::Scene)] = std::make_unique<FrameBuffer>(device.Get(), screenWidth, screenHeight);
	frameBuffers[static_cast<int>(FrameBufferId::Luminance)] = std::make_unique<FrameBuffer>(device.Get(), screenWidth, screenHeight);

	//レンダーステート生成
	renderState = std::make_unique<RenderState>(device.Get());
	// レンダラ生成
	primitiveRenderer = std::make_unique<PrimitiveRenderer>(device.Get(),"Data/Texture/swoosh.jpg");
	//primitiveRenderer = std::make_unique<PrimitiveRenderer>(device.Get(),"Data/Texture/swoosh.png");

	//ギズモ生成
	gizmos = std::make_unique<Gizmos>(device.Get());

	//シェーダ生成
	shaders[static_cast<int>(ShaderId::Phong)] = std::make_unique<PhongShader>(device.Get());
	shaders[static_cast<int>(ShaderId::Toon)] = std::make_unique<ToonShader>(device.Get());
	shaders[static_cast<int>(ShaderId::WaterSurface)] = std::make_unique<WaterSurfaceShader>(device.Get());

	//シャドウマップ生成
	shadowMap = std::make_unique<ShadowMap>(device.Get());
}
//画面表示
void Graphics::Present(UINT syncInterval)
{
	swapchain->Present(syncInterval, 0);
}
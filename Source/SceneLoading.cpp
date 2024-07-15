#include "Graphics/Graphics.h"
#include "Input/Input.h"

#include "SceneManager.h"
#include "SceneLoading.h"


void SceneLoading::Initialize()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	sprite[0] = std::make_unique<Sprite>(device, "Data/Sprite/loadingIcon.png");
	sprite[1] = std::make_unique<Sprite>(device, "Data/Sprite/loadingSprite.png");

	font = std::make_unique<FontSprite>(device, "Data/Font/font6.png", 256);

	//thread start
	thread = std::make_unique<std::thread>(LoadingThread, this);
	timer = 0.0f;
}

void SceneLoading::Finalize()
{
	//thread end
	if (thread != nullptr)
	{
		thread->join();	//なくす前に待っておく
	}
}

void SceneLoading::Update(float elapsedTime)
{
	timer += elapsedTime;

	constexpr float speed = 180;
	angle += speed * elapsedTime;

	//ready=true sceneChange
	if (nextScene->IsReady() && timer > 1.7f)
	{
		SceneManager::Instance().ChangeScene(nextScene);
		nextScene = nullptr;
	}
}

void SceneLoading::Render()
{
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();
	ID3D11RenderTargetView* rtv = graphics.GetRenderTargetView();
	ID3D11DepthStencilView* dsv = graphics.GetDepthStencilView();

	// 画面クリア＆レンダーターゲット設定
	FLOAT color[] = { 0.0f, 0.0f, 0.5f, 1.0f };	// RGBA(0.0～1.0)
	dc->ClearRenderTargetView(rtv, color);
	dc->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	dc->OMSetRenderTargets(1, &rtv, dsv);

	RenderContext rc;
	//描画コンテキスト設定
	rc.deviceContext = Graphics::Instance().GetDeviceContext();
	rc.renderState = Graphics::Instance().GetRenderState();
	rc.shadowColor = { 0.5f,0.5f,0.5f };

	//2DSprite
	{
		float texture1Width = static_cast<float>(sprite[1]->GetTextureWidth());
		float texture1Height = static_cast<float>(sprite[1]->GetTextureHeight());
		sprite[1]->Render(dc, 0.0f, 0.0f, 0.0f, texture1Width, texture1Height,
			0, 0, texture1Width, texture1Height, 0.0f, { 1, 1, 1, 1 });

		float screenWidth = static_cast<float>(graphics.GetScreenWidth());
		float screenHeight = static_cast<float>(graphics.GetScreenHeight());
		float texture0Width = static_cast<float>(sprite[0]->GetTextureWidth());
		float texture0Height = static_cast<float>(sprite[0]->GetTextureHeight());

		float positionX = screenWidth - texture0Width;
		float positionY = screenHeight - texture0Height;

		sprite[0]->Render(dc, positionX, positionY, 0.0f, texture0Width, texture0Height,
			0, 0, texture0Width, texture0Height, angle, { 1, 1, 1, 1 });
	}
}

void SceneLoading::LoadingThread(SceneLoading* scene)
{
	//COM関連の初期化でスレッド毎に呼ぶ必要がある
	CoInitialize(nullptr);

	//NextScene init
	scene->nextScene->Initialize();

	//スレッドが終わる前にCOM関連の終了化
	CoUninitialize();

	//NextScene ReadySetting
	scene->nextScene->SetReady();
}

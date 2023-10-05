#include "Graphics/Graphics.h"
#include "EffectManager.h"

void EffectManager::Initialize()
{
	Graphics& graphics = Graphics::Instance();

	effekseerRenderer = EffekseerRendererDX11::Renderer::Create(
		graphics.GetDevice(), graphics.GetDeviceContext(), 2048);

	effekseerManager = Effekseer::Manager::Create(2048);

	//rendererの各種設定
	effekseerManager->SetSpriteRenderer(effekseerRenderer->CreateSpriteRenderer());
	effekseerManager->SetRibbonRenderer(effekseerRenderer->CreateRibbonRenderer());
	effekseerManager->SetRingRenderer(effekseerRenderer->CreateRingRenderer());
	effekseerManager->SetTrackRenderer(effekseerRenderer->CreateTrackRenderer());
	effekseerManager->SetModelRenderer(effekseerRenderer->CreateModelRenderer());

	//Effekseer内でのローダーの設定
	effekseerManager->SetTextureLoader(effekseerRenderer->CreateTextureLoader());
	effekseerManager->SetModelLoader(effekseerRenderer->CreateModelLoader());
	effekseerManager->SetMaterialLoader(effekseerRenderer->CreateMaterialLoader());

	//左手系座標で計算する
	effekseerManager->SetCoordinateSystem(Effekseer::CoordinateSystem::LH);
}

void EffectManager::Finalize()
{
	//スマートポインタによって破棄されるので何もしない
}

void EffectManager::Update(float elapsedTime)
{
	effekseerManager->Update(elapsedTime * 60.0f);
}

void EffectManager::Render(const XMFLOAT4X4& view, const XMFLOAT4X4& projection)
{
	effekseerRenderer->SetCameraMatrix(*reinterpret_cast<const Effekseer::Matrix44*>(&view));
	effekseerRenderer->SetProjectionMatrix(*reinterpret_cast<const Effekseer::Matrix44*>(&projection));

	effekseerRenderer->BeginRendering();

	//マネージャー単位で描画するので描画順を制御する場合はマネージャーを複数個作成し、
	//Draw()関数を実行する順序で制御できそう
	effekseerManager->Draw();

	effekseerRenderer->EndRendering();
}

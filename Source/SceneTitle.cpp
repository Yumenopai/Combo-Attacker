#include "Graphics/Graphics.h"
#include "Light/LightManager.h"
#include "SceneManager.h"
#include "SceneTitle.h"
#include "SceneGame.h"
#include "SceneLoading.h"

#include "SceneClear.h"

#include "Input/Input.h"

void SceneTitle::Initialize()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	sprite = std::make_unique<Sprite>(device, "Data/Sprite/logo.png");
	model = std::make_unique<Model>(device, "Data/Model/SD-UnityChan/UnityChan.fbx", 0.2f);
	model->PlayAnimation(static_cast<int>(Player::Animation::Idle), true);

	stage = std::make_unique<Stage>();

	float screenWidth = Graphics::Instance().GetScreenWidth();
	float screenHeight = Graphics::Instance().GetScreenHeight();

	Camera& camera = Camera::Instance();
	//カメラ設定
	camera.SetPerspectiveFov(
		XMConvertToRadians(45),		//画角
		screenWidth / screenHeight,	//画面アスペクト比
		0.1f,
		100.0f
	);
	camera.SetLookAt(
		XMFLOAT3(0, 30, -80),	//視点
		XMFLOAT3(0, -38, 0),		//注視点
		XMFLOAT3(0, 1, 0)		//上ベクトル
	);

	// 平行光源を追加
	mainDirectionalLight = new Light(LightType::Directional);
	mainDirectionalLight->SetDirection({ 2, -2, 2 });
	LightManager::Instane().Register(mainDirectionalLight);
}

void SceneTitle::Finalize()
{
}

void SceneTitle::Update(float elapsedTime)
{
	GamePad& gamePad = Input::Instance().GetGamePad();

	const GamePadButton anyButton = GamePad::BTN_A | GamePad::BTN_B | GamePad::BTN_X | GamePad::BTN_Y | GamePad::BTN_START;
	if (gamePad.GetButtonDown() & anyButton)
	{
		SceneManager::Instance().ChangeScene(new SceneLoading(new SceneGame(), -255));
	}

	//if (isScaleDown) scaleTimer -= 0.03f;
	//else scaleTimer += 0.03f;
	//scale.x = scale.y = scale.z = sinf(scaleTimer);
	{
		//ワールド行列計算
		XMMATRIX S = XMMatrixScaling(scale.x, scale.y, scale.z);
		XMMATRIX R = XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z);
		XMMATRIX T = XMMatrixTranslation(-20, 0.0f, -50);
		XMFLOAT4X4 worldTramsform;
		XMStoreFloat4x4(&worldTramsform, S * R * T);

		//モデルアニメーション更新処理
		model->UpdateAnimation(elapsedTime);

		//モデル行列更新
		model->UpdateTransform(worldTramsform);
	}
	stage->Update(elapsedTime);

	timer += elapsedTime;
	if (timer >= 2.0f) timer = 0.0f;
}

void SceneTitle::Render()
{
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();
	Camera& camera = Camera::Instance();
	RenderContext rc;
	ShadowMap* shadowMap = Graphics::Instance().GetShadowMap();

	rc.timer = ++waterTimer;

	//カメラ更新処理
	rc.view = camera.GetView();
	rc.projection = camera.GetProjection();

	//描画コンテキスト設定
	rc.camera = &camera;
	rc.deviceContext = Graphics::Instance().GetDeviceContext();
	rc.renderState = Graphics::Instance().GetRenderState();
	LightManager::Instane().PushRenderContext(rc);// ライトの情報を詰め込む
	rc.shadowMap = shadowMap;
	rc.shadowColor = { 0.5f,0.5f,0.5f };

	//シャドウマップ描画
	shadowMap->Begin(rc, camera.GetFocus());
	//シャドウマップにモデル描画
	stage->ShadowRender(rc, shadowMap);
	shadowMap->Draw(rc, model.get());
	shadowMap->End(rc);

	RenderState* renderState = Graphics::Instance().GetRenderState();

	ID3D11SamplerState* samplers[] =
	{
		renderState->GetSamplerState(SamplerState::PointClamp)
	};
	dc->PSSetSamplers(0, _countof(samplers), samplers);

	// 深度テストなし＆深度書き込みなし
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::NoTestNoWrite), 0);


	ID3D11RenderTargetView* rtv = graphics.GetRenderTargetView();
	ID3D11DepthStencilView* dsv = graphics.GetDepthStencilView();

	// 画面クリア＆レンダーターゲット設定
	FLOAT color[] = { 0.0f, 0.0f, 0.5f, 1.0f };	// RGBA(0.0～1.0)
	dc->ClearRenderTargetView(rtv, color);
	dc->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	dc->OMSetRenderTargets(1, &rtv, dsv);

	//3DModel
	{
		ModelShader* shader = Graphics::Instance().GetShader(ShaderId::Toon);
		shader->Begin(rc);
		//シェーダーにモデル描画
		stage->TerrainRender(rc, shader);
		shader->Draw(rc, model.get());
		shader->End(rc);

		ModelShader* waterShader = Graphics::Instance().GetShader(ShaderId::WaterSurface);
		waterShader->Begin(rc);
		stage->WaterRender(rc, waterShader);
		waterShader->End(rc);
	}
	//2DSprite
	{
		float screenWidth = static_cast<float>(graphics.GetScreenWidth());
		float screenHeight = static_cast<float>(graphics.GetScreenHeight());

		FLOAT blendFactor[4] = { 1.0f,1.0f,1.0f,1.0f };
		UINT sampleMask = 0xFFFFFFFF;

		//titleSprite
		dc->OMSetBlendState(renderState->GetBlendState(BlendState::Transparency), blendFactor, sampleMask);
		sprite->Render(dc, 0.0f, 60.0f, 0.0f, screenWidth, 120, 0, 0, screenWidth, 120, 0, { 1, 1, 1, 1 });
		if (timer <= 1.6f)
			sprite->Render(dc, 0.0f, 480.0f, 0.0f, screenWidth, 120, 0, 120, screenWidth, 120, 0, { 1, 1, 1, 1 });
	}

}

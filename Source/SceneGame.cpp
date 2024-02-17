#include <functional>
#include "Graphics/Graphics.h"
#include "Graphics/FontSprite.h"
#include "SceneManager.h"
#include "SceneTitle.h"
#include "SceneLoading.h"
#include "SceneGame.h"
#include "PlayerManager.h"
#include "EnemyManager.h"
#include "EnemySlime.h"
#include "EnemyTurtleShell.h"
#include "EnemyBlue.h"
#include "TransformUtils.h"
#include "Light/LightManager.h"
#include "EffectManager.h"
#include "Input/Input.h"

#include "imgui.h"

// 初期化
void SceneGame::Initialize()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	float screenWidth = Graphics::Instance().GetScreenWidth();
	float screenHeight = Graphics::Instance().GetScreenHeight();

	Camera& camera = Camera::Instance();
	//カメラ設定
	camera.SetPerspectiveFov(
		XMConvertToRadians(45),		//画角
		screenWidth / screenHeight,	//画面アスペクト比
		0.1f,
		1000.0f
	);
	camera.SetLookAt(
		XMFLOAT3(0, 10, -10),	//視点
		XMFLOAT3(0, 0, 0),		//注視点
		XMFLOAT3(0, 1, 0)		//上ベクトル
	);
	//freeCameraController.SyncCameraToController(camera);

	cameraController = std::make_unique<CameraController>();

	//モデル作成
	stage = std::make_unique<Stage>();
	player1P = std::make_unique<Player1P>();
	playerAI = std::make_unique<PlayerAI>();

	for (int i = 0; i < enemySlimeCount; i++)
	{
		enemySlime[i] = std::make_unique<EnemySlime>();
	}
	for (int i = 0; i < enemyTurtleShellCount; i++)
	{
		enemyTurtleShell[i] = std::make_unique<EnemyTurtleShell>();
	}
	enemyBlue = std::make_unique<EnemyBlue>();

	PlayerManager& playerManager = PlayerManager::Instance();
	playerManager.Register(player1P.get());
	playerManager.Register(playerAI.get());

	//スカイボックス
	skyBox = std::make_unique<SkyBox>("Data/Texture/incskies_002_8k.png");

	//sprites[1] = std::make_unique<Sprite>(device);
	sprites[0] = std::make_unique<Sprite>(device, "Data/Sprite/button.png");
	sprites[1] = std::make_unique<Sprite>(device, "Data/Sprite/item.png");
	sprites[2] = std::make_unique<Sprite>(device, "Data/Sprite/item2.png");
	sprites[3] = std::make_unique<Sprite>(device, "Data/Sprite/weapon.png");
	font = std::make_unique<FontSprite>(device, "Data/Font/font6.png", 256);
	//ゲージスプライト
	gauge = std::make_unique<Sprite>(device);

	//エネミー初期化
	enemySlime[0]->SetPosition(DirectX::XMFLOAT3(30, 5, -43));
	enemySlime[1]->SetPosition(DirectX::XMFLOAT3(28, 5, -21));
	enemySlime[2]->SetPosition(DirectX::XMFLOAT3(30, 5, -23));
	enemySlime[3]->SetPosition(DirectX::XMFLOAT3(-28, 5, 4));

	enemyTurtleShell[0]->SetPosition(DirectX::XMFLOAT3(-26, 5, 6));
	enemyTurtleShell[1]->SetPosition(DirectX::XMFLOAT3(28, 5, 0));

	enemyBlue->SetPosition(DirectX::XMFLOAT3(-28, 5, -12));

	EnemyManager& enemyManager = EnemyManager::Instance();
	for (int i = 0; i < enemySlimeCount; i++)
	{
		enemyManager.Register(enemySlime[i].get());
	}
	for (int i = 0; i < enemyTurtleShellCount; i++)
	{
		enemyManager.Register(enemyTurtleShell[i].get());
	}

	enemyManager.Register(enemyBlue.get());

	// 平行光源を追加
	mainDirectionalLight = new Light(LightType::Directional);
	mainDirectionalLight->SetDirection({ 1, -1, -2 });
	LightManager::Instane().Register(mainDirectionalLight);
}

// 終了化
void SceneGame::Finalize()
{
	//エネミー終了化
	EnemyManager::Instance().Clear();
}

void SceneGame::Update(float elapsedTime)
{
	//カメラコントローラー更新処理
	XMFLOAT3 target = player1P->GetPosition();
	target.y += 0.5f;
	cameraController->SetTarget(target);
	cameraController->Update(elapsedTime);

	//カメラ更新処理
	//Camera& camera = Camera::Instance();
	//freeCameraController.Update();
	//freeCameraController.SyncControllerToCamera(camera);

	// マネージャーによる更新
	PlayerManager::Instance().Update(elapsedTime);
	EnemyManager::Instance().Update(elapsedTime);
	stage->Update(elapsedTime);
	//エフェクト更新処理
	EffectManager::Instance().Update(elapsedTime);

	GamePad& gamePad = Input::Instance().GetGamePad();

	//if (gamePad.GetButtonDown() & GamePad::BTN_START)
	//{
	//	SceneManager::Instance().ChangeScene(new SceneTitle());
	//}
}

void SceneGame::Render()
{
	Camera& camera = Camera::Instance();
	ShadowMap* shadowMap = Graphics::Instance().GetShadowMap();
	
	RenderContext rc;
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
	stage->ShadowRender(rc, shadowMap);
	PlayerManager::Instance().ShadowRender(rc, shadowMap);
	EnemyManager::Instance().ShadowRender(rc, shadowMap);
	shadowMap->End(rc);

	//2d
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = rc.deviceContext;
	RenderState* renderState = Graphics::Instance().GetRenderState();

	ID3D11SamplerState* samplers[] =
	{
		renderState->GetSamplerState(SamplerState::LinearClamp)
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

	float screenWidth = static_cast<float>(graphics.GetScreenWidth());
	float screenHeight = static_cast<float>(graphics.GetScreenHeight());
	// ビューポートの設定
	D3D11_VIEWPORT vp = {};
	vp.Width = graphics.GetScreenWidth();
	vp.Height = graphics.GetScreenHeight();
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	dc->RSSetViewports(1, &vp);

	// スカイボックスの描画
	skyBox->Render(rc);

	ModelShader* shader = Graphics::Instance().GetShader(ShaderId::Toon);
	shader->Begin(rc);
	player1P->Render(rc, shader);
	stage->TerrainRender(rc, shader);
	EnemyManager::Instance().Render(rc, shader);
	shader->End(rc);

	RenderContext AIrc = rc;
	AIrc.shadowColor = { 0.7f,0.1f,0.1f };
	shader->Begin(AIrc);
	playerAI->Render(AIrc, shader);
	shader->End(AIrc);

	ModelShader* waterShader = Graphics::Instance().GetShader(ShaderId::WaterSurface);
	waterShader->Begin(rc);
	stage->WaterRender(rc, waterShader);
	waterShader->End(rc);

	//3Dエフェクト描画
	EffectManager::Instance().Render(rc.view, rc.projection);

	// 2Dスプライト描画
	{
		PlayerManager::Instance().Render2d(rc, gauge.get());
		RenderCharacterName(rc, rc.view, rc.projection);

		RenderEnemyGauge(dc, rc.view, rc.projection);
	}
	//2DSprite
	{
		float screenWidth = static_cast<float>(graphics.GetScreenWidth());
		float screenHeight = static_cast<float>(graphics.GetScreenHeight());
		float textureWidth = static_cast<float>(sprites[0]->GetTextureWidth());
		float textureHeight = static_cast<float>(sprites[0]->GetTextureHeight());

		FLOAT blendFactor[4] = { 1.0f,1.0f,1.0f,1.0f };
		UINT sampleMask = 0xFFFFFFFF;

		//titleSprite
		dc->OMSetBlendState(renderState->GetBlendState(BlendState::Transparency), blendFactor, sampleMask);

		//Item枠
		for (int i = 0; i < 6; i++)
		{
			sprites[2]->Render(dc, 300.0f + 115*i, 610.0f, 0.0f, 100, 100, 0, 0, textureWidth, textureWidth, 0, 1, 1, 1, 1);
		}
		//AttackButton
		sprites[0]->Render(dc, 1150.0f, 250.0f, 0.0f, 80, 80, 0, 0, textureWidth, textureWidth, 0, 1, 1, 1, 1);
		sprites[0]->Render(dc, 1100.0f, 300.0f, 0.0f, 80, 80, 0, 0, textureWidth, textureWidth, 0, 1, 1, 1, 1);
		sprites[0]->Render(dc, 1200.0f, 300.0f, 0.0f, 80, 80, 0, 0, textureWidth, textureWidth, 0, 1, 1, 1, 1);
		sprites[1]->Render(dc, 1150.0f, 350.0f, 0.0f, 80, 80, 0, 0, textureWidth, textureWidth, 0, 1, 1, 1, 1);
		sprites[3]->Render(dc, 1150.0f+15.0f, 250.0f+10.0f, 0.0f, 50, 50, 900, 0, textureWidth, textureWidth, 0, 1, 1, 1, 1);
		sprites[3]->Render(dc, 1100.0f+15.0f, 300.0f+10.0f, 0.0f, 50, 50, 300, 0, textureWidth, textureWidth, 0, 1, 1, 1, 1);
		sprites[3]->Render(dc, 1200.0f+15.0f, 300.0f+10.0f, 0.0f, 50, 50, 600, 0, textureWidth, textureWidth, 0, 1, 1, 1, 1);
		sprites[3]->Render(dc, 1150.0f+15.0f, 350.0f+10.0f, 0.0f, 50, 50, 0, 0, textureWidth, textureWidth, 0, 1, 1, 1, 1);
	}


	font->Textout(rc, "Player", 16, 0, 1.0f, { -10, 10, 0 }, 12, 16, 32, 32, 16, 16, 0, 1, 1, 1, 1);
	//font->Textout(rc, "Time:" + std::to_string((int)gameTimer), 16, 0, 1.0f, { 910, 10, 0 }, 32, 32, 32, 32, 16, 16, 0, 1, 1, 1, 1);
	
	//gizmos
	Gizmos* gizmos = Graphics::Instance().GetGizmos();
	rc.camera = &camera;
	rc.deviceContext = Graphics::Instance().GetDeviceContext();
	rc.renderState = Graphics::Instance().GetRenderState();
	//描画実行
	gizmos->Render(rc);

	//Loadingの為ここでブレンドステート変更
	FLOAT blendFactor[4] = { 1.0f,1.0f,1.0f,1.0f };
	UINT sampleMask = 0xFFFFFFFF;
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Transparency), blendFactor, sampleMask);

#if showDebug//def _DEBUG
	// 3Dデバッグ描画
	{
		//プレイヤーデバッグプリミティブ描画
		player1P->DrawDebugPrimitive();
		playerAI->DrawDebugPrimitive();
		//エネミーデバッグプリミティブ描画
		EnemyManager::Instance().DrawDebugPrimitive();
	}

	//デバッグメニュー描画
	//DrawSceneGUI();
	//DrawPropertyGUI();
	camera.DebugImGui();
	player1P->DebugMenu();
	playerAI->DebugMenu();
	enemyBlue->DebugMenu();

#if 0
	// shadowMap
	{
		ShadowMap* shadowMap = Graphics::Instance().GetShadowMap();

		ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
		ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

		ImGui::Begin("ShadowMap", nullptr, ImGuiWindowFlags_None);

		ImGui::Image(shadowMap->GetShaderResourceView(), ImVec2(200, 200));

		ImGui::End();
	}

#endif
#endif
}

// エネミーHPゲージ描画
void SceneGame::RenderEnemyGauge(ID3D11DeviceContext* dc, const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection)
{
	//ビューポート
	D3D11_VIEWPORT viewport;
	UINT numViewports = 1;
	dc->RSGetViewports(&numViewports, &viewport);

	//変換行列
	XMMATRIX View = XMLoadFloat4x4(&view);
	XMMATRIX Projection = XMLoadFloat4x4(&projection);
	XMMATRIX World = XMMatrixIdentity();

	//全ての敵の頭上にHPゲージを表示
	EnemyManager& enemyManager = EnemyManager::Instance();
	int enemyCount = enemyManager.GetEnemyCount();

	for (int i = 0; i < enemyCount; ++i)
	{
		Enemy* enemy = enemyManager.GetEnemy(i);

		//エネミー頭上のワールド座標
		XMFLOAT3 worldPosition = enemy->GetPosition();
		worldPosition.y += enemy->GetHeight();
		XMVECTOR WorldPosition = XMLoadFloat3(&worldPosition);

		//ワールドからスクリーンへの変換
		XMVECTOR ScreenPosition = XMVector3Project(
			WorldPosition,
			viewport.TopLeftX,
			viewport.TopLeftY,
			viewport.Width,
			viewport.Height,
			viewport.MinDepth,
			viewport.MaxDepth,
			Projection,
			View,
			World
		);

		XMFLOAT3 screenPosition;
		XMStoreFloat3(&screenPosition, ScreenPosition);

		//カメラの背後にいるか、明らかに離れているなら描画しない
		if (screenPosition.z < 0.0f || screenPosition.z > 1.0f) continue;

		//HPゲージの長さ
		const float guageWidth = 60.0f;
		const float guageHeight = 8.0f;

		float healthRate = enemy->GetHealth() / static_cast<float>(enemy->GetMaxHealth());

		//ゲージ描画(下地)
		gauge->Render(dc,
			screenPosition.x - guageWidth * 0.5f - 2,
			screenPosition.y - guageHeight - 2,
			0,
			guageWidth + 4,
			guageHeight + 4,
			0, 0,
			static_cast<float>(gauge->GetTextureWidth()),
			static_cast<float>(gauge->GetTextureHeight()),
			0.0f,
			0.5f, 0.5f, 0.5f, 0.5f
		);
		//ゲージ描画
		gauge->Render(dc,
			screenPosition.x - guageWidth * 0.5f,
			screenPosition.y - guageHeight,
			0,
			guageWidth * healthRate,
			guageHeight,
			0, 0,
			static_cast<float>(gauge->GetTextureWidth()),
			static_cast<float>(gauge->GetTextureHeight()),
			0.0f,
			1.0f, 0.0f, 0.0f, 1.0f
		);
	}
}

// キャラクター名前描画
void SceneGame::RenderCharacterName(const RenderContext& rc, const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection)
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	//ビューポート
	D3D11_VIEWPORT viewport;
	UINT numViewports = 1;
	dc->RSGetViewports(&numViewports, &viewport);

	//変換行列
	XMMATRIX View = XMLoadFloat4x4(&view);
	XMMATRIX Projection = XMLoadFloat4x4(&projection);
	XMMATRIX World = XMMatrixIdentity();

	{
		Player1P& player = Player1P::Instance();

		//Player頭上のワールド座標
		XMFLOAT3 worldPosition = player.GetPosition();
		worldPosition.y += player.GetHeight() + 0.4f;
		XMVECTOR WorldPosition = XMLoadFloat3(&worldPosition);

		//ワールドからスクリーンへの変換
		XMVECTOR ScreenPosition = XMVector3Project(
			WorldPosition,
			viewport.TopLeftX,
			viewport.TopLeftY,
			viewport.Width,
			viewport.Height,
			viewport.MinDepth,
			viewport.MaxDepth,
			Projection,
			View,
			World
		);

		XMFLOAT3 screenPosition;
		XMStoreFloat3(&screenPosition, ScreenPosition);
		//HPゲージの長さ
		const float guageWidth = 60.0f;
		const float guageHeight = 8.0f;
		float screenWidth = Graphics::Instance().GetScreenWidth();
		float screenHeight = Graphics::Instance().GetScreenHeight();


		//カメラの背後にいるか、明らかに離れているなら描画しない
		if (screenPosition.z > 0.0f && screenPosition.z < 1.0f)
		{
			font->Textout(rc, "PLAYER",
				0,
				screenPosition.y,
				0,
				{ screenPosition.x - 12 * 5, 0, 0 },
				12, 16,
				32, 32, 16, 16, 0, 0.1f, 0.65f, 0.9f, 1);
		}
	}
	{
		PlayerAI& player = PlayerAI::Instance();

		//Player頭上のワールド座標
		XMFLOAT3 worldPosition = player.GetPosition();
		worldPosition.y += player.GetHeight() + 0.4f;
		XMVECTOR WorldPosition = XMLoadFloat3(&worldPosition);

		//ワールドからスクリーンへの変換
		XMVECTOR ScreenPosition = XMVector3Project(
			WorldPosition,
			viewport.TopLeftX,
			viewport.TopLeftY,
			viewport.Width,
			viewport.Height,
			viewport.MinDepth,
			viewport.MaxDepth,
			Projection,
			View,
			World
		);

		XMFLOAT3 screenPosition;
		XMStoreFloat3(&screenPosition, ScreenPosition);
		//HPゲージの長さ
		const float guageWidth = 60.0f;
		const float guageHeight = 8.0f;
		float screenWidth = Graphics::Instance().GetScreenWidth();
		float screenHeight = Graphics::Instance().GetScreenHeight();

		//カメラの背後にいるか、明らかに離れているなら描画しない
		if (screenPosition.z > 0.0f && screenPosition.z < 1.0f)
		{
			font->Textout(rc, "COM",
				0,
				screenPosition.y,
				0,
				{ screenPosition.x - 12 * 3, 0, 0 },
				12, 16,
				32, 32, 16, 16, 0, 1, 1, 1, 1);
		}
	}
}

//シーンGUI描画
void SceneGame::DrawSceneGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_None))
	{
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//位置
			ImGui::DragFloat3("Position", &position.x, 0.1f);

			//回転
			XMFLOAT3 a;
			a.x = XMConvertToDegrees(angle.x);
			a.y = XMConvertToDegrees(angle.y);
			a.z = XMConvertToDegrees(angle.z);
			ImGui::DragFloat3("Angle", &a.x, 1.0f);
			angle.x = XMConvertToRadians(a.x);
			angle.y = XMConvertToRadians(a.y);
			angle.z = XMConvertToRadians(a.z);

			//スケール
			ImGui::DragFloat3("Scale", &scale.x, 0.01f);
		}

		if (ImGui::CollapsingHeader("Hierarchy", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//ノードツリーを再帰的に描画する関数
			std::function<void(Model::Node*)> drawNodeTree = [&](Model::Node* node)
			{
				//矢印をクリック、またはノードをダブルクリックで階層を開く
				ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow
					| ImGuiTreeNodeFlags_OpenOnDoubleClick;

				//子がいない場合は矢印を付けない
				size_t childCount = node->children.size();
				if (childCount == 0)
				{
					nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
				}

				//選択フラグ
				if (selectionNode == node)
				{
					nodeFlags |= ImGuiTreeNodeFlags_Selected;
				}

				//ツリーノードを表示
				bool opened = ImGui::TreeNodeEx(node, nodeFlags, node->name.c_str());

				//フォーカスされたノードを選択
				if (ImGui::IsItemFocused())
				{
					selectionNode = node;
				}

				//開かれている場合、子階層も同じ処理を行う
				if (opened && childCount > 0)
				{
					for (Model::Node* child : node->children)
					{
						drawNodeTree(child);
					}
					ImGui::TreePop();
				}
			};
			//再帰的にノードを描画
			//drawNodeTree(model->GetRootNode());
		}

		ImGui::End();
	}
}

//プロパティGUI描画
void SceneGame::DrawPropertyGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
	ImGui::SetNextWindowPos(ImVec2(pos.x + 970, pos.y + 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	ImGui::Begin("Property", nullptr, ImGuiWindowFlags_None);

	if (selectionNode != nullptr)
	{
		if (ImGui::CollapsingHeader("Node", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//位置
			ImGui::DragFloat3("Position", &selectionNode->position.x, 0.1f);

			//回転
			XMFLOAT3 angle;
			TransformUtils::QuaternionToRollPitchYaw(selectionNode->rotation, angle.x, angle.y, angle.z);
			angle.x = XMConvertToDegrees(angle.x);
			angle.y = XMConvertToDegrees(angle.y);
			angle.z = XMConvertToDegrees(angle.z);
			if (ImGui::DragFloat3("Rotation", &angle.x, 1.0f))
			{
				angle.x = XMConvertToRadians(angle.x);
				angle.y = XMConvertToRadians(angle.y);
				angle.z = XMConvertToRadians(angle.z);
				XMVECTOR Rotation = XMQuaternionRotationRollPitchYaw(angle.x, angle.y, angle.z);

				//XMStoreFloat4(&selectionNode->rotation, Rotation);
			}
			//スケール
			ImGui::DragFloat3("Scale", &selectionNode->scale.x, 0.01f);
		}
	}

	ImGui::End();
}
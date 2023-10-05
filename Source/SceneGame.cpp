#include <functional>
#include "Graphics/Graphics.h"
#include "Graphics/FontSprite.h"
#include "SceneManager.h"
#include "SceneLoading.h"
#include "SceneGame.h"
#include "EnemyManager.h"
#include "EnemySlime.h"
#include "EnemyTurtleShell.h"
#include "TransformUtils.h"
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
	player = std::make_unique<Player>();

	sprites[0] = std::make_unique<Sprite>(device, "Data/Sprite/view.jpg");
	//sprites[1] = std::make_unique<Sprite>(device);
	//sprites[2] = std::make_unique<Sprite>(device, "Data/Sprite/3.png");
	//sprites[3] = std::make_unique<Sprite>(device, "Data/Sprite/4.png");
	font = std::make_unique<FontSprite>(device, "Data/Font/font6.png", 256);
	//ゲージスプライト
	gauge = std::make_unique<Sprite>(device);

	//エネミー初期化
	EnemyManager& enemyManager = EnemyManager::Instance();
	{
		EnemySlime* slime = new EnemySlime();
		slime->SetPosition(DirectX::XMFLOAT3(10, 10, 0));
		enemyManager.Register(slime);
	}
	//for (int i = 0; i < 8; i++)
	//{
	//	EnemySlime* slime = new EnemySlime();
	//	slime->SetPosition(DirectX::XMFLOAT3(20 + i * 3.0f, 1, 0));
	//	enemyManager.Register(slime);
	//}
	{
		EnemyTurtleShell* turtleShell = new EnemyTurtleShell();
		turtleShell->SetPosition(DirectX::XMFLOAT3(26, 10, 0));
		enemyManager.Register(turtleShell);
	}
	{
		EnemyTurtleShell* turtleShell = new EnemyTurtleShell();
		turtleShell->SetPosition(DirectX::XMFLOAT3(28, 10, 0));
		enemyManager.Register(turtleShell);
	}
	for (int i = 0; i < 4; i++)
	{
		EnemyTurtleShell* turtleShell = new EnemyTurtleShell();
		turtleShell->SetPosition(DirectX::XMFLOAT3(111 + i * 6.0f, 10, 0));
		enemyManager.Register(turtleShell);
	}

	//ライト設定
	DirectionalLight directionalLight;
	directionalLight.direction = { 2, -4, 3 };
	directionalLight.color = { 1,1,1 };
	lightManager.SetDirectionalLight(directionalLight);
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
	XMFLOAT3 target = player->GetPosition();
	ViewPosition.x = -(target.x + 20.0f); //背景スライド
	target.y += 0.5f;
	cameraController->SetTarget(target);
	cameraController->Update(elapsedTime);

	player->Update(elapsedTime,1);

	//エネミー更新
	EnemyManager::Instance().Update(elapsedTime);
	stage->Update(elapsedTime);
	//エフェクト更新処理
	EffectManager::Instance().Update(elapsedTime);
}

void SceneGame::Render()
{
	Camera& camera = Camera::Instance();
	ShadowMap* shadowMap = Graphics::Instance().GetShadowMap();
	
	RenderContext rc;
	//カメラ更新処理
	rc.view = camera.GetView();
	rc.projection = camera.GetProjection();

	//描画コンテキスト設定
	rc.camera = &camera;
	rc.deviceContext = Graphics::Instance().GetDeviceContext();
	rc.renderState = Graphics::Instance().GetRenderState();
	rc.lightManager = &lightManager;
	rc.shadowMap = shadowMap;
	rc.shadowColor = { 0.5f,0.5f,0.5f };

	//シャドウマップ描画
	shadowMap->Begin(rc, camera.GetFocus());
	stage->ShadowRender(rc, shadowMap);
	player->ShadowRender(rc, shadowMap);
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
	float textureWidth = static_cast<float>(sprites[0]->GetTextureWidth());
	float textureHeight = static_cast<float>(sprites[0]->GetTextureHeight());

	sprites[0]->Render(rc.deviceContext, ViewPosition.x, ViewPosition.y, ViewPosition.z, textureWidth, textureHeight, 0, 0, textureWidth, textureHeight, 0, 1, 1, 1, 1);

	ModelShader* shader = Graphics::Instance().GetShader(ShaderId::Toon);
	shader->Begin(rc);
	player->Render(rc, shader);
	EnemyManager::Instance().Render(rc, shader);
	shader->End(rc);

	ModelShader* stageShader = Graphics::Instance().GetShader(ShaderId::Phong);
	stageShader->Begin(rc);
	stage->Render(rc, stageShader);
	stageShader->End(rc);

	//{
	//	PrimitiveRenderer* primitiveRenderer = Graphics::Instance().GetPrimitiveRenderer();

	//	// ポリゴン描画
	//	primitiveRenderer->Render(rc.deviceContext, rc.camera->GetView(), rc.camera->GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//	// グリッド描画
	//	primitiveRenderer->DrawGrid(20, 1);
	//	primitiveRenderer->Render(rc.deviceContext, rc.camera->GetView(), rc.camera->GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	//}

	//3Dエフェクト描画
	EffectManager::Instance().Render(rc.view, rc.projection);

	////2DSprite
	//{
	//	float screenWidth = static_cast<float>(graphics.GetScreenWidth());
	//	float screenHeight = static_cast<float>(graphics.GetScreenHeight());
	//	float textureWidth = static_cast<float>(sprite[0]->GetTextureWidth());
	//	float textureHeight = static_cast<float>(sprite[0]->GetTextureHeight());

	//	FLOAT blendFactor[4] = { 1.0f,1.0f,1.0f,1.0f };
	//	UINT sampleMask = 0xFFFFFFFF;

	//	//titleSprite
	//	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Transparency), blendFactor, sampleMask);
	//	sprite[0]->Render(dc, 0.0f, 0.0f, 0.0f, screenWidth, screenHeight, 0, 0, textureWidth, textureHeight, 0, 1, 1, 1, 1);
	//}
	// 2Dスプライト描画
	{
		RenderEnemyGauge(dc, rc.view, rc.projection);
	}


	font->Textout(rc, "Unity-Chan:", 16, 0, 1.0f, { -10, 10, 0 }, 32, 32, 32, 32, 16, 16, 0, 1, 1, 1, 1);
	font->Textout(rc, "Time:" + std::to_string((int)gameTimer), 16, 0, 1.0f, { 910, 10, 0 }, 32, 32, 32, 32, 16, 16, 0, 1, 1, 1, 1);
	// 3Dデバッグ描画
	{
		//プレイヤーデバッグプリミティブ描画
		player->DrawDebugPrimitive();
		//エネミーデバッグプリミティブ描画
		EnemyManager::Instance().DrawDebugPrimitive();
	}

	//デバッグメニュー描画
	//DrawSceneGUI();
	//DrawPropertyGUI();
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

	//デバッグメニュー描画
	{
		ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
		ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Player", nullptr, ImGuiWindowFlags_None))
		{
			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
			{
				//スケール
				ImGui::DragInt("playCount", &playCount, 1);
			}

			ImGui::End();
		}
	}
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


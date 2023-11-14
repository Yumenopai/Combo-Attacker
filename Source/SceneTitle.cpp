#include <functional>
#include "Graphics/Graphics.h"
#include "Graphics/FontSprite.h"
#include "SceneManager.h"
#include "SceneTitle.h"
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
void SceneTitle::Initialize()
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
		XMFLOAT3(0, 10, 10),	//視点
		XMFLOAT3(0, 0, 0),		//注視点
		XMFLOAT3(0, 1, 0)		//上ベクトル
	);

	cameraController = std::make_unique<CameraController>();

	//モデル作成
	stage = std::make_unique<Stage>();
	player = std::make_unique<Player>();
	sprites[0] = std::make_unique<Sprite>(device, "Data/Sprite/view.jpg");
	//sprites[1] = std::make_unique<Sprite>(device, "Data/Sprite/1.png");
	//sprites[2] = std::make_unique<Sprite>(device, "Data/Sprite/2.png");
	//sprites[3] = std::make_unique<Sprite>(device, "Data/Sprite/3.png");
	//sprites[4] = std::make_unique<Sprite>(device, "Data/Sprite/6.png");
	//sprites[5] = std::make_unique<Sprite>(device, "Data/Sprite/7.png");
	font = std::make_unique<FontSprite>(device, "Data/Font/font6.png", 256);

	//エネミー初期化
	EnemyManager& enemyManager = EnemyManager::Instance();
	EnemySlime* slime = new EnemySlime();
	slime->SetPosition(XMFLOAT3(60, 10, 0));
	enemyManager.Register(slime);

	//ライト設定
	DirectionalLight directionalLight;
	directionalLight.direction = { 2, -4, 3 };
	directionalLight.color = { 1,1,1 };
	lightManager.SetDirectionalLight(directionalLight);
}

// 終了化
void SceneTitle::Finalize()
{
	//エネミー終了化
	EnemyManager::Instance().Clear();

}

void SceneTitle::Update(float elapsedTime)
{
	player->Update(elapsedTime, playCount);

	//if (player->GetPosition().y < -8.0f)
	//{
	//	SceneManager::Instance().ChangeScene(new SceneTitle());
	//}
	//if (player->GetState() == Player::State::Finish || isBlack)
	//{
	//	timer += elapsedTime;
	//	blackPosition.x += timer * 8.0f;
	//	if(blackPosition.x >= 0)
	//		SceneManager::Instance().ChangeScene(new SceneLoading(new SceneGame(3), 3));
	//}

	if (player->GetState() == Player::State::Damage || player->GetState() == Player::State::Dead) return;
	//エネミー更新
	EnemyManager::Instance().Update(elapsedTime);
	stage->Update(elapsedTime);
	//エフェクト更新処理
	EffectManager::Instance().Update(elapsedTime);

	//カメラコントローラー更新処理
	XMFLOAT3 target = player->GetPosition();
	ViewPosition.x = -(target.x + 10.0f); //背景スライド
	target.y += 0.5f;
	if (player->GetPosition().y > -8.0f)
	{
		cameraController->SetTarget(target);
	}
	//else
	//{
	//	XMFLOAT3 playerPotision = player->GetPosition();
	//	XMVECTOR vectorPlayerP = XMLoadFloat3(&playerPotision);
	//	if (!isInit)
	//	{
	//		XMFLOAT3 flagPotision = flag->GetPosition();
	//		
	//		XMVECTOR vectorFlagP = XMLoadFloat3(&flagPotision);
	//		playerToTarget = vectorPlayerP;

	//		zoomRateCalculation = XMVectorScale(XMVectorSubtract(vectorPlayerP, vectorFlagP), zoomRate);
	//	}
	//	playerToTarget = XMVectorAdd(playerToTarget, zoomRateCalculation);

	//	XMFLOAT3 move;
	//	XMStoreFloat3(&move, playerToTarget);
	//	cameraController->SetTarget(move);
	//}
	cameraController->Update(elapsedTime);
}

void SceneTitle::Render()
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
	//2d背景
	{
		float textureWidth = static_cast<float>(sprites[0]->GetTextureWidth());
		float textureHeight = static_cast<float>(sprites[0]->GetTextureHeight());

		sprites[0]->Render(dc, ViewPosition.x, ViewPosition.y, ViewPosition.z, textureWidth, textureHeight, 0, 0, textureWidth, textureHeight, 0, 1, 1, 1, 1);
	}

	//シャドウマップ描画
	shadowMap->Begin(rc, camera.GetFocus());
	stage->ShadowRender(rc, shadowMap);
	player->ShadowRender(rc, shadowMap);
	EnemyManager::Instance().ShadowRender(rc, shadowMap);
	shadowMap->End(rc);

	//ModelShader* shader = Graphics::Instance().GetShader(ShaderId::Phong);
	ModelShader* shader = Graphics::Instance().GetShader(ShaderId::Toon);
	shader->Begin(rc);
	stage->TerrainRender(rc, shader);
	player->Render(rc, shader);
	EnemyManager::Instance().Render(rc, shader);
	shader->End(rc);

	//3Dエフェクト描画
	EffectManager::Instance().Render(rc.view, rc.projection);
	//プレイヤーデバッグプリミティブ描画
	player->DrawDebugPrimitive();

	//2DSprite
	{
		//float textureWidth = static_cast<float>(sprites[1]->GetTextureWidth());
		//float textureHeight = static_cast<float>(sprites[1]->GetTextureHeight());

		//FLOAT blendFactor[4] = { 1.0f,1.0f,1.0f,1.0f };
		//UINT sampleMask = 0xFFFFFFFF;

		//dc->OMSetBlendState(renderState->GetBlendState(BlendState::Transparency), blendFactor, sampleMask);
		//sprites[1]->Render(dc, (ViewPosition.x + 20.0f) * 30.0f, ViewPosition.y, ViewPosition.z, screenWidth, screenHeight, 0, 0, textureWidth, textureHeight, 0, 1, 1, 1, 1);
		//if (ViewPosition.x + 20.0f > -10.0f)
		//{
		//	sprites[2]->Render(dc, 0, 0, 0, screenWidth, screenHeight, 0, 0, textureWidth, textureHeight, 0, 1, 1, 1, 1);
		//	sprites[4]->Render(dc, 0, 0, 0, screenWidth, screenHeight, 0, 0, textureWidth, textureHeight, 0, 1, 1, 1, 1);
		//}
		//if (ViewPosition.x + 20.0f < -30.0f && ViewPosition.x + 20.0f > -50.0f)
		//{
		//	sprites[3]->Render(dc, 0, (player->GetPosition().y * 30.0f) + 75.0f, 0, screenWidth, screenHeight, 0, 0, textureWidth, textureHeight, 0, 1, 1, 1, 1);
		//}
		//sprites[5]->Render(dc, blackPosition.x, blackPosition.y, blackPosition.z, screenWidth, screenHeight, 0, 0, textureWidth, textureHeight, 0, 1, 1, 1, 1);

	}

	//デバッグメニュー描画
	DrawSceneGUI();
	//DrawPropertyGUI();
	//デバッグメニュー描画
#if 0
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

//シーンGUI描画
void SceneTitle::DrawSceneGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_None))
	{
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//位置
			XMFLOAT3 move;
			XMStoreFloat3(&move, playerToTarget);
			ImGui::DragFloat3("move", &move.x, 0.1f);

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
void SceneTitle::DrawPropertyGUI()
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

				XMStoreFloat4(&selectionNode->rotation, Rotation);
			}
			//スケール
			ImGui::DragFloat3("Scale", &selectionNode->scale.x, 0.01f);
		}
	}

	ImGui::End();
}


#include <functional>

#include "SceneGame.h"
#include "Graphics/Graphics.h"
#include "Graphics/FontSprite.h"
#include "SceneManager.h"
#include "SceneLoading.h"
#include "Character/Manager/PlayerManager.h"
#include "Character/Manager/EnemyManager.h"
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
		XMFLOAT3(32, 4, 29),	//視点
		XMFLOAT3(28, 1.5f, 19),	//注視点
		XMFLOAT3(0, 1, 0)		//上ベクトル
	);

	cameraController = std::make_unique<CameraController>();

	//モデル作成
	stage = std::make_unique<Stage>();
	player1P = std::make_unique<Player1P>();
	playerAI = std::make_unique<PlayerAI>();

	for (int i = 0; i < enemy_slime_count; i++)
	{
		enemySlime[i] = std::make_unique<EnemySlime>();
	}
	for (int i = 0; i < enemy_turtleShell_count; i++)
	{
		enemyTurtleShell[i] = std::make_unique<EnemyTurtleShell>();
	}
	enemyDragon = std::make_unique<EnemyDragon>();

	PlayerManager& playerManager = PlayerManager::Instance();
	playerManager.Register(player1P.get());
	playerManager.Register(playerAI.get());

	//スカイボックス
	skyBox = std::make_unique<SkyBox>("Data/Texture/incskies_002_8k.png");

	// load sprite
	spriteButtonFrame = std::make_unique<Sprite>(device, "Data/Sprite/frame.png");
	spriteWeaponIcon = std::make_unique<Sprite>(device, "Data/Sprite/weapon.png");
	spriteNameOnButton = std::make_unique<Sprite>(device, "Data/Sprite/name.png");
	spriteMissionFrame = std::make_unique<Sprite>(device, "Data/Sprite/missionFrame.png");
	spriteMissionText = std::make_unique<Sprite>(device, "Data/Sprite/missionText.png");
	spriteNotification = std::make_unique<Sprite>(device, "Data/Sprite/notification.png");
	
	font = std::make_unique<FontSprite>(device, "Data/Font/font6.png", 256);
	gauge = std::make_unique<Sprite>(device);

	//エネミー初期化
	enemySlime[0]->SetPosition(DirectX::XMFLOAT3(30, 5, -43));
	enemySlime[1]->SetPosition(DirectX::XMFLOAT3(28, 5, -21));
	enemySlime[2]->SetPosition(DirectX::XMFLOAT3(30, 5, -23));
	enemySlime[3]->SetPosition(DirectX::XMFLOAT3(-28, 5, 4));

	enemyTurtleShell[0]->SetPosition(DirectX::XMFLOAT3(-26, 5, 6));
	enemyTurtleShell[1]->SetPosition(DirectX::XMFLOAT3(28, 5, 0));
	enemyTurtleShell[2]->SetPosition(DirectX::XMFLOAT3(-7, 5, -36));
	enemyTurtleShell[3]->SetPosition(DirectX::XMFLOAT3(-12, 5, -40));

	enemyDragon->SetPosition(DirectX::XMFLOAT3(-28, 5, -12));

	EnemyManager& enemyManager = EnemyManager::Instance();
	for (int i = 0; i < enemy_slime_count; i++)
	{
		enemyManager.Register(enemySlime[i].get());
	}
	for (int i = 0; i < enemy_turtleShell_count; i++)
	{
		enemyManager.Register(enemyTurtleShell[i].get());
	}

	enemyManager.Register(enemyDragon.get());

	// 平行光源を追加
	Light* mainDirectionalLight = new Light(LightType::Directional);
	mainDirectionalLight->SetDirection({ 1, -1, -2 });
	LightManager::Instane().Register(mainDirectionalLight);
}

// 終了化
void SceneGame::Finalize()
{
	LightManager::Instane().Clear();
	EnemyManager::Instance().Clear();
	PlayerManager::Instance().Clear();
}

// 更新処理
void SceneGame::Update(float elapsedTime)
{
	//カメラコントローラー更新処理
	XMFLOAT3 target = player1P->GetPosition();
	target.y += SCENEGAME_camera_y;
	cameraController->SetTarget(target);
	cameraController->Update(elapsedTime);

	// マネージャーによる更新
	PlayerManager::Instance().Update(elapsedTime);
	EnemyManager::Instance().Update(elapsedTime);
	stage->Update(elapsedTime);

	//エフェクト更新処理
	EffectManager::Instance().Update(elapsedTime);
}

// 描画処理
void SceneGame::Render()
{
	Camera& camera = Camera::Instance();
	ShadowMap* shadowMap = Graphics::Instance().GetShadowMap();
	
	RenderContext rc;
	rc.timer = ++waterTimer;

	// カメラ更新処理
	rc.view = camera.GetView();
	rc.projection = camera.GetProjection();

	// 描画コンテキスト設定
	rc.camera = &camera;
	rc.deviceContext = Graphics::Instance().GetDeviceContext();
	rc.renderState = Graphics::Instance().GetRenderState();
	LightManager::Instane().PushRenderContext(rc); // ライトの情報を詰め込む
	rc.shadowMap = shadowMap;
	rc.shadowColor = SCENEGAME_shadow_color;

	// シャドウマップ描画
	shadowMap->Begin(rc, camera.GetFocus());
	stage->ShadowRender(rc, shadowMap);
	PlayerManager::Instance().ShadowRender(rc, shadowMap);
	EnemyManager::Instance().ShadowRender(rc, shadowMap);
	shadowMap->End(rc);

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

	// 3Dモデル描画
	ModelShader* shader = Graphics::Instance().GetShader(ShaderId::Toon);
	shader->Begin(rc);
	PlayerManager::Instance().Render3d(rc, shader);
	stage->TerrainRender(rc, shader);
	EnemyManager::Instance().Render(rc, shader);
	shader->End(rc);

	// 水面描画
	ModelShader* waterShader = Graphics::Instance().GetShader(ShaderId::WaterSurface);
	waterShader->Begin(rc);
	stage->WaterRender(rc, waterShader);
	waterShader->End(rc);

	//3Dエフェクト描画
	EffectManager::Instance().Render(rc.view, rc.projection);

	// ブレンドステート設定
	FLOAT blendFactor[4] = { 1.0f,1.0f,1.0f,1.0f };
	UINT sampleMask = 0xFFFFFFFF;
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Transparency), blendFactor, sampleMask);

	// 2Dスプライト描画
	{
		// エネミーHP
		RenderEnemyGauge(dc, rc.view, rc.projection);
		// プレイヤー2DRender
		PlayerManager::Instance().Render2d(rc, gauge.get(), font.get(), 
			spriteButtonFrame.get(),
			spriteWeaponIcon.get(),
			spriteNotification.get()
		);
		// ボタンUI
		RenderButtonUI(dc);
		// ミッションUI
		RenderMissionUI(dc);
	}

#if _DEBUG
	// gizmos
	Gizmos* gizmos = Graphics::Instance().GetGizmos();
	// 描画実行
	gizmos->Render(rc);

	// 3Dデバッグ描画
	{
		//プレイヤーデバッグプリミティブ描画
		player1P->DrawDebugPrimitive();
		playerAI->DrawDebugPrimitive();
		//エネミーデバッグプリミティブ描画
		EnemyManager::Instance().DrawDebugPrimitive();
	}

	//デバッグメニュー描画
	{
		//DrawSceneGUI();
		//DrawPropertyGUI();
		//DrawShadowMapGUI();
		camera.DebugImGui();
		player1P->DebugMenu();
		playerAI->DebugMenu();
		enemyDragon->DebugMenu();
	}
#endif
}

bool SceneGame::PlayersHaveAnySameWeapon()
{
	for (int i = 0; i < SC_INT(Player::AttackType::MaxCount); i++)
	{
		if (Player1P::Instance().GetHaveEachWeapon(SC_AT(i))
			&& PlayerAI::Instance().GetHaveEachWeapon(SC_AT(i)))
		{
			return true;
		}
	}
	return false;
}

// エネミーHPゲージ描画
void SceneGame::RenderEnemyGauge(ID3D11DeviceContext* dc, const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection)
{
	//ビューポート
	D3D11_VIEWPORT viewport;
	UINT numViewports = 1;
	dc->RSGetViewports(&numViewports, &viewport);

	//変換行列
	DirectX::XMMATRIX View = XMLoadFloat4x4(&view);
	DirectX::XMMATRIX Projection = XMLoadFloat4x4(&projection);
	DirectX::XMMATRIX World = XMMatrixIdentity();

	//全ての敵の頭上にHPゲージを表示
	EnemyManager& enemyManager = EnemyManager::Instance();
	int enemyCount = enemyManager.GetEnemyCount();

	for (int i = 0; i < enemyCount; ++i)
	{
		Enemy* enemy = enemyManager.GetEnemy(i);
		//距離が遠い時はcontinue
		if (Player1P::Instance().GetEachEnemyDist(enemy) > enemy_hp_gauge_display_dist) continue;

		//エネミー頭上のワールド座標
		DirectX::XMFLOAT3 worldPosition = enemy->GetPosition();
		worldPosition.y += enemy->GetHeight();
		DirectX::XMVECTOR WorldPosition = XMLoadFloat3(&worldPosition);

		//ワールドからスクリーンへの変換
		DirectX::XMVECTOR ScreenPosition = XMVector3Project(
			WorldPosition,
			viewport.TopLeftX, viewport.TopLeftY,
			viewport.Width, viewport.Height,
			viewport.MinDepth, viewport.MaxDepth,
			Projection, View, World);

		DirectX::XMFLOAT3 screenPosition;
		DirectX::XMStoreFloat3(&screenPosition, ScreenPosition);
		//カメラの背後にいるか、明らかに離れているなら描画しない
		if (screenPosition.z < 0.0f || screenPosition.z > 1.0f) continue;

		//HPパーセンテージ
		float healthRate = enemy->GetHealthRate() / 100.0f; //百分率から小数に変換

		//ゲージ描画(下地)
		gauge->Render(dc,
			screenPosition.x - enemy_hp_gauge_size.x / 2 - enemy_hp_gauge_frame_expansion / 2, // X_中央に配置するため幅の半分とゲージ長さの半分で求める
			screenPosition.y - enemy_hp_gauge_size.y - enemy_hp_gauge_frame_expansion / 2, // Y_上下の拡張を合わせたサイズ分で足しているため半分引く
			SPRITE_position_default_z,
			enemy_hp_gauge_size.x + enemy_hp_gauge_frame_expansion,
			enemy_hp_gauge_size.y + enemy_hp_gauge_frame_expansion,
			SPRITE_angle_default,
			enemy_hp_gauge_frame_color
		);
		//ゲージ描画
		gauge->Render(dc,
			screenPosition.x - enemy_hp_gauge_size.x / 2,
			screenPosition.y - enemy_hp_gauge_size.y,
			SPRITE_position_default_z,
			enemy_hp_gauge_size.x * healthRate,
			enemy_hp_gauge_size.y,
			SPRITE_angle_default,
			enemy_hp_gauge_color_normal
		);
	}
}

// ボタンUI描画
void SceneGame::RenderButtonUI(ID3D11DeviceContext* dc)
{
	float spriteCutPosition_x = 0;

	// X：左 Player
	{
		// ボタンフレーム描画
		spriteButtonFrame->Render(dc,
			{ ButtonFrame_button_X_position_x, ButtonFrame_button_XB_position_y, SPRITE_position_default_z },
			ButtonFrame_size,
			SPRITE_cut_position_default, ButtonFrame_sprite_size,
			SPRITE_angle_default, SPRITE_color_default);

		auto nextWeapon1P = Player1P::Instance().GetNextWeapon();
		if (nextWeapon1P != Player1P::Instance().GetCurrentUseWeapon()) // 次が現在の武器＝武器が一つだけしかもっていない時は表示しない
		{
			// 武器に対応するカット位置
			spriteCutPosition_x = SpriteCutPositionX(nextWeapon1P);

			// ボタンアイコン描画
			spriteWeaponIcon->Render(dc,
				{ ButtonFrame_button_X_position_x + ButtonIcon_frame_to_icon_offset.x, ButtonFrame_button_XB_position_y + ButtonIcon_frame_to_icon_offset.y, SPRITE_position_default_z },
				ButtonIcon_render_size,
				{ spriteCutPosition_x, ButtonFrame_sprite_size.y },
				ButtonFrame_sprite_size,
				SPRITE_angle_default,
				SPRITE_color_default);
		}
		// 名前描画
		spriteNameOnButton->Render(dc,
			{ ButtonFrame_button_X_position_x + ButtonIcon_frame_to_icon_offset.x, ButtonFrame_button_XB_position_y + ButtonIcon_frame_to_icon_offset.y, SPRITE_position_default_z },
			ButtonName_render_size,
			SPRITE_cut_position_default,
			ButtonName_sprite_size,
			SPRITE_angle_default,
			SPRITE_color_default);

	}
	// Y：上 Buddy
	{
		// ボタンフレーム描画
		spriteButtonFrame->Render(dc,
			{ ButtonFrame_button_YA_position_x, ButtonFrame_button_Y_position_y, SPRITE_position_default_z },
			ButtonFrame_size,
			SPRITE_cut_position_default, ButtonFrame_sprite_size,
			SPRITE_angle_default, SPRITE_color_default);

		bool enable_show = true;
		// 回復遷移可能な場合
		if (Player1P::Instance().GetEnableRecoverTransition())
		{
			spriteCutPosition_x = ButtonFrame_sprite_size.x * ButtonIcon_cut_rate_recover;
		}
		// 武器変更
		else
		{
			auto nextWeaponAI = PlayerAI::Instance().GetNextWeapon();
			if (nextWeaponAI == PlayerAI::Instance().GetCurrentUseWeapon())
			{
				// 次が現在の武器＝武器が一つだけしかもっていない時は表示しない
				enable_show = false;
			}
			else
			{
				// 武器に対応するカット位置
				spriteCutPosition_x = SpriteCutPositionX(nextWeaponAI);
			}
		}
		if (enable_show)
		{
			// ボタンアイコン描画
			spriteWeaponIcon->Render(dc,
				{ ButtonFrame_button_YA_position_x + ButtonIcon_frame_to_icon_offset.x, ButtonFrame_button_Y_position_y + ButtonIcon_frame_to_icon_offset.y, SPRITE_position_default_z },
				ButtonIcon_render_size,
				{ spriteCutPosition_x, ButtonFrame_sprite_size.y },
				ButtonFrame_sprite_size,
				SPRITE_angle_default,
				SPRITE_color_default);
		}
		// 名前描画
		spriteNameOnButton->Render(dc,
			{ ButtonFrame_button_YA_position_x + ButtonIcon_frame_to_icon_offset.x, ButtonFrame_button_Y_position_y + ButtonIcon_frame_to_icon_offset.y, SPRITE_position_default_z },
			ButtonName_render_size,
			{ SPRITE_cut_position_default.x, ButtonName_sprite_size.y },
			ButtonName_sprite_size,
			SPRITE_angle_default,
			SPRITE_color_default);
	}
	// B：右 Attack
	{
		// ボタンフレーム描画
		spriteButtonFrame->Render(dc,
			{ ButtonFrame_button_B_position_x, ButtonFrame_button_XB_position_y, SPRITE_position_default_z },
			ButtonFrame_size,
			SPRITE_cut_position_default, ButtonFrame_sprite_size,
			SPRITE_angle_default, SPRITE_color_default);

		// 武器に対応するカット位置
		spriteCutPosition_x = SpriteCutPositionX(Player1P::Instance().GetCurrentUseWeapon());
		// ボタンアイコン描画
		spriteWeaponIcon->Render(dc,
			{ ButtonFrame_button_B_position_x + ButtonIcon_frame_to_icon_offset.x, ButtonFrame_button_XB_position_y + ButtonIcon_frame_to_icon_offset.y, SPRITE_position_default_z },
			ButtonIcon_render_size,
			{ spriteCutPosition_x, SPRITE_cut_position_default.y },
			ButtonFrame_sprite_size,
			SPRITE_angle_default,
			SPRITE_color_default);
	}
	// A：下 Jump
	{
		// ボタンフレーム描画
		spriteButtonFrame->Render(dc,
			{ ButtonFrame_button_YA_position_x, ButtonFrame_button_A_position_y, SPRITE_position_default_z },
			ButtonFrame_size,
			{ ButtonFrame_sprite_size.x, SPRITE_cut_position_default.y }, ButtonFrame_sprite_size,
			SPRITE_angle_default, SPRITE_color_default);

		// ボタンアイコン描画
		spriteWeaponIcon->Render(dc,
			{ ButtonFrame_button_YA_position_x + ButtonIcon_frame_to_icon_offset.x, ButtonFrame_button_A_position_y + ButtonIcon_frame_to_icon_offset.y, SPRITE_position_default_z },
			ButtonIcon_render_size,
			SPRITE_cut_position_default,
			ButtonFrame_sprite_size,
			SPRITE_angle_default,
			SPRITE_color_default);
	}
}
// 武器に対応するX座標スプライトカット位置を返す
float SceneGame::SpriteCutPositionX(Player::AttackType at)
{
	float position = 0.0f;
	switch (at)
	{
	case Player::AttackType::Sword:
		position = ButtonFrame_sprite_size.x * ButtonIcon_cut_rate_sword;
		break;
	case Player::AttackType::Spear:
		position = ButtonFrame_sprite_size.x * ButtonIcon_cut_rate_spear;
		break;
	case Player::AttackType::Hammer:
		position = ButtonFrame_sprite_size.x * ButtonIcon_cut_rate_hammer;
		break;
	}
	return position;
}

// ミッションUI描画
void SceneGame::RenderMissionUI(ID3D11DeviceContext* dc)
{
	// ミッションフレーム描画
	spriteMissionFrame->Render(dc,
		Mission_position,
		Mission_render_size,
		SPRITE_cut_position_default, 
		Mission_sprite_size,
		SPRITE_angle_default, SPRITE_color_default);

	missionSpriteTimer++;
	if (missionSpriteTimer > Mission_display_timer_max)
	{
		// タイマーリセット
		missionSpriteTimer = 0;
		// AIのHPが危うければ
		if (PlayerAI::Instance().GetHpWorning())
		{
			missionSpriteNumber = Mission::Recover;
		}
		else
		{
			// 1PとAIが同じ武器を持っているか
			bool have = PlayersHaveAnySameWeapon();
			// 表示できる最後の番号まで来ていたら
			if ((have && missionSpriteNumber == Mission::SpecialAttack) ||
				(!have && missionSpriteNumber == Mission::UpperAttack))
			{
				// 初めの番号に戻す
				missionSpriteNumber = Mission::GetWeapon;
			}
			else
			{
				// 次の番号にする
				missionSpriteNumber = static_cast<Mission>(SC_INT(missionSpriteNumber) + 1);
			}
		}
	}

	// ミッションテキスト描画
	spriteMissionText->Render(dc,
		Mission_position,
		Mission_render_size,
		missionSpriteCutPosition[SC_INT(missionSpriteNumber)],
		Mission_sprite_size,
		SPRITE_angle_default, SPRITE_color_default);
}

#pragma region DEBUG_DRAW
//シャドウマップGUI描画
void SceneGame::DrawShadowMapGUI()
{
	ShadowMap* shadowMap = Graphics::Instance().GetShadowMap();

	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	ImGui::Begin("ShadowMap", nullptr, ImGuiWindowFlags_None);

	ImGui::Image(shadowMap->GetShaderResourceView(), ImVec2(200, 200));

	ImGui::End();
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
			XMFLOAT3 a = {};
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
			XMFLOAT3 angle = {};
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
#pragma endregion
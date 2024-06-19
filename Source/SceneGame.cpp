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

// ������
void SceneGame::Initialize()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	float screenWidth = Graphics::Instance().GetScreenWidth();
	float screenHeight = Graphics::Instance().GetScreenHeight();

	Camera& camera = Camera::Instance();
	//�J�����ݒ�
	camera.SetPerspectiveFov(
		XMConvertToRadians(45),		//��p
		screenWidth / screenHeight,	//��ʃA�X�y�N�g��
		0.1f,
		1000.0f
	);
	camera.SetLookAt(
		XMFLOAT3(32, 4, 29),	//���_
		XMFLOAT3(28, 1.5f, 19),	//�����_
		XMFLOAT3(0, 1, 0)		//��x�N�g��
	);

	cameraController = std::make_unique<CameraController>();

	//���f���쐬
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

	//�X�J�C�{�b�N�X
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

	//�G�l�~�[������
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

	// ���s������ǉ�
	Light* mainDirectionalLight = new Light(LightType::Directional);
	mainDirectionalLight->SetDirection({ 1, -1, -2 });
	LightManager::Instane().Register(mainDirectionalLight);
}

// �I����
void SceneGame::Finalize()
{
	LightManager::Instane().Clear();
	EnemyManager::Instance().Clear();
	PlayerManager::Instance().Clear();
}

// �X�V����
void SceneGame::Update(float elapsedTime)
{
	//�J�����R���g���[���[�X�V����
	XMFLOAT3 target = player1P->GetPosition();
	target.y += SCENEGAME_camera_y;
	cameraController->SetTarget(target);
	cameraController->Update(elapsedTime);

	// �}�l�[�W���[�ɂ��X�V
	PlayerManager::Instance().Update(elapsedTime);
	EnemyManager::Instance().Update(elapsedTime);
	stage->Update(elapsedTime);

	//�G�t�F�N�g�X�V����
	EffectManager::Instance().Update(elapsedTime);
}

// �`�揈��
void SceneGame::Render()
{
	Camera& camera = Camera::Instance();
	ShadowMap* shadowMap = Graphics::Instance().GetShadowMap();
	
	RenderContext rc;
	rc.timer = ++waterTimer;

	// �J�����X�V����
	rc.view = camera.GetView();
	rc.projection = camera.GetProjection();

	// �`��R���e�L�X�g�ݒ�
	rc.camera = &camera;
	rc.deviceContext = Graphics::Instance().GetDeviceContext();
	rc.renderState = Graphics::Instance().GetRenderState();
	LightManager::Instane().PushRenderContext(rc); // ���C�g�̏����l�ߍ���
	rc.shadowMap = shadowMap;
	rc.shadowColor = SCENEGAME_shadow_color;

	// �V���h�E�}�b�v�`��
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

	// �[�x�e�X�g�Ȃ����[�x�������݂Ȃ�
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::NoTestNoWrite), 0);

	ID3D11RenderTargetView* rtv = graphics.GetRenderTargetView();
	ID3D11DepthStencilView* dsv = graphics.GetDepthStencilView();

	// ��ʃN���A�������_�[�^�[�Q�b�g�ݒ�
	FLOAT color[] = { 0.0f, 0.0f, 0.5f, 1.0f };	// RGBA(0.0�`1.0)
	dc->ClearRenderTargetView(rtv, color);
	dc->ClearDepthStencilView(dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	dc->OMSetRenderTargets(1, &rtv, dsv);

	float screenWidth = static_cast<float>(graphics.GetScreenWidth());
	float screenHeight = static_cast<float>(graphics.GetScreenHeight());
	// �r���[�|�[�g�̐ݒ�
	D3D11_VIEWPORT vp = {};
	vp.Width = graphics.GetScreenWidth();
	vp.Height = graphics.GetScreenHeight();
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	dc->RSSetViewports(1, &vp);

	// �X�J�C�{�b�N�X�̕`��
	skyBox->Render(rc);

	// 3D���f���`��
	ModelShader* shader = Graphics::Instance().GetShader(ShaderId::Toon);
	shader->Begin(rc);
	PlayerManager::Instance().Render3d(rc, shader);
	stage->TerrainRender(rc, shader);
	EnemyManager::Instance().Render(rc, shader);
	shader->End(rc);

	// ���ʕ`��
	ModelShader* waterShader = Graphics::Instance().GetShader(ShaderId::WaterSurface);
	waterShader->Begin(rc);
	stage->WaterRender(rc, waterShader);
	waterShader->End(rc);

	//3D�G�t�F�N�g�`��
	EffectManager::Instance().Render(rc.view, rc.projection);

	// �u�����h�X�e�[�g�ݒ�
	FLOAT blendFactor[4] = { 1.0f,1.0f,1.0f,1.0f };
	UINT sampleMask = 0xFFFFFFFF;
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Transparency), blendFactor, sampleMask);

	// 2D�X�v���C�g�`��
	{
		// �G�l�~�[HP
		RenderEnemyGauge(dc, rc.view, rc.projection);
		// �v���C���[2DRender
		PlayerManager::Instance().Render2d(rc, gauge.get(), font.get(), 
			spriteButtonFrame.get(),
			spriteWeaponIcon.get(),
			spriteNotification.get()
		);
		// �{�^��UI
		RenderButtonUI(dc);
		// �~�b�V����UI
		RenderMissionUI(dc);
	}

#if _DEBUG
	// gizmos
	Gizmos* gizmos = Graphics::Instance().GetGizmos();
	// �`����s
	gizmos->Render(rc);

	// 3D�f�o�b�O�`��
	{
		//�v���C���[�f�o�b�O�v���~�e�B�u�`��
		player1P->DrawDebugPrimitive();
		playerAI->DrawDebugPrimitive();
		//�G�l�~�[�f�o�b�O�v���~�e�B�u�`��
		EnemyManager::Instance().DrawDebugPrimitive();
	}

	//�f�o�b�O���j���[�`��
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

// �G�l�~�[HP�Q�[�W�`��
void SceneGame::RenderEnemyGauge(ID3D11DeviceContext* dc, const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection)
{
	//�r���[�|�[�g
	D3D11_VIEWPORT viewport;
	UINT numViewports = 1;
	dc->RSGetViewports(&numViewports, &viewport);

	//�ϊ��s��
	DirectX::XMMATRIX View = XMLoadFloat4x4(&view);
	DirectX::XMMATRIX Projection = XMLoadFloat4x4(&projection);
	DirectX::XMMATRIX World = XMMatrixIdentity();

	//�S�Ă̓G�̓����HP�Q�[�W��\��
	EnemyManager& enemyManager = EnemyManager::Instance();
	int enemyCount = enemyManager.GetEnemyCount();

	for (int i = 0; i < enemyCount; ++i)
	{
		Enemy* enemy = enemyManager.GetEnemy(i);
		//��������������continue
		if (Player1P::Instance().GetEachEnemyDist(enemy) > enemy_hp_gauge_display_dist) continue;

		//�G�l�~�[����̃��[���h���W
		DirectX::XMFLOAT3 worldPosition = enemy->GetPosition();
		worldPosition.y += enemy->GetHeight();
		DirectX::XMVECTOR WorldPosition = XMLoadFloat3(&worldPosition);

		//���[���h����X�N���[���ւ̕ϊ�
		DirectX::XMVECTOR ScreenPosition = XMVector3Project(
			WorldPosition,
			viewport.TopLeftX, viewport.TopLeftY,
			viewport.Width, viewport.Height,
			viewport.MinDepth, viewport.MaxDepth,
			Projection, View, World);

		DirectX::XMFLOAT3 screenPosition;
		DirectX::XMStoreFloat3(&screenPosition, ScreenPosition);
		//�J�����̔w��ɂ��邩�A���炩�ɗ���Ă���Ȃ�`�悵�Ȃ�
		if (screenPosition.z < 0.0f || screenPosition.z > 1.0f) continue;

		//HP�p�[�Z���e�[�W
		float healthRate = enemy->GetHealthRate() / 100.0f; //�S�������珬���ɕϊ�

		//�Q�[�W�`��(���n)
		gauge->Render(dc,
			screenPosition.x - enemy_hp_gauge_size.x / 2 - enemy_hp_gauge_frame_expansion / 2, // X_�����ɔz�u���邽�ߕ��̔����ƃQ�[�W�����̔����ŋ��߂�
			screenPosition.y - enemy_hp_gauge_size.y - enemy_hp_gauge_frame_expansion / 2, // Y_�㉺�̊g�������킹���T�C�Y���ő����Ă��邽�ߔ�������
			SPRITE_position_default_z,
			enemy_hp_gauge_size.x + enemy_hp_gauge_frame_expansion,
			enemy_hp_gauge_size.y + enemy_hp_gauge_frame_expansion,
			SPRITE_angle_default,
			enemy_hp_gauge_frame_color
		);
		//�Q�[�W�`��
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

// �{�^��UI�`��
void SceneGame::RenderButtonUI(ID3D11DeviceContext* dc)
{
	float spriteCutPosition_x = 0;

	// X�F�� Player
	{
		// �{�^���t���[���`��
		spriteButtonFrame->Render(dc,
			{ ButtonFrame_button_X_position_x, ButtonFrame_button_XB_position_y, SPRITE_position_default_z },
			ButtonFrame_size,
			SPRITE_cut_position_default, ButtonFrame_sprite_size,
			SPRITE_angle_default, SPRITE_color_default);

		auto nextWeapon1P = Player1P::Instance().GetNextWeapon();
		if (nextWeapon1P != Player1P::Instance().GetCurrentUseWeapon()) // �������݂̕��큁���킪��������������Ă��Ȃ����͕\�����Ȃ�
		{
			// ����ɑΉ�����J�b�g�ʒu
			spriteCutPosition_x = SpriteCutPositionX(nextWeapon1P);

			// �{�^���A�C�R���`��
			spriteWeaponIcon->Render(dc,
				{ ButtonFrame_button_X_position_x + ButtonIcon_frame_to_icon_offset.x, ButtonFrame_button_XB_position_y + ButtonIcon_frame_to_icon_offset.y, SPRITE_position_default_z },
				ButtonIcon_render_size,
				{ spriteCutPosition_x, ButtonFrame_sprite_size.y },
				ButtonFrame_sprite_size,
				SPRITE_angle_default,
				SPRITE_color_default);
		}
		// ���O�`��
		spriteNameOnButton->Render(dc,
			{ ButtonFrame_button_X_position_x + ButtonIcon_frame_to_icon_offset.x, ButtonFrame_button_XB_position_y + ButtonIcon_frame_to_icon_offset.y, SPRITE_position_default_z },
			ButtonName_render_size,
			SPRITE_cut_position_default,
			ButtonName_sprite_size,
			SPRITE_angle_default,
			SPRITE_color_default);

	}
	// Y�F�� Buddy
	{
		// �{�^���t���[���`��
		spriteButtonFrame->Render(dc,
			{ ButtonFrame_button_YA_position_x, ButtonFrame_button_Y_position_y, SPRITE_position_default_z },
			ButtonFrame_size,
			SPRITE_cut_position_default, ButtonFrame_sprite_size,
			SPRITE_angle_default, SPRITE_color_default);

		bool enable_show = true;
		// �񕜑J�ډ\�ȏꍇ
		if (Player1P::Instance().GetEnableRecoverTransition())
		{
			spriteCutPosition_x = ButtonFrame_sprite_size.x * ButtonIcon_cut_rate_recover;
		}
		// ����ύX
		else
		{
			auto nextWeaponAI = PlayerAI::Instance().GetNextWeapon();
			if (nextWeaponAI == PlayerAI::Instance().GetCurrentUseWeapon())
			{
				// �������݂̕��큁���킪��������������Ă��Ȃ����͕\�����Ȃ�
				enable_show = false;
			}
			else
			{
				// ����ɑΉ�����J�b�g�ʒu
				spriteCutPosition_x = SpriteCutPositionX(nextWeaponAI);
			}
		}
		if (enable_show)
		{
			// �{�^���A�C�R���`��
			spriteWeaponIcon->Render(dc,
				{ ButtonFrame_button_YA_position_x + ButtonIcon_frame_to_icon_offset.x, ButtonFrame_button_Y_position_y + ButtonIcon_frame_to_icon_offset.y, SPRITE_position_default_z },
				ButtonIcon_render_size,
				{ spriteCutPosition_x, ButtonFrame_sprite_size.y },
				ButtonFrame_sprite_size,
				SPRITE_angle_default,
				SPRITE_color_default);
		}
		// ���O�`��
		spriteNameOnButton->Render(dc,
			{ ButtonFrame_button_YA_position_x + ButtonIcon_frame_to_icon_offset.x, ButtonFrame_button_Y_position_y + ButtonIcon_frame_to_icon_offset.y, SPRITE_position_default_z },
			ButtonName_render_size,
			{ SPRITE_cut_position_default.x, ButtonName_sprite_size.y },
			ButtonName_sprite_size,
			SPRITE_angle_default,
			SPRITE_color_default);
	}
	// B�F�E Attack
	{
		// �{�^���t���[���`��
		spriteButtonFrame->Render(dc,
			{ ButtonFrame_button_B_position_x, ButtonFrame_button_XB_position_y, SPRITE_position_default_z },
			ButtonFrame_size,
			SPRITE_cut_position_default, ButtonFrame_sprite_size,
			SPRITE_angle_default, SPRITE_color_default);

		// ����ɑΉ�����J�b�g�ʒu
		spriteCutPosition_x = SpriteCutPositionX(Player1P::Instance().GetCurrentUseWeapon());
		// �{�^���A�C�R���`��
		spriteWeaponIcon->Render(dc,
			{ ButtonFrame_button_B_position_x + ButtonIcon_frame_to_icon_offset.x, ButtonFrame_button_XB_position_y + ButtonIcon_frame_to_icon_offset.y, SPRITE_position_default_z },
			ButtonIcon_render_size,
			{ spriteCutPosition_x, SPRITE_cut_position_default.y },
			ButtonFrame_sprite_size,
			SPRITE_angle_default,
			SPRITE_color_default);
	}
	// A�F�� Jump
	{
		// �{�^���t���[���`��
		spriteButtonFrame->Render(dc,
			{ ButtonFrame_button_YA_position_x, ButtonFrame_button_A_position_y, SPRITE_position_default_z },
			ButtonFrame_size,
			{ ButtonFrame_sprite_size.x, SPRITE_cut_position_default.y }, ButtonFrame_sprite_size,
			SPRITE_angle_default, SPRITE_color_default);

		// �{�^���A�C�R���`��
		spriteWeaponIcon->Render(dc,
			{ ButtonFrame_button_YA_position_x + ButtonIcon_frame_to_icon_offset.x, ButtonFrame_button_A_position_y + ButtonIcon_frame_to_icon_offset.y, SPRITE_position_default_z },
			ButtonIcon_render_size,
			SPRITE_cut_position_default,
			ButtonFrame_sprite_size,
			SPRITE_angle_default,
			SPRITE_color_default);
	}
}
// ����ɑΉ�����X���W�X�v���C�g�J�b�g�ʒu��Ԃ�
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

// �~�b�V����UI�`��
void SceneGame::RenderMissionUI(ID3D11DeviceContext* dc)
{
	// �~�b�V�����t���[���`��
	spriteMissionFrame->Render(dc,
		Mission_position,
		Mission_render_size,
		SPRITE_cut_position_default, 
		Mission_sprite_size,
		SPRITE_angle_default, SPRITE_color_default);

	missionSpriteTimer++;
	if (missionSpriteTimer > Mission_display_timer_max)
	{
		// �^�C�}�[���Z�b�g
		missionSpriteTimer = 0;
		// AI��HP���낤�����
		if (PlayerAI::Instance().GetHpWorning())
		{
			missionSpriteNumber = Mission::Recover;
		}
		else
		{
			// 1P��AI����������������Ă��邩
			bool have = PlayersHaveAnySameWeapon();
			// �\���ł���Ō�̔ԍ��܂ŗ��Ă�����
			if ((have && missionSpriteNumber == Mission::SpecialAttack) ||
				(!have && missionSpriteNumber == Mission::UpperAttack))
			{
				// ���߂̔ԍ��ɖ߂�
				missionSpriteNumber = Mission::GetWeapon;
			}
			else
			{
				// ���̔ԍ��ɂ���
				missionSpriteNumber = static_cast<Mission>(SC_INT(missionSpriteNumber) + 1);
			}
		}
	}

	// �~�b�V�����e�L�X�g�`��
	spriteMissionText->Render(dc,
		Mission_position,
		Mission_render_size,
		missionSpriteCutPosition[SC_INT(missionSpriteNumber)],
		Mission_sprite_size,
		SPRITE_angle_default, SPRITE_color_default);
}

#pragma region DEBUG_DRAW
//�V���h�E�}�b�vGUI�`��
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

//�V�[��GUI�`��
void SceneGame::DrawSceneGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	if (ImGui::Begin("Scene", nullptr, ImGuiWindowFlags_None))
	{
		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//�ʒu
			ImGui::DragFloat3("Position", &position.x, 0.1f);

			//��]
			XMFLOAT3 a = {};
			a.x = XMConvertToDegrees(angle.x);
			a.y = XMConvertToDegrees(angle.y);
			a.z = XMConvertToDegrees(angle.z);
			ImGui::DragFloat3("Angle", &a.x, 1.0f);
			angle.x = XMConvertToRadians(a.x);
			angle.y = XMConvertToRadians(a.y);
			angle.z = XMConvertToRadians(a.z);

			//�X�P�[��
			ImGui::DragFloat3("Scale", &scale.x, 0.01f);
		}

		if (ImGui::CollapsingHeader("Hierarchy", ImGuiTreeNodeFlags_DefaultOpen))
		{
			//�m�[�h�c���[���ċA�I�ɕ`�悷��֐�
			std::function<void(Model::Node*)> drawNodeTree = [&](Model::Node* node)
			{
				//�����N���b�N�A�܂��̓m�[�h���_�u���N���b�N�ŊK�w���J��
				ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_OpenOnArrow
					| ImGuiTreeNodeFlags_OpenOnDoubleClick;

				//�q�����Ȃ��ꍇ�͖���t���Ȃ�
				size_t childCount = node->children.size();
				if (childCount == 0)
				{
					nodeFlags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
				}

				//�I���t���O
				if (selectionNode == node)
				{
					nodeFlags |= ImGuiTreeNodeFlags_Selected;
				}

				//�c���[�m�[�h��\��
				bool opened = ImGui::TreeNodeEx(node, nodeFlags, node->name.c_str());

				//�t�H�[�J�X���ꂽ�m�[�h��I��
				if (ImGui::IsItemFocused())
				{
					selectionNode = node;
				}

				//�J����Ă���ꍇ�A�q�K�w�������������s��
				if (opened && childCount > 0)
				{
					for (Model::Node* child : node->children)
					{
						drawNodeTree(child);
					}
					ImGui::TreePop();
				}
			};
			//�ċA�I�Ƀm�[�h��`��
			//drawNodeTree(model->GetRootNode());
		}

		ImGui::End();
	}
}

//�v���p�e�BGUI�`��
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
			//�ʒu
			ImGui::DragFloat3("Position", &selectionNode->position.x, 0.1f);

			//��]
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
			//�X�P�[��
			ImGui::DragFloat3("Scale", &selectionNode->scale.x, 0.01f);
		}
	}

	ImGui::End();
}
#pragma endregion
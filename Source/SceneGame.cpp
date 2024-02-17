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
		XMFLOAT3(0, 10, -10),	//���_
		XMFLOAT3(0, 0, 0),		//�����_
		XMFLOAT3(0, 1, 0)		//��x�N�g��
	);
	//freeCameraController.SyncCameraToController(camera);

	cameraController = std::make_unique<CameraController>();

	//���f���쐬
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

	//�X�J�C�{�b�N�X
	skyBox = std::make_unique<SkyBox>("Data/Texture/incskies_002_8k.png");

	//sprites[1] = std::make_unique<Sprite>(device);
	sprites[0] = std::make_unique<Sprite>(device, "Data/Sprite/button.png");
	sprites[1] = std::make_unique<Sprite>(device, "Data/Sprite/item.png");
	sprites[2] = std::make_unique<Sprite>(device, "Data/Sprite/item2.png");
	sprites[3] = std::make_unique<Sprite>(device, "Data/Sprite/weapon.png");
	font = std::make_unique<FontSprite>(device, "Data/Font/font6.png", 256);
	//�Q�[�W�X�v���C�g
	gauge = std::make_unique<Sprite>(device);

	//�G�l�~�[������
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

	// ���s������ǉ�
	mainDirectionalLight = new Light(LightType::Directional);
	mainDirectionalLight->SetDirection({ 1, -1, -2 });
	LightManager::Instane().Register(mainDirectionalLight);
}

// �I����
void SceneGame::Finalize()
{
	//�G�l�~�[�I����
	EnemyManager::Instance().Clear();
}

void SceneGame::Update(float elapsedTime)
{
	//�J�����R���g���[���[�X�V����
	XMFLOAT3 target = player1P->GetPosition();
	target.y += 0.5f;
	cameraController->SetTarget(target);
	cameraController->Update(elapsedTime);

	//�J�����X�V����
	//Camera& camera = Camera::Instance();
	//freeCameraController.Update();
	//freeCameraController.SyncControllerToCamera(camera);

	// �}�l�[�W���[�ɂ��X�V
	PlayerManager::Instance().Update(elapsedTime);
	EnemyManager::Instance().Update(elapsedTime);
	stage->Update(elapsedTime);
	//�G�t�F�N�g�X�V����
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

	//�J�����X�V����
	rc.view = camera.GetView();
	rc.projection = camera.GetProjection();

	//�`��R���e�L�X�g�ݒ�
	rc.camera = &camera;
	rc.deviceContext = Graphics::Instance().GetDeviceContext();
	rc.renderState = Graphics::Instance().GetRenderState();
	LightManager::Instane().PushRenderContext(rc);// ���C�g�̏����l�ߍ���
	rc.shadowMap = shadowMap;
	rc.shadowColor = { 0.5f,0.5f,0.5f };

	//�V���h�E�}�b�v�`��
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

	//3D�G�t�F�N�g�`��
	EffectManager::Instance().Render(rc.view, rc.projection);

	// 2D�X�v���C�g�`��
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

		//Item�g
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
	//�`����s
	gizmos->Render(rc);

	//Loading�ׂ̈����Ńu�����h�X�e�[�g�ύX
	FLOAT blendFactor[4] = { 1.0f,1.0f,1.0f,1.0f };
	UINT sampleMask = 0xFFFFFFFF;
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Transparency), blendFactor, sampleMask);

#if showDebug//def _DEBUG
	// 3D�f�o�b�O�`��
	{
		//�v���C���[�f�o�b�O�v���~�e�B�u�`��
		player1P->DrawDebugPrimitive();
		playerAI->DrawDebugPrimitive();
		//�G�l�~�[�f�o�b�O�v���~�e�B�u�`��
		EnemyManager::Instance().DrawDebugPrimitive();
	}

	//�f�o�b�O���j���[�`��
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

// �G�l�~�[HP�Q�[�W�`��
void SceneGame::RenderEnemyGauge(ID3D11DeviceContext* dc, const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection)
{
	//�r���[�|�[�g
	D3D11_VIEWPORT viewport;
	UINT numViewports = 1;
	dc->RSGetViewports(&numViewports, &viewport);

	//�ϊ��s��
	XMMATRIX View = XMLoadFloat4x4(&view);
	XMMATRIX Projection = XMLoadFloat4x4(&projection);
	XMMATRIX World = XMMatrixIdentity();

	//�S�Ă̓G�̓����HP�Q�[�W��\��
	EnemyManager& enemyManager = EnemyManager::Instance();
	int enemyCount = enemyManager.GetEnemyCount();

	for (int i = 0; i < enemyCount; ++i)
	{
		Enemy* enemy = enemyManager.GetEnemy(i);

		//�G�l�~�[����̃��[���h���W
		XMFLOAT3 worldPosition = enemy->GetPosition();
		worldPosition.y += enemy->GetHeight();
		XMVECTOR WorldPosition = XMLoadFloat3(&worldPosition);

		//���[���h����X�N���[���ւ̕ϊ�
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

		//�J�����̔w��ɂ��邩�A���炩�ɗ���Ă���Ȃ�`�悵�Ȃ�
		if (screenPosition.z < 0.0f || screenPosition.z > 1.0f) continue;

		//HP�Q�[�W�̒���
		const float guageWidth = 60.0f;
		const float guageHeight = 8.0f;

		float healthRate = enemy->GetHealth() / static_cast<float>(enemy->GetMaxHealth());

		//�Q�[�W�`��(���n)
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
		//�Q�[�W�`��
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

// �L�����N�^�[���O�`��
void SceneGame::RenderCharacterName(const RenderContext& rc, const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection)
{
	ID3D11DeviceContext* dc = rc.deviceContext;

	//�r���[�|�[�g
	D3D11_VIEWPORT viewport;
	UINT numViewports = 1;
	dc->RSGetViewports(&numViewports, &viewport);

	//�ϊ��s��
	XMMATRIX View = XMLoadFloat4x4(&view);
	XMMATRIX Projection = XMLoadFloat4x4(&projection);
	XMMATRIX World = XMMatrixIdentity();

	{
		Player1P& player = Player1P::Instance();

		//Player����̃��[���h���W
		XMFLOAT3 worldPosition = player.GetPosition();
		worldPosition.y += player.GetHeight() + 0.4f;
		XMVECTOR WorldPosition = XMLoadFloat3(&worldPosition);

		//���[���h����X�N���[���ւ̕ϊ�
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
		//HP�Q�[�W�̒���
		const float guageWidth = 60.0f;
		const float guageHeight = 8.0f;
		float screenWidth = Graphics::Instance().GetScreenWidth();
		float screenHeight = Graphics::Instance().GetScreenHeight();


		//�J�����̔w��ɂ��邩�A���炩�ɗ���Ă���Ȃ�`�悵�Ȃ�
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

		//Player����̃��[���h���W
		XMFLOAT3 worldPosition = player.GetPosition();
		worldPosition.y += player.GetHeight() + 0.4f;
		XMVECTOR WorldPosition = XMLoadFloat3(&worldPosition);

		//���[���h����X�N���[���ւ̕ϊ�
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
		//HP�Q�[�W�̒���
		const float guageWidth = 60.0f;
		const float guageHeight = 8.0f;
		float screenWidth = Graphics::Instance().GetScreenWidth();
		float screenHeight = Graphics::Instance().GetScreenHeight();

		//�J�����̔w��ɂ��邩�A���炩�ɗ���Ă���Ȃ�`�悵�Ȃ�
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
			XMFLOAT3 a;
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
			//�X�P�[��
			ImGui::DragFloat3("Scale", &selectionNode->scale.x, 0.01f);
		}
	}

	ImGui::End();
}
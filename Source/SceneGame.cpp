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
	player = std::make_unique<Player>();

	sprites[0] = std::make_unique<Sprite>(device, "Data/Sprite/view.jpg");
	//sprites[1] = std::make_unique<Sprite>(device);
	//sprites[2] = std::make_unique<Sprite>(device, "Data/Sprite/3.png");
	//sprites[3] = std::make_unique<Sprite>(device, "Data/Sprite/4.png");
	font = std::make_unique<FontSprite>(device, "Data/Font/font6.png", 256);
	//�Q�[�W�X�v���C�g
	gauge = std::make_unique<Sprite>(device);

	//�G�l�~�[������
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

	//���C�g�ݒ�
	DirectionalLight directionalLight;
	directionalLight.direction = { 2, -4, 3 };
	directionalLight.color = { 1,1,1 };
	lightManager.SetDirectionalLight(directionalLight);
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
	XMFLOAT3 target = player->GetPosition();
	ViewPosition.x = -(target.x + 20.0f); //�w�i�X���C�h
	target.y += 0.5f;
	cameraController->SetTarget(target);
	cameraController->Update(elapsedTime);

	player->Update(elapsedTime,1);

	//�G�l�~�[�X�V
	EnemyManager::Instance().Update(elapsedTime);
	stage->Update(elapsedTime);
	//�G�t�F�N�g�X�V����
	EffectManager::Instance().Update(elapsedTime);
}

void SceneGame::Render()
{
	Camera& camera = Camera::Instance();
	ShadowMap* shadowMap = Graphics::Instance().GetShadowMap();
	
	RenderContext rc;
	//�J�����X�V����
	rc.view = camera.GetView();
	rc.projection = camera.GetProjection();

	//�`��R���e�L�X�g�ݒ�
	rc.camera = &camera;
	rc.deviceContext = Graphics::Instance().GetDeviceContext();
	rc.renderState = Graphics::Instance().GetRenderState();
	rc.lightManager = &lightManager;
	rc.shadowMap = shadowMap;
	rc.shadowColor = { 0.5f,0.5f,0.5f };

	//�V���h�E�}�b�v�`��
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

	//	// �|���S���`��
	//	primitiveRenderer->Render(rc.deviceContext, rc.camera->GetView(), rc.camera->GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
	//	// �O���b�h�`��
	//	primitiveRenderer->DrawGrid(20, 1);
	//	primitiveRenderer->Render(rc.deviceContext, rc.camera->GetView(), rc.camera->GetProjection(), D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	//}

	//3D�G�t�F�N�g�`��
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
	// 2D�X�v���C�g�`��
	{
		RenderEnemyGauge(dc, rc.view, rc.projection);
	}


	font->Textout(rc, "Unity-Chan:", 16, 0, 1.0f, { -10, 10, 0 }, 32, 32, 32, 32, 16, 16, 0, 1, 1, 1, 1);
	font->Textout(rc, "Time:" + std::to_string((int)gameTimer), 16, 0, 1.0f, { 910, 10, 0 }, 32, 32, 32, 32, 16, 16, 0, 1, 1, 1, 1);
	// 3D�f�o�b�O�`��
	{
		//�v���C���[�f�o�b�O�v���~�e�B�u�`��
		player->DrawDebugPrimitive();
		//�G�l�~�[�f�o�b�O�v���~�e�B�u�`��
		EnemyManager::Instance().DrawDebugPrimitive();
	}

	//�f�o�b�O���j���[�`��
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

	//�f�o�b�O���j���[�`��
	{
		ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
		ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_FirstUseEver);
		ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

		if (ImGui::Begin("Player", nullptr, ImGuiWindowFlags_None))
		{
			if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
			{
				//�X�P�[��
				ImGui::DragInt("playCount", &playCount, 1);
			}

			ImGui::End();
		}
	}
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


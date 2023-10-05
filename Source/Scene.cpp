#include <functional>
#include "Scene.h"
#include "Graphics/Graphics.h"
#include "imgui.h"
#include "TransformUtils.h"




//�R���X�g���N�^
SpriteTestScene::SpriteTestScene()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();

	sprites[0] = std::make_unique<Sprite>(device, "Data/Sprite/player-sprites.png");
	sprites[1] = std::make_unique<Sprite>(device);
}

//�`�揈��
void SpriteTestScene::Render()
{
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
	RenderState* renderState = Graphics::Instance().GetRenderState();

	ID3D11SamplerState* samplers[] =
	{
		renderState->GetSamplerState(SamplerState::PointClamp)
	};
	dc->PSSetSamplers(0, _countof(samplers), samplers);

	float width = 140;
	float height = 240;
	sprites[0]->Render(dc, 100, 50, 0, width, height, 10 * width, 3 * height, width, height, 0, 1, 1, 1, 1);
	sprites[0]->Render(dc, 100, 350, 0, 480, 300, 0, 1, 1, 1, 1);
	sprites[1]->Render(dc, 300, 50, 0, width, height, 0, 0, 1, 0, 1);
}

//�R���X�g���N�^
DepthTestScene::DepthTestScene()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();

	sprite = std::make_unique<Sprite>(device);
}

//�`�揈��
void DepthTestScene::Render()
{
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
	RenderState* renderState = Graphics::Instance().GetRenderState();

	ID3D11SamplerState* samplers[] =
	{
		renderState->GetSamplerState(SamplerState::PointClamp)
	};
	dc->PSSetSamplers(0, _countof(samplers), samplers);

	//�[�x�������݂Ȃ����[�x�e�X�g�Ȃ�
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::NoTestNoWrite), 0);
	sprite->Render(dc, 50, 50, 0.0f, 100, 100, 0, 1, 0, 0, 0);
	sprite->Render(dc, 100, 100, 1.0f, 100, 100, 0, 0, 1, 0, 0);
	sprite->Render(dc, 150, 150, 0.5f, 100, 100, 0, 1, 1, 0, 0);

	//�[�x�������݂��聕�[�x�e�X�g����
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
	sprite->Render(dc, 350, 50, 0.0f, 100, 100, 0, 1, 0, 0, 0);
	sprite->Render(dc, 400, 100, 1.0f, 100, 100, 0, 0, 1, 0, 0);
	sprite->Render(dc, 450, 150, 0.5f, 100, 100, 0, 1, 1, 0, 0);

	//�[�x�������݂̂�
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::WriteOnly), 0);
	sprite->Render(dc, 650, 50, 0.0f, 100, 100, 0, 1, 0, 0, 0);
	//�[�x�e�X�g�̂�
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::TestOnly), 0);
	sprite->Render(dc, 700, 100, 0.5f, 100, 100, 0, 1, 1, 0, 0);
	//�[�x�������݂̂�
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::WriteOnly), 0);
	sprite->Render(dc, 750, 150, 1.0f, 100, 100, 0, 0, 1, 0, 0);
}

// �R���X�g���N�^
BlendTestScene::BlendTestScene()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();

	sprite = std::make_unique<Sprite>(device);
}

// �`�揈��
void BlendTestScene::Render()
{
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
	RenderState* renderState = Graphics::Instance().GetRenderState();

	ID3D11SamplerState* samplers[] =
	{
		renderState->GetSamplerState(SamplerState::PointClamp)
	};
	dc->PSSetSamplers(0, _countof(samplers), samplers);

	// �[�x�e�X�g�Ȃ����[�x�������݂Ȃ�
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::NoTestNoWrite), 0);

	FLOAT blendFactor[4] = { 1.0f,1.0f,1.0f,1.0f };
	UINT sampleMask = 0xFFFFFFFF;

	//���Z�����e�X�g
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), blendFactor, sampleMask);
	sprite->Render(dc, 50, 50, 0.0f, 150, 150, 0, 0, 0, 0, 1.0f);
	sprite->Render(dc, 50, 50, 0.0f, 100, 100, 0, 1, 0, 0, 1.0f);
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Additive), blendFactor, sampleMask);
	sprite->Render(dc, 100, 100, 0.0f, 100, 100, 0, 0, 1, 0, 1.0f);

	//���Z�����e�X�g
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), blendFactor, sampleMask);
	sprite->Render(dc, 250, 50, 0.0f, 150, 150, 0, 1, 1, 1, 1.0f);
	sprite->Render(dc, 250, 50, 0.0f, 100, 100, 0, 1, 1, 0, 1.0f);
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Subtraction), blendFactor, sampleMask);
	sprite->Render(dc, 300, 100, 0.0f, 100, 100, 0, 0, 1, 0, 1.0f);

	//��Z�����e�X�g
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), blendFactor, sampleMask);
	sprite->Render(dc, 450, 50, 0.0f, 150, 150, 0, 1, 1, 1, 1.0f);
	sprite->Render(dc, 450, 50, 0.0f, 100, 100, 0, 1, 1, 0, 1.0f);
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Subtraction), blendFactor, sampleMask);
	sprite->Render(dc, 500, 100, 0.0f, 100, 100, 0, 0.5f, 0.5f, 0.5f, 1.0f);

	//���������e�X�g
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), blendFactor, sampleMask);
	sprite->Render(dc, 650, 50, 0.0f, 150, 150, 0, 1, 1, 1, 0.5f);
	sprite->Render(dc, 650, 50, 0.0f, 100, 100, 0, 1, 0, 0, 0.5f);
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Transparency), blendFactor, sampleMask);
	sprite->Render(dc, 700, 100, 0.0f, 100, 100, 0, 0, 0, 1, 0.5f);
}

RasterizeTestScene::RasterizeTestScene()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();

	sprite = std::make_unique<Sprite>(device);
}

void RasterizeTestScene::Render()
{
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();
	RenderState* renderState = Graphics::Instance().GetRenderState();

	ID3D11SamplerState* samplers[] =
	{
		renderState->GetSamplerState(SamplerState::PointClamp)
	};
	dc->PSSetSamplers(0, _countof(samplers), samplers);

	// �[�x�e�X�g�Ȃ����[�x�������݂Ȃ�
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::NoTestNoWrite), 0);

	//�x�^�h�聕�J�����O�Ȃ�
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullNone));
	sprite->Render(dc, 50, 50, 0.0f, 100, 100, 0, 1, 0, 0, 1);
	sprite->Render(dc, 150, 200, 0.0f, -100, 100, 0, 1, 0, 0, 1);

	//�x�^�h�聕���ʃJ�����O
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullBack));
	sprite->Render(dc, 200, 50, 0.0f, 100, 100, 0, 1, 0, 0, 1);
	sprite->Render(dc, 300, 200, 0.0f, -100, 100, 0, 1, 0, 0, 1);

	//���C���[�t���[�����J�����O�Ȃ�
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::WireCullNone));
	sprite->Render(dc, 350, 50, 0.0f, 100, 100, 0, 1, 0, 0, 1);
	sprite->Render(dc, 450, 200, 0.0f, -100, 100, 0, 1, 0, 0, 1);

	//���C���[�t���[�����J�����O�Ȃ�
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::WireCullBack));
	sprite->Render(dc, 500, 50, 0.0f, 100, 100, 0, 1, 0, 0, 1);
	sprite->Render(dc, 600, 200, 0.0f, -100, 100, 0, 1, 0, 0, 1);
}

//�R���X�g���N�^
GizmosTestScene::GizmosTestScene()
{
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
		{ 0,1,-5 },//���_
		{ 0,0,0 },//�����_
		{ 0,1,0 }//��x�N�g��
	);
}

void GizmosTestScene::Update(float elapsedTime)
{	
	//��]����
	rotation += 0.5f * elapsedTime;
}

//�`�揈��
void GizmosTestScene::Render()
{
	Gizmos* gizmos = Graphics::Instance().GetGizmos();

	//���`��
	gizmos->DrawBox(
		{ 0,0,0 },			//�ʒu
		{ 0,rotation,0 },	//��]
		{ 1,1,1 },			//�T�C�Y
		{ 1,1,1,1 });		//�F

	//���`��
	gizmos->DrawSphere(
		{ 2,0,0 },		//�ʒu
		1.0f,			//���a
		{ 1,0,0,1 });	//�F

	Camera& camera = Camera::Instance();
	//�`��R���e�L�X�g�ݒ�
	RenderContext rc;
	rc.camera = &camera;
	rc.deviceContext = Graphics::Instance().GetDeviceContext();
	rc.renderState = Graphics::Instance().GetRenderState();

	//�`����s
	gizmos->Render(rc);
}


//�R���X�g���N�^
void ModelTestScene::Initialize()
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
		{ 20,20,20 },	//���_
		{ 0,12,0 },	//�����_
		{ 0,1,0 }	//��x�N�g��
	);
	cameraController.SyncCameraToController(camera);

	//���f���쐬
	//model = std::make_unique<Model>(device, "Data/Model/Plantune/plantune.fbx", 0.01f);
	//model = std::make_unique<Model>(device, "Data/Model/Polca/polca.pmx");
	model = std::make_unique<Model>(device, "Data/Model/Forest/Free_Forest.fbx", 0.5f);
	model->PlayAnimation(0, true);

	//���C�g�ݒ�
	DirectionalLight directionalLight;
	directionalLight.direction = { 1, -1, 0 };
	directionalLight.color = { 1,1,1 };
	lightManager.SetDirectionalLight(directionalLight);
}

void ModelTestScene::Update(float elapsedTime)
{
	Camera& camera = Camera::Instance();
	//�J�����X�V����
	cameraController.Update();
	cameraController.SyncControllerToCamera(camera);

	//���[���h�s��v�Z
	XMMATRIX S = XMMatrixScaling(scale.x, scale.y, scale.z);
	XMMATRIX R = XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z);
	XMMATRIX T = XMMatrixTranslation(position.x, position.y, position.z);
	XMFLOAT4X4 worldTramsform;
	XMStoreFloat4x4(&worldTramsform, S * R * T);

	//�A�j���[�V�����X�V
	model->UpdateAnimation(elapsedTime);
	//�g�����X�t�H�[���X�V
	model->UpdateTransform(worldTramsform);

}

//�`�揈��
void ModelTestScene::Render()
{
	//�`��R���e�L�X�g�ݒ�
	Camera& camera = Camera::Instance();
	ShadowMap* shadowMap = Graphics::Instance().GetShadowMap();

	RenderContext rc;
	rc.camera = &camera;
	rc.deviceContext = Graphics::Instance().GetDeviceContext();
	rc.renderState = Graphics::Instance().GetRenderState();
	rc.lightManager = &lightManager;
	rc.shadowMap = shadowMap;
	rc.shadowColor = { 0.5f,0.5f,0.5f };

	//�V���h�E�}�b�v�`��
	shadowMap->Begin(rc, camera.GetFocus());
	shadowMap->Draw(rc, model.get());
	shadowMap->End(rc);

	//�`��
	ModelShader* shader = Graphics::Instance().GetShader(ShaderId::Phong);
	shader->Begin(rc);
	shader->Draw(rc, model.get());
	shader->End(rc);

	//�f�o�b�O���j���[�`��
	DrawSceneGUI();
	DrawPropertyGUI();
	DrawAnimationGUI();
}

//�V�[��GUI�`��
void ModelTestScene::DrawSceneGUI()
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
			drawNodeTree(model->GetRootNode());
		}

		ImGui::End();
	}
}

//�v���p�e�BGUI�`��
void ModelTestScene::DrawPropertyGUI()
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

				XMStoreFloat4(&selectionNode->rotation, Rotation);
			}
			//�X�P�[��
			ImGui::DragFloat3("Scale", &selectionNode->scale.x, 0.01f);
		}
	}

	ImGui::End();
}

//�A�j���[�V����GUI�`��
void ModelTestScene::DrawAnimationGUI()
{
	ImGui::SetNextWindowPos(ImVec2(10,350), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	ImGui::Begin("Animation", nullptr, ImGuiWindowFlags_None);

	ImGui::Checkbox("Loop", &animationLoop);
	ImGui::DragFloat("BlendSec", &animationBlendSeconds, 0.01f);

	int index = 0;
	for (const Model::Animation& animation : model->GetAnimations())
	{
		ImGuiTreeNodeFlags nodeFlags = ImGuiTreeNodeFlags_Leaf;

		ImGui::TreeNodeEx(&animation, nodeFlags, animation.name.c_str());

		//�_�u���N���b�N�ŃA�j���[�V�����Đ�
		if (ImGui::IsItemClicked())
		{
			if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				model->PlayAnimation(index, animationLoop, animationBlendSeconds);
			}
		}

		ImGui::TreePop();
		index++;
	}

	ImGui::End();
}

//�R���X�g���N�^
PostEffectTestScene::PostEffectTestScene()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();

	sprite = std::make_unique<Sprite>(device, "Data/Sprite/screenshot.jpg");
	postEffect = std::make_unique<PostEffect>(device);
}

//�`�揈��
void PostEffectTestScene::Render()
{
	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();

	RenderState* renderState = Graphics::Instance().GetRenderState();
	float screenWidth = Graphics::Instance().GetScreenWidth();
	float screenHeight = Graphics::Instance().GetScreenHeight();

	//�u�����h�X�e�[�g�ݒ�
	FLOAT blendFactor[4] = { 1.0f,1.0f,1.0f,1.0f };
	dc->OMSetBlendState(renderState->GetBlendState(BlendState::Opaque), blendFactor, 0xFFFFFFFF);

	//�[�x�X�e���V���X�e�[�g�ݒ�
	dc->OMSetDepthStencilState(renderState->GetDepthStencilState(DepthState::NoTestNoWrite), 0);

	//���X�^���C�U�X�e�[�g�ݒ�
	dc->RSSetState(renderState->GetRasterizerState(RasterizerState::SolidCullNone));

	//�T���v���[�X�e�[�g�ݒ�
	ID3D11SamplerState* samplers[] =
	{
		renderState->GetSamplerState(SamplerState::LinearWrap)
	};
	dc->PSSetSamplers(0, _countof(samplers), samplers);

	//�t���[���o�b�t�@���擾
	FrameBuffer* displayFB = Graphics::Instance().GetFrameBuffer(FrameBufferId::Display);
	FrameBuffer* sceneFB = Graphics::Instance().GetFrameBuffer(FrameBufferId::Scene);
	FrameBuffer* luminanceFB = Graphics::Instance().GetFrameBuffer(FrameBufferId::Luminance);

	//�V�[���p�̃t���[���o�b�t�@�ɃX�v���C�g��`��
	sceneFB->SetRenderTarget(dc);

	//�X�v���C�g�`��
	sprite->Render(dc, 0, 0, 0, screenWidth, screenHeight, 0, 1, 1, 1, 1);

	//�`��R���e�L�X�g�ݒ�
	RenderContext rc;
	rc.deviceContext = Graphics::Instance().GetDeviceContext();
	rc.renderState = Graphics::Instance().GetRenderState();

	//�|�X�g�G�t�F�N�g�����J�n
	postEffect->Begin(rc);

	//�P�x���o����
	luminanceFB->SetRenderTarget(dc); //�P�x���o�p�̃t���[���o�b�t�@�ɕ`��
	postEffect->LuminanceExtraction(rc, sceneFB->GetColorMap());

	//�u���[������
	displayFB->SetRenderTarget(dc); //�o�b�N�o�b�t�@�Ƀu���[�������������ʂ�`��
	postEffect->Bloom(rc, sceneFB->GetColorMap(), luminanceFB->GetColorMap());

	//�I������
	postEffect->End(rc);

	//�f�o�b�OGUI�`��
	DrawPostEffectGUI();
}

//�|�X�g�G�t�F�N�gGUI�`��
void PostEffectTestScene::DrawPostEffectGUI()
{
	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	ImGui::Begin("PostEffect", nullptr, ImGuiWindowFlags_None);

	postEffect->DrawDebugGUI();

	ImGui::End();
}

void ShadowTestScene::Initialize()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	float screenWidth = Graphics::Instance().GetScreenWidth();
	float screenHeight = Graphics::Instance().GetScreenHeight();

	//�J�����ݒ�
	Camera& camera = Camera::Instance();
	camera.SetPerspectiveFov(
		XMConvertToRadians(45),		//��p
		screenWidth / screenHeight,	//��ʃA�X�y�N�g��
		0.1f,
		1000.0f
	);
	camera.SetLookAt(
		{ 0,10,20 },	//���_
		{ 0,0,0 },	//�����_
		{ 0,1,0 }	//��x�N�g��
	);
	cameraController.SyncCameraToController(camera);

	//���f���쐬
	stage = std::make_unique<Model>(device, "Data/Model/ExampleStage/ExampleStage.fbx");
	character = std::make_unique<Model>(device, "Data/Model/SD-UnityChan/UnityChan.fbx",0.02f);
	character->PlayAnimation(0, true);

	//���C�g�ݒ�
	DirectionalLight directionalLight;
	directionalLight.direction = { 1, -1, 0 };
	directionalLight.color = { 1,1,1 };
	lightManager.SetDirectionalLight(directionalLight);
}

void ShadowTestScene::Update(float elapsedTime)
{
	//�J�����X�V����
	Camera& camera = Camera::Instance();
	cameraController.Update();
	cameraController.SyncControllerToCamera(camera);

	//���[���h�s��v�Z
	XMFLOAT4X4 worldTramsform;
	XMStoreFloat4x4(&worldTramsform, XMMatrixIdentity());

	//�A�j���[�V�����X�V
	character->UpdateAnimation(elapsedTime);

	//�g�����X�t�H�[���X�V
	stage->UpdateTransform(worldTramsform);
	character->UpdateTransform(worldTramsform);
}

void ShadowTestScene::Render()
{
	Camera& camera = Camera::Instance();
	ShadowMap* shadowMap = Graphics::Instance().GetShadowMap();

	//�`��R���e�L�X�g�ݒ�
	RenderContext rc;
	rc.camera = &camera;
	rc.deviceContext = Graphics::Instance().GetDeviceContext();
	rc.renderState = Graphics::Instance().GetRenderState();
	rc.lightManager = &lightManager;
	rc.shadowMap = shadowMap;
	rc.shadowColor = { 0.5f,0.5f,0.5f };

	//�V���h�E�}�b�v�`��
	shadowMap->Begin(rc,camera.GetFocus());
	shadowMap->Draw(rc, stage.get());
	shadowMap->Draw(rc, character.get());
	shadowMap->End(rc);

	//�`��
	ModelShader* shader = Graphics::Instance().GetShader(ShaderId::Phong);
	shader->Begin(rc);
	shader->Draw(rc, stage.get());
	shader->Draw(rc, character.get());
	shader->End(rc);

	//GUI
	DrawShadowMapGUI();
}

void ShadowTestScene::DrawShadowMapGUI()
{
	ShadowMap* shadowMap = Graphics::Instance().GetShadowMap();

	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
	ImGui::SetNextWindowPos(ImVec2(pos.x + 10, pos.y + 10), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(300, 300), ImGuiCond_FirstUseEver);

	ImGui::Begin("ShadowMap", nullptr, ImGuiWindowFlags_None);

	ImGui::Image(shadowMap->GetShaderResourceView(), ImVec2(200, 200));

	ImGui::End();
}

#include "Graphics/Graphics.h"

#include "SceneManager.h"
#include "SceneTitle.h"
#include "SceneClear.h"
#include "SceneGame.h"
#include "SceneLoading.h"


#include "Input/Input.h"

void SceneClear::Initialize()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	sprite[0] = std::make_unique<Sprite>(device, "Data/Sprite/4.png");
	sprite[1] = std::make_unique<Sprite>(device, "Data/Sprite/5.png");
	//model = std::make_unique<Model>(device, "Data/Model/Jammo/Jammo_Player.fbx", 0.02f);
	model->PlayAnimation(static_cast<int>(Player::Animation::Running), true);

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
}

void SceneClear::Finalize()
{
}

void SceneClear::Update(float elapsedTime)
{
	GamePad& gamePad = Input::Instance().GetGamePad();

	const GamePadButton anyButton = GamePad::BTN_A | GamePad::BTN_B | GamePad::BTN_X | GamePad::BTN_Y| GamePad::BTN_START;
	if (gamePad.GetButtonDown() & anyButton)
	{
		SceneManager::Instance().ChangeScene(new SceneLoading(new SceneTitle(), -255));
	}

	if (scaleTimer > 91) isScaleDown = true;
	else if (scaleTimer < 89) isScaleDown = false;

	//if (isScaleDown) scaleTimer -= 0.03f;
	//else scaleTimer += 0.03f;
	//scale.x = scale.y = scale.z = sinf(scaleTimer);
	{
		//���[���h�s��v�Z
		XMMATRIX S = XMMatrixScaling(scale.x, scale.y, scale.z);
		XMMATRIX R = XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z);
		XMMATRIX T = XMMatrixTranslation(0, -5.0f, 0);
		XMFLOAT4X4 worldTramsform;
		XMStoreFloat4x4(&worldTramsform, S * R * T);
		
		//���f���A�j���[�V�����X�V����
		model->UpdateAnimation(elapsedTime);

		//���f���s��X�V
		model->UpdateTransform(worldTramsform);
	}

	timer += elapsedTime;
	if (timer <= 0.5f)isShow = true;
	else isShow = false;

	if (timer >= 1.0f) timer = 0.0f;
}

void SceneClear::Render()
{
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();
	Camera& camera = Camera::Instance();
	RenderContext rc;
	ShadowMap* shadowMap = Graphics::Instance().GetShadowMap();

	//�J�����X�V����
	rc.view = camera.GetView();
	rc.projection = camera.GetProjection();

	//�`��R���e�L�X�g�ݒ�
	rc.camera = &camera;
	rc.deviceContext = Graphics::Instance().GetDeviceContext();
	rc.renderState = Graphics::Instance().GetRenderState();
	RenderState* renderState = Graphics::Instance().GetRenderState();

	ID3D11SamplerState* samplers[] =
	{
		renderState->GetSamplerState(SamplerState::PointClamp)
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

	//3DModel
	{
		ModelShader* shader = Graphics::Instance().GetShader(ShaderId::Phong);
		shader->Begin(rc);
		//�V�F�[�_�[�Ƀ��f���`��
		shader->Draw(rc, model.get());
		shader->End(rc);
	}
	//2DSprite
	{
		float screenWidth = static_cast<float>(graphics.GetScreenWidth());
		float screenHeight = static_cast<float>(graphics.GetScreenHeight());
		float textureWidth = static_cast<float>(sprite[0]->GetTextureWidth());
		float textureHeight = static_cast<float>(sprite[0]->GetTextureHeight());

		FLOAT blendFactor[4] = { 1.0f,1.0f,1.0f,1.0f };
		UINT sampleMask = 0xFFFFFFFF;

		//titleSprite
		dc->OMSetBlendState(renderState->GetBlendState(BlendState::Transparency), blendFactor, sampleMask);
		sprite[0]->Render(dc, 0.0f, 0.0f, 0.0f, screenWidth, screenHeight, 0, 0, textureWidth, textureHeight, 0, { 1, 1, 1, 1 });
		if (isShow)
			sprite[1]->Render(dc, 0.0f, 0.0f, 0.0f, screenWidth, screenHeight, 0, 0, textureWidth, textureHeight, 0, { 1, 1, 1, 1 });
	}
}

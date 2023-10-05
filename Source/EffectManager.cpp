#include "Graphics/Graphics.h"
#include "EffectManager.h"

void EffectManager::Initialize()
{
	Graphics& graphics = Graphics::Instance();

	effekseerRenderer = EffekseerRendererDX11::Renderer::Create(
		graphics.GetDevice(), graphics.GetDeviceContext(), 2048);

	effekseerManager = Effekseer::Manager::Create(2048);

	//renderer�̊e��ݒ�
	effekseerManager->SetSpriteRenderer(effekseerRenderer->CreateSpriteRenderer());
	effekseerManager->SetRibbonRenderer(effekseerRenderer->CreateRibbonRenderer());
	effekseerManager->SetRingRenderer(effekseerRenderer->CreateRingRenderer());
	effekseerManager->SetTrackRenderer(effekseerRenderer->CreateTrackRenderer());
	effekseerManager->SetModelRenderer(effekseerRenderer->CreateModelRenderer());

	//Effekseer���ł̃��[�_�[�̐ݒ�
	effekseerManager->SetTextureLoader(effekseerRenderer->CreateTextureLoader());
	effekseerManager->SetModelLoader(effekseerRenderer->CreateModelLoader());
	effekseerManager->SetMaterialLoader(effekseerRenderer->CreateMaterialLoader());

	//����n���W�Ōv�Z����
	effekseerManager->SetCoordinateSystem(Effekseer::CoordinateSystem::LH);
}

void EffectManager::Finalize()
{
	//�X�}�[�g�|�C���^�ɂ���Ĕj�������̂ŉ������Ȃ�
}

void EffectManager::Update(float elapsedTime)
{
	effekseerManager->Update(elapsedTime * 60.0f);
}

void EffectManager::Render(const XMFLOAT4X4& view, const XMFLOAT4X4& projection)
{
	effekseerRenderer->SetCameraMatrix(*reinterpret_cast<const Effekseer::Matrix44*>(&view));
	effekseerRenderer->SetProjectionMatrix(*reinterpret_cast<const Effekseer::Matrix44*>(&projection));

	effekseerRenderer->BeginRendering();

	//�}�l�[�W���[�P�ʂŕ`�悷��̂ŕ`�揇�𐧌䂷��ꍇ�̓}�l�[�W���[�𕡐��쐬���A
	//Draw()�֐������s���鏇���Ő���ł�����
	effekseerManager->Draw();

	effekseerRenderer->EndRendering();
}

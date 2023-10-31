#include "Stage.h"
#include "Graphics/Graphics.h"

static Stage* instance = nullptr;
Stage& Stage::Instance() { return *instance; }

//�R���X�g���N�^
Stage::Stage()
{
	instance = this;

	ID3D11Device* device = Graphics::Instance().GetDevice();

	//�X�e�[�W���f���ǂݍ���
	//model = std::make_unique<Model>(device, "Data/Model/ExampleStage/ExampleStage.fbx");
	model = std::make_unique<Model>(device, "Data/Model/StageLowPoly/Environment.fbx",0.02f);
}

bool Stage::RayCast(const XMFLOAT3& start, const XMFLOAT3& end, HitResult& hit)
{
	return Collision::IntersectRayVsModel(start, end, model.get(), hit);
}

void Stage::Update(float elapsedTime)
{
	//���[���h�s��v�Z
	XMMATRIX S = XMMatrixScaling(scale.x, scale.y, scale.z);
	XMMATRIX R = XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z);
	XMMATRIX T = XMMatrixTranslation(position.x, position.y, position.z);
	XMFLOAT4X4 worldTramsform;
	XMStoreFloat4x4(&worldTramsform, S * R * T);

	//�g�����X�t�H�[���X�V
	model->UpdateTransform(worldTramsform);
}

void Stage::ShadowRender(const RenderContext& rc, ShadowMap* shadowMap)
{
	//�V���h�E�}�b�v�Ƀ��f���`��
	shadowMap->Draw(rc, model.get());
}
void Stage::Render(const RenderContext& rc, ModelShader* shader)
{
	for (const auto& mat : model->GetMaterials())
	{
		if (mat.name == "Water" && shader != Graphics::Instance().GetShader(ShaderId::WaterSurface)) continue;
		if (mat.name != "Water" && shader == Graphics::Instance().GetShader(ShaderId::WaterSurface)) continue;

		//�V�F�[�_�[�Ƀ��f���`��
		shader->Draw(rc, model.get());
	}
}

#pragma once

#include <memory>
#include "Shader/Shader.h"
#include "Graphics/Model.h"
#include "Collision.h"

class Stage
{
public:
	static Stage& Instance();

	Stage();
	~Stage() {}

	//���C�L���X�g
	bool RayCast(const XMFLOAT3& start, const XMFLOAT3& end, HitResult& hit);

	//�X�V
	void Update(float elapsedTime);
	//�`��
	void ShadowRender(const RenderContext& rc, ShadowMap* shadowMap);
	void WaterRender(const RenderContext& rc, ModelShader* shader);
	//�`��
	void TerrainRender(const RenderContext& rc, ModelShader* shader);

private:
	XMFLOAT3	position = { 0,0,0 };
	XMFLOAT3	angle = { 0,0,0 };
	XMFLOAT3	scale = { 1,1,1 };

	std::unique_ptr<Model> model;
};
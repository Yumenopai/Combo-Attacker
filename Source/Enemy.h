#pragma once

#include "Shader/Shader.h"
#include "Character.h"

class Enemy :public Character
{
public:
	Enemy(){}
	~Enemy() override{}

	//�X�V����									//�������z�֐�
	virtual void Update(float elapsedTime) = 0;

	//�`�揈��
	virtual void ShadowRender(const RenderContext& rc, ShadowMap* shadowMap) = 0;
	virtual void Render(const RenderContext& rc, ModelShader* shader) = 0;

	//�j��
	void Destroy();

	//�f�o�b�O�v���~�e�B�u�`��
	virtual void DrawDebugPrimitive();
};

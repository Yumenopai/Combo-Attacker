#pragma once

#include "Shader/Shader.h"
#include "Character.h"

class Enemy :public Character
{
public:
	Enemy(){}
	~Enemy() override{}

	//更新処理									//純粋仮想関数
	virtual void Update(float elapsedTime) = 0;

	//描画処理
	virtual void ShadowRender(const RenderContext& rc, ShadowMap* shadowMap) = 0;
	virtual void Render(const RenderContext& rc, ModelShader* shader) = 0;

	//破棄
	void Destroy();

	//デバッグプリミティブ描画
	virtual void DrawDebugPrimitive();
};

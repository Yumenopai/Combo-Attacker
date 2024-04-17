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



	//Setter
	void SetTargetPosition(DirectX::XMFLOAT3& pos) { targetPosition = pos; }

	//Getter
	virtual const float GetEffectOffset_Y() = 0;

protected:
	DirectX::XMFLOAT3 targetPosition = { 0,0,0 };

	//// パラメータ
	//Player* firstAttackPlayer = nullptr;
	//int AttackedCount[2] = { 0,0 };

};

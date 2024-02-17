#pragma once

#include "Graphics/Sprite.h"
#include "Scene.h"

class SceneClear : public Scene
{
public:
	SceneClear() {}
	~SceneClear() override {}

	void Initialize() override;
	void Finalize() override;
	void Update(float elapsedTime) override;
	void Render() override;

private:
	std::unique_ptr<Sprite> sprite[2];
	std::unique_ptr<Model> model;

	XMFLOAT3	angle = { -0.3f, 3.1f, 0 };
	XMFLOAT3	scale = { 1,1,1 };

	float timer = 0.0f;
	bool isShow = true;
	float scaleTimer = 90;
	bool isScaleDown = true;
};
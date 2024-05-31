#pragma once

#include "Graphics/Sprite.h"
#include "Scene.h"
#include "Stage.h"
#include "Light/Light.h"

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
	std::unique_ptr<Sprite> sprite;
	std::unique_ptr<Model> model;
	std::unique_ptr<Stage> stage;
	Light* mainDirectionalLight = nullptr;

	XMFLOAT3	angle = { -0.3f, 3.1f, 0 };
	XMFLOAT3	scale = { 0.6f,0.6f,0.6f };
	float waterTimer = 0.0f;

	float timer = 0.0f;
	bool isShow = true;
	float scaleTimer = 90;
	bool isScaleDown = true;
};
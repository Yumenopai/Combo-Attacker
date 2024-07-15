#pragma once
#include "Graphics/Sprite.h"
#include "Graphics/FontSprite.h"
#include "Scene.h"

#include <thread>

class SceneLoading : public Scene
{
public:
	SceneLoading(Scene* nextScene) : nextScene(nextScene) {}
	~SceneLoading() override {}

	void Initialize() override;
	void Finalize() override;
	void Update(float elapsedTime) override;
	void Render() override;

private:
	//loading thread
	static void LoadingThread(SceneLoading* scene);

private:
	std::unique_ptr<Sprite> sprite[2];
	std::unique_ptr<FontSprite> font;

	float angle = 0.0f;
	Scene* nextScene = nullptr;
	std::unique_ptr<std::thread> thread;
	float timer = 0.0f;;
};
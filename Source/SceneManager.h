#pragma once
#include "Scene.h"

class SceneManager
{
private:
	SceneManager() {}
	~SceneManager() {}

public:
	static SceneManager& Instance()
	{
		static SceneManager instance;
		return instance;
	}

	void Update(float elapsedTime);
	void Render();
	void Clear();
	void ChangeScene(Scene* scene);
	Scene* GetScene() { return currentScene; }

private:
	Scene* currentScene = nullptr;
	Scene* nextScene = nullptr;
};
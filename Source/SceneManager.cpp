#include "SceneManager.h"

void SceneManager::Update(float elapsedTime)
{
	if (nextScene != nullptr)
	{
		//oldDalete
		Clear();

		//newSetting
		currentScene = nextScene;
		nextScene = nullptr;

		//init
		if (!currentScene->IsReady())
			currentScene->Initialize();
	}

	if (currentScene != nullptr)
	{
		currentScene->Update(elapsedTime);
	}
}

void SceneManager::Render()
{
	if (currentScene != nullptr)
	{
		currentScene->Render();
	}

}

void SceneManager::Clear()
{
	if (currentScene != nullptr)
	{
		currentScene->Finalize();
		delete currentScene;
		currentScene = nullptr;
	}
}

void SceneManager::ChangeScene(Scene* scene)
{
	if (nextScene != nullptr)
	{
		delete nextScene;
		nextScene = nullptr;
	}

	//newSceneSetting
	nextScene = scene;
}
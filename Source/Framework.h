#pragma once

#include <windows.h>
#include "HighResolutionTimer.h"
#include "Scene.h"
#include "Input/input.h"

class Framework
{
public:
	Framework(HWND hWnd);
	~Framework();

private:
	void Update(float elapsedTime);
	void Render(float elapsedTime);

	void CalculateFrameStats();

public:
	int Run();
	LRESULT CALLBACK HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
	const HWND				hWnd;
	HighResolutionTimer		timer;
	std::unique_ptr<Scene>	scene;
	Input					input;

	bool isSlow = false;
	bool showDebug = false;
};


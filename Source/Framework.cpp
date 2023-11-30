#include <memory>
#include <sstream>
#include <imgui.h>

#include "Framework.h"
#include "Graphics/Graphics.h"
#include "Graphics/ImGuiRenderer.h"

#include "EffectManager.h"

#include "SceneManager.h"
#include "SceneTitle.h"
#include "SceneGame.h"

// ���������Ԋu�ݒ�
static const int syncInterval = 1;

// �R���X�g���N�^
Framework::Framework(HWND hWnd)
	: hWnd(hWnd)
	, input(hWnd)
{
	//�O���t�B�b�N������
	Graphics::Instance().Initialize(hWnd);

	//�G�t�F�N�g�}�l�[�W���[������
	EffectManager::Instance().Initialize();

	//IMGUI������
	ImGuiRenderer::Initialize(hWnd, Graphics::Instance().GetDevice(), Graphics::Instance().GetDeviceContext());

	//�V�[��������
	SceneManager::Instance().ChangeScene(new SceneGame(1));
	//SceneManager::Instance().ChangeScene(new ShadowTestScene);
}

// �f�X�g���N�^
Framework::~Framework()
{
	SceneManager::Instance().Clear();
	EffectManager::Instance().Finalize();

	//IMGUI�I����
	ImGuiRenderer::Finalize();
}

// �X�V����
void Framework::Update(float elapsedTime)
{
	// ���͍X�V����
	input.Update();

	if (isSlow) elapsedTime *= 0.25f;

	// �V�[���X�V����
	SceneManager::Instance().Update(elapsedTime);
}

// �`�揈��
void Framework::Render(float elapsedTime)
{
	//�ʃX���b�h���Ƀf�o�C�X�R���e�L�X�g���g���Ă����ꍇ��
	//�����A�N�Z�X���Ȃ��悤�ɔr�����䂷��
	std::lock_guard <std::mutex> lock(Graphics::Instance().GetMutex());


	ID3D11DeviceContext* dc = Graphics::Instance().GetDeviceContext();

	//IMGUI�t���[���J�n����
	ImGuiRenderer::NewFrame();

	//��ʃN���A
	Graphics::Instance().GetFrameBuffer(FrameBufferId::Display)->Clear(dc, 0, 0, 1, 1);

	//�����_�[�^�[�Q�b�g�ݒ�
	Graphics::Instance().GetFrameBuffer(FrameBufferId::Display)->SetRenderTarget(dc);

	// �V�[���`�揈��
	SceneManager::Instance().Render();

#if 0

	//IMGUI�f���E�B���h�E(�@�\�e�X�g�p)
	ImGui::ShowDemoWindow();

#endif
#ifdef _DEBUG 

	ImGui::Checkbox("slow", &isSlow);
	
	//IMGUI�`��
	ImGuiRenderer::Render(dc);

#endif
	//��ʕ\��
	Graphics::Instance().Present(syncInterval);
}

// �t���[�����[�g�v�Z
void Framework::CalculateFrameStats()
{
	// Code computes the average frames per second, and also the 
	// average time it takes to render one frame.  These stats 
	// are appended to the window caption bar.
	static int frames = 0;
	static float time_tlapsed = 0.0f;

	frames++;

	// Compute averages over one second period.
	if ((timer.TimeStamp() - time_tlapsed) >= 1.0f)
	{
		float fps = static_cast<float>(frames); // fps = frameCnt / 1
		float mspf = 1000.0f / fps;
		std::ostringstream outs;
		outs.precision(6);
		outs << "FPS : " << fps << " / " << "Frame Time : " << mspf << " (ms)";
		//outs << "Go! Onto Line Field";
		SetWindowTextA(hWnd, outs.str().c_str());

		// Reset for next average.
		frames = 0;
		time_tlapsed += 1.0f;
	}
}

// �A�v���P�[�V�������[�v
int Framework::Run()
{
	MSG msg = {};

	while (WM_QUIT != msg.message)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			timer.Tick();
			CalculateFrameStats();

			float elapsedTime = syncInterval == 0
				? timer.TimeInterval()
				: syncInterval / 60.0f
				;
			Update(elapsedTime);
			Render(elapsedTime);
		}
	}
	return static_cast<int>(msg.wParam);
}

// ���b�Z�[�W�n���h��
LRESULT CALLBACK Framework::HandleMessage(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGuiRenderer::HandleMessage(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc;
		hdc = BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		break;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_CREATE:
		break;
	case WM_KEYDOWN:
		if (wParam == VK_ESCAPE) PostMessage(hWnd, WM_CLOSE, 0, 0);
		break;
	case WM_ENTERSIZEMOVE:
		// WM_EXITSIZEMOVE is sent when the user grabs the resize bars.
		timer.Stop();
		break;
	case WM_EXITSIZEMOVE:
		// WM_EXITSIZEMOVE is sent when the user releases the resize bars.
		// Here we reset everything based on the new window dimensions.
		timer.Start();
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

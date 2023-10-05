#pragma once

#include <DirectXMath.h>
#include <Effekseer.h>
#include <EffekseerRendererDX11.h>
using namespace DirectX;

class EffectManager
{
private:
	EffectManager() {}
	~EffectManager() {}

public:
	//唯一のインスタンス取得
	static EffectManager& Instance()
	{
		static EffectManager instance;
		return instance;
	}

	void Initialize();
	void Finalize();
	void Update(float elapsedTime);
	void Render(const XMFLOAT4X4& view, const XMFLOAT4X4& projection);

	Effekseer::ManagerRef GetEffekseerManager() { return effekseerManager; }

private:
	Effekseer::ManagerRef			effekseerManager;
	EffekseerRenderer::RendererRef	effekseerRenderer;
};
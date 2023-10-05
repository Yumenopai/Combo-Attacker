#pragma once

#include <DirectXMath.h>
#include <Effekseer.h>
using namespace DirectX;

class Effect
{
public:
	Effect(const char* filename);
	~Effect() {};

	Effekseer::Handle Play(const XMFLOAT3& position, float scale = 1.0f);

	void Stop(Effekseer::Handle handle);

	void SetPosition(Effekseer::Handle handle, const XMFLOAT3& position);

	void SetScale(Effekseer::Handle handle, const XMFLOAT3& scale);

private:
	Effekseer::EffectRef effekseerEffect;
};
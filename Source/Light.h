#pragma once

#include <DirectXMath.h>
using namespace DirectX;

struct DirectionalLight
{
	XMFLOAT3 direction;
	XMFLOAT3 color;
};

class LightManager
{
public:
	//�f�B���N�V���i�����C�g�ݒ�
	void SetDirectionalLight(DirectionalLight& light) { directionalLight = light; }

	//�f�B���N�V���i�����C�g�擾
	const DirectionalLight& GetDirectionalLight() const { return directionalLight; }

private:
	DirectionalLight directionalLight;
};
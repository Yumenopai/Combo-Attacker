#pragma once

#include "Camera/Camera.h"
#include "RenderState.h"
#include "Shader/ShadowMap.h"

//UV�X�N���[�����
struct UVScrollData
{
	XMFLOAT2 uvScrollValue; // UV�X�N���[���l
};
//���s�������
struct DirectionalLightData
{
	XMFLOAT4 direction; // ����
	XMFLOAT4 color; // �F
};

//�_�������
struct PointLightData
{
	XMFLOAT4 position;	//���W
	XMFLOAT4 color;		//�F
	float	 range;		//�͈�
	XMFLOAT3 dummy;
};
// �_�����̍ő吔
static constexpr int PointLightMax = 8;

// �X�|�b�g���C�g���
struct SpotLightData
{
	XMFLOAT4 position; // ���W
	XMFLOAT4 direction; // ����
	XMFLOAT4 color; // �F
	float range; // �͈�
	float innerCorn; // �C���i�[�p�x�͈�
	float outerCorn; // �A�E�^�[�p�x�͈�
	float dummy;
};
// �X�|�b�g���C�g�̍ő吔
static constexpr int SpotLightMax = 8;


struct RenderContext
{
	ID3D11DeviceContext*	deviceContext;

	const RenderState*		renderState;
	const Camera*			camera;

	// ���C�g���
	XMFLOAT4 ambientLightColor;
	DirectionalLightData directionalLightData;
	PointLightData pointLightData[PointLightMax];	// �_�������
	SpotLightData spotLightData[SpotLightMax]; // �X�|�b�g���C�g���
	int pointLightCount = 0; // �_������
	int spotLightCount = 0; // �X�|�b�g���C�g��

	XMFLOAT4X4		view;
	XMFLOAT4X4		projection;
	XMFLOAT4		lightDirection;

	// �X�N���[���f�[�^
	UVScrollData uvScrollData;

	const ShadowMap*	shadowMap;
	XMFLOAT3			shadowColor;

	//water
	float timer = 0.0f;
	ID3D11ShaderResourceView* reflectShaderResourceView = nullptr; //���˃}�b�v
	XMFLOAT4X4 reflectViewProjection;
};

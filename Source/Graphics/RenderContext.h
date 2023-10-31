#pragma once

#include "Camera/Camera.h"
#include "RenderState.h"
#include "Light.h"
#include "Shader/ShadowMap.h"

//UVスクロール情報
struct UVScrollData
{
	XMFLOAT2 uvScrollValue; // UVスクロール値
};

struct RenderContext
{
	ID3D11DeviceContext*	deviceContext;

	const RenderState*		renderState;
	const Camera*			camera;
	const LightManager*		lightManager;

	XMFLOAT4X4		view;
	XMFLOAT4X4		projection;
	XMFLOAT4		lightDirection;

	// スクロールデータ
	UVScrollData uvScrollData;

	const ShadowMap*	shadowMap;
	XMFLOAT3			shadowColor;

	//water
	float timer = 0.0f;
	ID3D11ShaderResourceView* reflectShaderResourceView = nullptr; //反射マップ
	XMFLOAT4X4 reflectViewProjection;

};

#pragma once

#include "Camera/Camera.h"
#include "RenderState.h"
#include "Shader/ShadowMap.h"

//UVスクロール情報
struct UVScrollData
{
	XMFLOAT2 uvScrollValue; // UVスクロール値
};
//平行光源情報
struct DirectionalLightData
{
	XMFLOAT4 direction; // 向き
	XMFLOAT4 color; // 色
};

//点光源情報
struct PointLightData
{
	XMFLOAT4 position;	//座標
	XMFLOAT4 color;		//色
	float	 range;		//範囲
	XMFLOAT3 dummy;
};
// 点光源の最大数
static constexpr int PointLightMax = 8;

// スポットライト情報
struct SpotLightData
{
	XMFLOAT4 position; // 座標
	XMFLOAT4 direction; // 向き
	XMFLOAT4 color; // 色
	float range; // 範囲
	float innerCorn; // インナー角度範囲
	float outerCorn; // アウター角度範囲
	float dummy;
};
// スポットライトの最大数
static constexpr int SpotLightMax = 8;


struct RenderContext
{
	ID3D11DeviceContext*	deviceContext;

	const RenderState*		renderState;
	const Camera*			camera;

	// ライト情報
	XMFLOAT4 ambientLightColor;
	DirectionalLightData directionalLightData;
	PointLightData pointLightData[PointLightMax];	// 点光源情報
	SpotLightData spotLightData[SpotLightMax]; // スポットライト情報
	int pointLightCount = 0; // 点光源数
	int spotLightCount = 0; // スポットライト数

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

#pragma once
#include <d3d11.h>
#include <DirectXMath.h>

// int/置換
#define SC_INT static_cast<int>

/******************** TextOut ********************/
// 奥行/デフォルト
static const float TEXT_Depth_Default = 0.0f;
// 幅/デフォルト
static const float TEXT_SizeWidge_Default = 12.0f;
// 高さ/デフォルト
static const float TEXT_SizeHeight_Default = 16.0f;
// 切抜位置_X/デフォルト
static const float TEXT_CutPositionX_Default = 32.0f;
// 切抜位置_Y/デフォルト
static const float TEXT_CutPositionY_Default = 32.0f;
// 切抜サイズ_幅/デフォルト
static const float TEXT_CutWidge_Default = 16.0f;
// 切抜サイズ_高さ/デフォルト
static const float TEXT_CutHeight_Default = 16.0f;
// 角度/デフォルト
static const float TEXT_Angle_Default = 0.0f;

/******************** Sprite ********************/
// テクスチャなし
static const float Sprite_NoneTexture = 0.0f;
// 奥行/デフォルト
static const float Sprite_Position_Z_Default = 0.0f;
// 奥行/デフォルト
static const DirectX::XMFLOAT2 Sprite_CutPosition_Default = { 0.0f, 0.0f };
// 角度/デフォルト
static const float Sprite_Angle_Default = 0.0f;
// 色/デフォルト
static const DirectX::XMFLOAT4 Sprite_Color_Default = { 1.0f, 1.0f, 1.0f, 1.0f };

/******************** Gizmos ********************/
// デバッグ円柱の角度/デフォルト
static const DirectX::XMFLOAT3 GIZMOS_CylinderAngle_Default = { 0.0f, 0.0f, 0.0f };
// カラー/赤
static const DirectX::XMFLOAT4 GIZMOS_Color_Red = { 1.0f, 0.0f, 0.0f, 1.0f };
// カラー/緑
static const DirectX::XMFLOAT4 GIZMOS_Color_Green = { 0.0f, 1.0f, 0.0f, 1.0f };
// カラー/青
static const DirectX::XMFLOAT4 GIZMOS_Color_Blue = { 0.0f, 0.0f, 1.0f, 1.0f };

/******************** Player ********************/
static const int PLAYER_Count = 2;

/******************** SceneGame ********************/
// カメラY軸調整
static const float SCENEGAME_Camera_Y = 0.5f;
// シャドウマップ色
static const DirectX::XMFLOAT3 SCENEGAME_ShadowColor = { 0.5f, 0.5f, 0.5f };


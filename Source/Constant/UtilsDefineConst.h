#pragma once
#include <d3d11.h>
#include <DirectXMath.h>

// int/置換
#define SC_INT static_cast<int>

/******************** TextOut ********************/
// 奥行/デフォルト
static const float TEXT_depth_default = 0.0f;
// サイズ/デフォルト
static const DirectX::XMFLOAT2 TEXT_display_size_default = { 12.0f, 16.0f };
// 切抜位置/デフォルト
static const DirectX::XMFLOAT2 TEXT_cut_position_default = { 32.0f, 32.0f };
// 切抜サイズ/デフォルト
static const DirectX::XMFLOAT2 TEXT_cut_size_default = { 16.0f, 16.0f };
// 角度/デフォルト
static const float TEXT_angle_default = 0.0f;

/******************** Sprite ********************/
// 奥行/デフォルト
static const float SPRITE_position_default_z = 0.0f;
// 切抜位置/デフォルト
static const DirectX::XMFLOAT2 SPRITE_cut_position_default = { 0.0f, 0.0f };
// 角度/デフォルト
static const float SPRITE_angle_default = 0.0f;
// 色/デフォルト
static const DirectX::XMFLOAT4 SPRITE_color_default = { 1.0f, 1.0f, 1.0f, 1.0f };

/******************** Gizmos ********************/
// デバッグ円柱の角度/デフォルト
static const DirectX::XMFLOAT3 GIZMOS_cylinder_angle_default = { 0.0f, 0.0f, 0.0f };
// カラー/赤
static const DirectX::XMFLOAT4 GIZMOS_color_red = { 1.0f, 0.0f, 0.0f, 1.0f };
// カラー/緑
static const DirectX::XMFLOAT4 GIZMOS_color_green = { 0.0f, 1.0f, 0.0f, 1.0f };
// カラー/青
static const DirectX::XMFLOAT4 GIZMOS_color_blue = { 0.0f, 0.0f, 1.0f, 1.0f };

/******************** Player ********************/
// Player1P/シリアルナンバー
static const int PL1P_NO = 0;
// PlayerAI/シリアルナンバー
static const int PLAI_NO = 1;

/******************** SceneGame ********************/
// カメラY軸調整
static const float SCENEGAME_camera_y = 0.5f;
// シャドウマップ色
static const DirectX::XMFLOAT3 SCENEGAME_shadow_color = { 0.5f, 0.5f, 0.5f };


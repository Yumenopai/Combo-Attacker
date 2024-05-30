#pragma once
#include <DirectXMath.h>

/******************** Enemy ********************/
// スライムの数
static const int enemy_slime_count = 4;
// 甲羅の数
static const int enemy_turtleShell_count = 4;

// HPゲージ/表示距離
static const float enemy_hp_gauge_display_dist = 12.0f;
// HPゲージ/サイズ
static const DirectX::XMFLOAT2 enemy_hp_gauge_size = { 60.0f, 8.0f };
// HPゲージ/背景拡張サイズ_XY
static const float enemy_hp_gauge_frame_expansion = 4.0f;
// HPゲージ/ゲージ下地カラー
static const DirectX::XMFLOAT4 enemy_hp_gauge_frame_color = { 0.5f, 0.5f, 0.5f, 0.5f };
// HPゲージ/ゲージカラー_通常
static const DirectX::XMFLOAT4 enemy_hp_gauge_color_normal = { 1.0f, 0.0f, 0.0f, 1.0f };

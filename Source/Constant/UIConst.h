#pragma once
#include <DirectXMath.h>

/******************** Icon ********************/
// 武器アイコン/スプライトサイズ
static const DirectX::XMFLOAT2 WeaponIcon_sprite_size = { 300.0f, 300.f };
// 武器アイコン/サイズ
static const DirectX::XMFLOAT2 WeaponIcon_size = { 45.0f, 45.0f };
// 武器アイコン/位置_X
static const float WeaponIcon_position_x = 1013.0f;
// 武器アイコン/オフセット_Y
static const float WeaponIcon_offset_y = 2.0f;
// 武器アイコン/色半透明_a
static const float WeaponIcon_color_translucent_a = 0.3f;
// 武器アイコン/色減少_rb
static const float WeaponIcon_color_decrease_rb = 0.4f;

// 武器フレーム/位置_X
static const float WeaponFrame_position_x = 1000.0f;
// 武器フレーム/オフセット
static const DirectX::XMFLOAT2 WeaponFrame_offset = { 65.0f, -10.0f };
// 武器フレーム/サイズ
static const DirectX::XMFLOAT2 WeaponFrame_render_size = { 70.0f, 70.0f };

// ボタンフレーム/スプライトサイズ
static const DirectX::XMFLOAT2 ButtonFrame_sprite_size = { 300.0f, 300.f };
// ボタンフレーム/サイズ
static const DirectX::XMFLOAT2 ButtonFrame_size = { 80.0f, 80.0f };

// ボタンフレーム/Xボタン位置_X
static const float ButtonFrame_button_X_position_x = 1100.0f;
// ボタンフレーム/YAボタン位置_X
static const float ButtonFrame_button_YA_position_x = 1150.0f;
// ボタンフレーム/Bボタン位置_X
static const float ButtonFrame_button_B_position_x = 1200.0f;
// ボタンフレーム/Yボタン位置_Y
static const float ButtonFrame_button_Y_position_y = 250.0f;
// ボタンフレーム/XBボタン位置_Y
static const float ButtonFrame_button_XB_position_y = 300.0f;
// ボタンフレーム/Aボタン位置_Y
static const float ButtonFrame_button_A_position_y = 350.0f;

// ボタンアイコン/フレームからアイコンの位置オフセット
static const  DirectX::XMFLOAT2 ButtonIcon_frame_to_icon_offset = { 15.0f, 10.0f };

// ボタンアイコン/スプライトサイズ
static const DirectX::XMFLOAT2 ButtonIcon_sprite_size = { 300.0f, 300.f };
// ボタンアイコン/サイズ
static const DirectX::XMFLOAT2 ButtonIcon_render_size = { 50.0f,50.0f };

// ボタンアイコン/回復薬のカット位置倍率
static const int ButtonIcon_cut_rate_recover = 0;
// ボタンアイコン/ソードのカット位置倍率
static const int ButtonIcon_cut_rate_sword = 1;
// ボタンアイコン/ハンマーのカット位置倍率
static const int ButtonIcon_cut_rate_hammer = 2;
// ボタンアイコン/スピアーのカット位置倍率
static const int ButtonIcon_cut_rate_spear = 3;

// ボタンに表示する名前/スプライトサイズ
static const DirectX::XMFLOAT2 ButtonName_sprite_size = { 300.0f, 150.f };
// ボタンに表示する名前/サイズ
static const DirectX::XMFLOAT2 ButtonName_render_size = { 50.0f, 25.0f };

/******************** Mission ********************/
// ミッション/スプライトサイズ
static const DirectX::XMFLOAT2 Mission_sprite_size = { 900.0f, 600.f };
// ミッション/位置
static const DirectX::XMFLOAT3 Mission_position = { 1040.0f, 10.f, 0.0f };
// ミッション/サイズ
static const DirectX::XMFLOAT2 Mission_render_size = { 225.0f, 150.0f };

// ミッション/表示時間
static const int Mission_display_timer_max = 180;

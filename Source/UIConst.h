#pragma once
#include <DirectXMath.h>

/******************** Icon ********************/
// 武器アイコン/スプライトサイズ
static const DirectX::XMFLOAT2 WeaponIcon_SpriteSize = { 300.0f, 300.f };
// 武器アイコン/サイズ
static const DirectX::XMFLOAT2 WeaponIcon_Size = { 45.0f, 45.f };
// 武器アイコン/位置_X
static const float WeaponIcon_Position_X = 1013.0f;
// 武器アイコン/オフセット_Y
static const float WeaponIcon_Offset_Y = 2.0f;
// 武器アイコン/色半透明_a
static const float WeaponIcon_ColorTranslucent_a = 0.3f;
// 武器アイコン/色減少_rb
static const float WeaponIcon_ColorDecrease_rb = 0.4f;

// 武器フレーム/位置_X
static const float WeaponFrame_Position_X = 1000.0f;
// 武器フレーム/オフセット_X
static const float WeaponFrame_Offset_X = 65.0f;
// 武器フレーム/オフセット_Y
static const float WeaponFrame_Offset_Y = -10.0f;
// 武器フレーム/サイズ
static const DirectX::XMFLOAT2 WeaponFrame_Size = { 70.0f, 70.0f };

// ボタンフレーム/スプライトサイズ
static const DirectX::XMFLOAT2 ButtonFrame_SpriteSize = { 300.0f, 300.f };
// ボタンフレーム/サイズ
static const DirectX::XMFLOAT2 ButtonFrame_Size = { 80.0f, 80.0f };

// ボタンフレーム/Xボタン位置_X
static const float ButtonFrame_ButtonX_Position_X = 1100.0f;
// ボタンフレーム/YAボタン位置_X
static const float ButtonFrame_ButtonYA_Position_X = 1150.0f;
// ボタンフレーム/Bボタン位置_X
static const float ButtonFrame_ButtonB_Position_X = 1200.0f;
// ボタンフレーム/Yボタン位置_Y
static const float ButtonFrame_ButtonY_Position_Y = 250.0f;
// ボタンフレーム/XBボタン位置_Y
static const float ButtonFrame_ButtonXB_Position_Y = 300.0f;
// ボタンフレーム/Aボタン位置_Y
static const float ButtonFrame_ButtonA_Position_Y = 350.0f;

// ボタンアイコン/フレームからアイコンの位置オフセット_X
static const float ButtonIcon_FrameToIconOffset_X = 15.0f;
// ボタンアイコン/フレームからアイコンの位置オフセット_Y
static const float ButtonIcon_FrameToIconOffset_Y = 10.0f;

// ボタンアイコン/スプライトサイズ
static const DirectX::XMFLOAT2 ButtonIcon_SpriteSize = { 300.0f, 300.f };
// ボタンアイコン/サイズ
static const DirectX::XMFLOAT2 ButtonIcon_Size = { 50.0f,50.0f };

// ボタンアイコン/回復薬のカット位置倍率
static const int ButtonIcon_CutRate_Recover = 0;
// ボタンアイコン/ソードのカット位置倍率
static const int ButtonIcon_CutRate_Sword = 1;
// ボタンアイコン/スピアーのカット位置倍率
static const int ButtonIcon_CutRate_Spear = 3;
// ボタンアイコン/ハンマーのカット位置倍率
static const int ButtonIcon_CutRate_Hammer = 2;

// ボタンに表示する名前/スプライトサイズ
static const DirectX::XMFLOAT2 ButtonName_SpriteSize = { 300.0f, 150.f };
// ボタンに表示する名前/サイズ
static const DirectX::XMFLOAT2 ButtonName_Size = { 50.0f, 25.0f };

/******************** Mission ********************/
// ミッション/スプライトサイズ
static const DirectX::XMFLOAT2 Mission_SpriteSize = { 900.0f, 600.f };
// ミッション/位置
static const DirectX::XMFLOAT3 Mission_Position = { 1040.0f, 10.f, 0.0f };
// ミッション/サイズ
static const DirectX::XMFLOAT2 Mission_Size = { 225.0f, 150.0f };

// ミッション/表示時間
static const int Mission_DisplayTimerMax = 180;

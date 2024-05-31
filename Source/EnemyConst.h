#pragma once
#include <DirectXMath.h>
#include "AnimationTimeStruct.h"

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

// 攻撃ノード半径
static const float attack_node_radius = 0.2f;

// レベルを与える最低条件のダメージパーセンテージ
static const float add_level_min_damage_rate = 0.2f;
// 多くダメージを与えたプレイヤーへのボーナスレベルアップ量
static const int most_attack_bonus_level_up = 1;

/******************** EnemySlime ********************/
// モデルのファイル指定
static const char* slime_model_file = "Data/Model/RPG_Slime/SlimePBR.fbx";
// 目玉ノードの名前
static const char* slime_eye_ball_node_name = "EyeBall";

// モデル描画サイズ
static const float slime_render_size = 0.01f;
// 当たり判定/半径
static const float slime_radius = 0.5f;
// 当たり判定/高さ
static const float slime_height = 1.0f;
// 最大HP
static const int slime_max_health = 30;
// 与えるダメージ
static const int slime_attack_damage = 2;
// レベルアップ量
static const int slime_add_level_up = 1;

// 縄張り範囲
static const float	slime_territory_range = 10.0f;
// 移動スピード
static const float	slime_move_speed = 1.5f;
// 回転スピード
static const float	slime_turn_speed = DirectX::XMConvertToRadians(360);
// 探知範囲
static const float	slime_search_range = 5.0f;
// 攻撃範囲
static const float	slime_attack_range = 3.0f;

// エフェクトY軸オフセット
static const float slime_effect_offset_y = 0.8f;
// 攻撃時間
static const AnimationTime slime_attack_time = { 0.1f,0.35f };
// 攻撃時の吹っ飛ばし力
static const float slime_impulse_power_rate = 3.0f;
// 攻撃時の吹っ飛ばし力
static const float slime_impulse_power_y = 4.0f;


/******************** EnemyTurtleShell ********************/
// モデルのファイル指定
static const char* turtle_model_file = "Data/Model/RPG_TurtleShell/TurtleShellPBR.fbx";
// 目玉ノードの名前
static const char* turtle_eye_ball_node_name = "Eyeball";

// モデル描画サイズ
static const float turtle_render_size = 0.01f;
// 当たり判定/半径
static const float turtle_radius = 0.5f;
// 当たり判定/高さ
static const float turtle_height = 1.0f;
// 最大HP
static const int turtle_max_health = 40;
// 与えるダメージ
static const int turtle_attack_damage = 1;
// レベルアップ量
static const int turtle_add_level_up = 2;

// 死亡時エフェクトサイズ
static const float turtle_dead_effect_size = 0.6f;

/******************** EnemyDragon ********************/
// モデルのファイル指定
static const char* dragon_model_file = "Data/Model/Enemy/Blue.fbx";
// 頭ノードの名前
static const char* dragon_head_node_name = "Jaw3";
// 爪ノードの名前
static const char* dragon_clawL_node_name = "WingClaw2_L";
// 爪ノードの名前
static const char* dragon_clawR_node_name = "WingClaw2_L_1";

// モデル描画サイズ
static const float dragon_render_size = 0.008f;
// 当たり判定/半径
static const float dragon_radius = 1.8f;
// 当たり判定/高さ
static const float dragon_height = 3.6f;
// 最大HP
static const int dragon_max_health = 150;
// 与えるダメージ
static const int dragon_attack_damage = 2;
// レベルアップ量
static const int dragon_add_level_up = 3;

// 縄張り範囲
static const float	dragon_territory_range = 10.0f;
// 移動スピード
static const float	dragon_move_speed = 3.0f;
// 回転スピード
static const float	dragon_turn_speed = DirectX::XMConvertToRadians(360);
// 探知範囲
static const float	dragon_search_range = 10.0f;
// 攻撃範囲
static const float	dragon_attack_range = 5.0f;

// エフェクトY軸オフセット
static const float dragon_effect_offset_y = 0.9f;
// 攻撃時間
static const AnimationTime dragon_attack_head_time = { 0.35f,0.6f };
// 攻撃時間
static const AnimationTime dragon_attack_claw_time = { 0.4f,1.05f };
// 攻撃時の吹っ飛ばし力
static const float dragon_impulse_power_rate = 10.0f;
// 攻撃時の吹っ飛ばし力
static const float dragon_impulse_power_y = 5.0f;

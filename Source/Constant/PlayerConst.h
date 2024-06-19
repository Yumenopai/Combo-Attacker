#pragma once
#include <string>
#include <DirectXMath.h>
#include "AnimationTimeStruct.h"

// Player::AttackType/置換
#define SC_AT static_cast<Player::AttackType>
// Player::PlayerMessage/置換
#define SC_PM static_cast<Player::PlayerMessage>

/******************** Player ********************/
// 初期レベル
static const int initial_level = 1;
// 攻撃中の敵ナンバー/誰も攻撃していない
static const int none_attacking = -1;
// プレイヤーと敵の判定距離/発見
static const float judge_dist_find = 6.5f;
// プレイヤーと敵の判定距離/攻撃
static const float judge_dist_attack = 2.5f;

// モデルサイズ
static const float model_size = 0.02f;
// 最大HP
static const int max_health = 100;
// 移動速度
static const float move_speed = 8.0f;

// 一回目のジャンプスピード
static const float first_jump_speed = 150.0f;
// 二回目のジャンプスピード
static const float second_jump_speed = 15.0f;
// 最大のジャンプスピード
static const float max_jump_speed = 17.5f;

// スピアージャンプ攻撃の降下増加率
static const float spear_jamp_attack_velocity_rate = 0.25f;

// 回復可能距離
static const float recover_dist = 3.0f;
// HP回復量
static const int recover_add_health = 40;

// プレイヤーから敵への攻撃無敵時間
static const float invincible_time = 0.0f;
// プレイヤーレベルによる攻撃増加量
static const float damage_increase = 0.15f;
// プレイヤー攻撃の吹っ飛ばす回設定
static const int impulse_attack_count = 4;
// プレイヤーから敵の吹っ飛ばすパワー
static const float impulse_power = 13.0f;
// プレイヤーから敵の吹っ飛ばすパワー調整_Y
static const float impulse_power_adjust_rate_y = 0.8f;

// ハンマー1/攻撃当たり判定時間
static const float attack_time_hammer1_start = 0.4f;
// ハンマー1/次の攻撃技を出せる時間
static const AnimationTime next_attack_time_hammer1to2 = { 0.5f, 0.8f };
// ハンマー2/攻撃当たり判定時間
static const float attack_time_hammer2_end = 0.7f;
// ハンマーJUMP/攻撃中の移動量
static const int attack_hammerJ_velocity = 800;
// スピアー1/次の攻撃技を出せる時間
static const AnimationTime next_attack_time_spear1to2 = { 0.2f, 0.7f };
// スピアー1/慣性前進時間
static const float add_velocity_time_spear1_end = 0.25f;
// スピアー1/攻撃中の移動量
static const int attack_spear1_add_velocity = 40;
// スピアー2/慣性前進時間
static const AnimationTime add_velocity_time_spear2 = { 0.30f, 0.45f };
// スピアー2/攻撃当たり判定時間
static const float attack_time_spear2_start = 0.30f;
// スピアー2/次の攻撃技を出せる時間
static const AnimationTime next_attack_time_spear2to3 = { 0.37f, 0.6f };
// スピアー2/攻撃中の移動量
static const int attack_spear2_add_velocity = 60;
// スピアー3/攻撃当たり判定時間
static const float attack_time_spear3_start = 0.30f;
// スピアー3/慣性前進時間
static const float add_velocity_time_spear3_end = 0.43f;
// スピアー3/攻撃中の移動量
static const int attack_spear3_add_velocity = 39;
// スピアーJUMP/慣性前進時間
static const AnimationTime add_velocity_time_spearJ = { 0.15f, 0.5f };
// スピアーJUMP/攻撃中の移動量
static const int attack_spearJ_velocity = 800;
// ソード1/慣性前進時間
static const float add_velocity_time_sword1_end = 0.2f;
// ソード1/攻撃中の移動量
static const int attack_sword1_add_velocity = 43;
// ソード1/次の攻撃技を出せる時間
static const AnimationTime next_attack_time_sword1to2 = { 0.3f, 0.7f };
// ソード2/慣性前進時間
static const float add_velocity_time_sword2_end = 0.25f;
// ソード2/攻撃中の移動量
static const int attack_sword2_add_velocity = 45;
// ソード2/次の攻撃技を出せる時間
static const AnimationTime next_attack_time_sword2to3 = { 0.35f, 0.6f };
// ソード3/攻撃当たり判定時間
static const AnimationTime attack_time_sword3 = { 0.25f, 0.6f };
// ソード3/慣性前進時間
static const AnimationTime add_velocity_time_sword3 = { 0.25f, 0.5f };
// ソード3/攻撃中の移動量
static const int attack_sword3_add_velocity = 48;
// ソード3/Y軸跳ね時間
static const float add_velocity_y_time_sword3_end = 0.4f;
// ソード3/Y軸跳ね量
static const float attack_sword3_add_velocity_y = 120.0f;
// ソードJUMP/攻撃中の移動量
static const int attack_swordJ_velocity = 800;
// ソードJUMP/攻撃当たり判定時間
static const float attack_time_swordJump_end = 0.63f;

// 連続攻撃フィニッシュカウント
static const int attack_count_finish = 4;
// 攻撃エフェクトサイズ
static const float attack_effect_size = 0.4f;

// プレイヤーの名前表示位置オフセット
static const DirectX::XMFLOAT2 name_offset = { -60.0f,5.0f };

// メッセージ/スプライトサイズ
static const DirectX::XMFLOAT2 message_sprite_size = { 600.0f, 100.f }; // 6:1
// メッセージ/表示位置オフセット
static const DirectX::XMFLOAT2 message_offset = { -90.0f, -5.0f };
// メッセージ/サイズ
static const DirectX::XMFLOAT2 message_size = { 180.0f, 30.0f }; // 6:1
//メッセージ/タイマー増加量
static const float message_timer_increase = 0.2f;
//メッセージ/タイマー初期値
static const float message_timer_initial = 0.0f;
//メッセージ/タイマー最大値
static const float message_timer_max = 10.0f;

// HPゲージ/サイズ
static const DirectX::XMFLOAT2 hp_gauge_size = { 700.0f, 15.0f };
// HPゲージ/ゲージカラー_通常
static const DirectX::XMFLOAT4 hp_gauge_color_normal = { 0.2f, 0.8f, 0.2f, 1.0f };
// HPゲージ/ゲージカラー_危険
static const DirectX::XMFLOAT4 hp_gauge_color_wornimg = { 0.8f, 0.2f, 0.2f, 1.0f };

// HPゲージ/背景拡張サイズ_XY
static const float hp_gauge_frame_expansion = 6.0f;
// HPゲージ/ゲージ下地カラー
static const DirectX::XMFLOAT4 hp_gauge_frame_color = { 0.3f, 0.3f, 0.3f, 0.8f };

// HPゲージ/名前表示位置
static const DirectX::XMFLOAT2 hp_gauge_name_offset = { 190.0f, 3.0f };

// HPゲージ/Lv表示位置_X
static const float lv_display_position_x = 740.0f;
// HPゲージ/HP表示位置_X
static const float hp_display_position_x = 850.0f;
// HPゲージ/HPゲージ下テキストオフセット_Y
static const float display_under_text_offset_y = 25.0f;

// HPゲージ/HP表示桁数
static const int hp_display_digit = 3;

/******************** EachPlayer ********************/
// 名前
static const std::string p1_name = "PLAYER";
static const std::string ai_Name = " BUDDY";
// 現在のプレイヤー登録数
static const int player_count = 2;
// シリアルナンバー
static const int p1_serial_number = 0;
static const int ai_serial_number = 1;
// 名前色
static const DirectX::XMFLOAT4 p1_name_color = { 0.1f, 0.65f, 0.9f, 1.0f };
static const DirectX::XMFLOAT4 ai_name_color = { 1.0f, 1.0f, 1.0f, 1.0f };
// 初期位置
static const DirectX::XMFLOAT3 p1_initial_position = { 28.0f, 5.0f, 19.0f };
static const DirectX::XMFLOAT3 ai_initial_position = { 25.0f, 5.0f, 14.0f };
// 初期角度
static const DirectX::XMFLOAT3 initial_angle = { 0.0f, 0.0f, 0.0f };
// 回転速度
static const float p1_turn_speed = DirectX::XMConvertToRadians(1200);
static const float ai_turn_speed = DirectX::XMConvertToRadians(360);
// HPゲージ/位置_Y
static const float p1_hp_gauge_position_y = 565.0f;
static const float ai_hp_gauge_position_y = 630.0f;

// 逃げるようになる最低全ダメージ量
static const int ai_ran_away_min_damage = 50;
// 逃げるダメージ割合
static const int ai_ran_away_damage_rate = 80;
// 残り僅かと判定する相手のダメージ割合
static const int ai_enemy_few_remain_damage_rate = 15;
// とどめを相手に譲る最低ダメージ割合
static const int ai_concede_finish_min_damage_rate = 20;
// 攻撃時の味方プレイヤーのとどめ攻撃を待つタイマー
static const int ai_wait_timer_max = 120;

// 味方についていく判定距離
static const float ai_player_follow_dist = 2.5f;
// 1Pから逃げる判定距離
static const float ai_ran_away_from_p1_dist = 3.5f;
// 敵から逃げる判定距離
static const float ai_ran_away_from_enemy_dist = 4.0f;
// 敵へ向かう判定距離
static const float ai_go_toward_enemy_dist = 10.0f;

/** PlayerAI_Message **/
// 位置
static const DirectX::XMFLOAT2 message_frame_position = { 10.0f, 560.0f };
// サイズ
static const DirectX::XMFLOAT2 message_frame_size = { 300.0f, 76.0f };
// 描画サイズ
static const DirectX::XMFLOAT2 message_frame_render_size = { 150.0f, 38.0f };



//TODO:武器クラス作成の際に移動
/******************** Weapon ********************/
// 武器の初期位置
static const DirectX::XMFLOAT3 WEAPON_initial_position = { 0.0f, 0.0f, 0.0f };
// 武器の根本のオフセット
static const DirectX::XMVECTOR WEAPON_initial_root_offset = { 0, 0, 0 };
// 武器の先のオフセット/ハンマー
static const DirectX::XMVECTOR WEAPON_hammer_initial_tip_offset = { 0, 35, 0 };
// 武器の先のオフセット/スピアー
static const DirectX::XMVECTOR WEAPON_spear_initial_tip_offset = { 0, 90, 0 };
// 武器の先のオフセット/ソード
static const DirectX::XMVECTOR WEAPON_sword_initial_tip_offset = { 0, 30, 0 };
// 武器の当たり判定半径/ハンマー
static const float WEAPON_hammer_radius = 0.5f;
// 武器の当たり判定半径/スピアー
static const float WEAPON_spear_radius = 0.4f;
// 武器の当たり判定半径/ソード
static const float WEAPON_sword_radius = 0.6f;
// 武器のダメージ量/ハンマー
static const int WEAPON_hammer_damage = 2;
// 武器のダメージ量/スピアー
static const int WEAPON_spear_damage = 1;
// 武器のダメージ量/ソード
static const int WEAPON_sword_damage = 1;

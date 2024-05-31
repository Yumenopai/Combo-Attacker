#pragma once
#include <DirectXMath.h>
#include "AnimationTimeStruct.h"

/******************** Enemy ********************/
// �X���C���̐�
static const int enemy_slime_count = 4;
// �b���̐�
static const int enemy_turtleShell_count = 4;

// HP�Q�[�W/�\������
static const float enemy_hp_gauge_display_dist = 12.0f;
// HP�Q�[�W/�T�C�Y
static const DirectX::XMFLOAT2 enemy_hp_gauge_size = { 60.0f, 8.0f };
// HP�Q�[�W/�w�i�g���T�C�Y_XY
static const float enemy_hp_gauge_frame_expansion = 4.0f;
// HP�Q�[�W/�Q�[�W���n�J���[
static const DirectX::XMFLOAT4 enemy_hp_gauge_frame_color = { 0.5f, 0.5f, 0.5f, 0.5f };
// HP�Q�[�W/�Q�[�W�J���[_�ʏ�
static const DirectX::XMFLOAT4 enemy_hp_gauge_color_normal = { 1.0f, 0.0f, 0.0f, 1.0f };

// �U���m�[�h���a
static const float attack_node_radius = 0.2f;

// ���x����^����Œ�����̃_���[�W�p�[�Z���e�[�W
static const float add_level_min_damage_rate = 0.2f;
// �����_���[�W��^�����v���C���[�ւ̃{�[�i�X���x���A�b�v��
static const int most_attack_bonus_level_up = 1;

/******************** EnemySlime ********************/
// ���f���̃t�@�C���w��
static const char* slime_model_file = "Data/Model/RPG_Slime/SlimePBR.fbx";
// �ڋʃm�[�h�̖��O
static const char* slime_eye_ball_node_name = "EyeBall";

// ���f���`��T�C�Y
static const float slime_render_size = 0.01f;
// �����蔻��/���a
static const float slime_radius = 0.5f;
// �����蔻��/����
static const float slime_height = 1.0f;
// �ő�HP
static const int slime_max_health = 30;
// �^����_���[�W
static const int slime_attack_damage = 2;
// ���x���A�b�v��
static const int slime_add_level_up = 1;

// �꒣��͈�
static const float	slime_territory_range = 10.0f;
// �ړ��X�s�[�h
static const float	slime_move_speed = 1.5f;
// ��]�X�s�[�h
static const float	slime_turn_speed = DirectX::XMConvertToRadians(360);
// �T�m�͈�
static const float	slime_search_range = 5.0f;
// �U���͈�
static const float	slime_attack_range = 3.0f;

// �G�t�F�N�gY���I�t�Z�b�g
static const float slime_effect_offset_y = 0.8f;
// �U������
static const AnimationTime slime_attack_time = { 0.1f,0.35f };
// �U�����̐�����΂���
static const float slime_impulse_power_rate = 3.0f;
// �U�����̐�����΂���
static const float slime_impulse_power_y = 4.0f;


/******************** EnemyTurtleShell ********************/
// ���f���̃t�@�C���w��
static const char* turtle_model_file = "Data/Model/RPG_TurtleShell/TurtleShellPBR.fbx";
// �ڋʃm�[�h�̖��O
static const char* turtle_eye_ball_node_name = "Eyeball";

// ���f���`��T�C�Y
static const float turtle_render_size = 0.01f;
// �����蔻��/���a
static const float turtle_radius = 0.5f;
// �����蔻��/����
static const float turtle_height = 1.0f;
// �ő�HP
static const int turtle_max_health = 40;
// �^����_���[�W
static const int turtle_attack_damage = 1;
// ���x���A�b�v��
static const int turtle_add_level_up = 2;

// ���S���G�t�F�N�g�T�C�Y
static const float turtle_dead_effect_size = 0.6f;

/******************** EnemyDragon ********************/
// ���f���̃t�@�C���w��
static const char* dragon_model_file = "Data/Model/Enemy/Blue.fbx";
// ���m�[�h�̖��O
static const char* dragon_head_node_name = "Jaw3";
// �܃m�[�h�̖��O
static const char* dragon_clawL_node_name = "WingClaw2_L";
// �܃m�[�h�̖��O
static const char* dragon_clawR_node_name = "WingClaw2_L_1";

// ���f���`��T�C�Y
static const float dragon_render_size = 0.008f;
// �����蔻��/���a
static const float dragon_radius = 1.8f;
// �����蔻��/����
static const float dragon_height = 3.6f;
// �ő�HP
static const int dragon_max_health = 150;
// �^����_���[�W
static const int dragon_attack_damage = 2;
// ���x���A�b�v��
static const int dragon_add_level_up = 3;

// �꒣��͈�
static const float	dragon_territory_range = 10.0f;
// �ړ��X�s�[�h
static const float	dragon_move_speed = 3.0f;
// ��]�X�s�[�h
static const float	dragon_turn_speed = DirectX::XMConvertToRadians(360);
// �T�m�͈�
static const float	dragon_search_range = 10.0f;
// �U���͈�
static const float	dragon_attack_range = 5.0f;

// �G�t�F�N�gY���I�t�Z�b�g
static const float dragon_effect_offset_y = 0.9f;
// �U������
static const AnimationTime dragon_attack_head_time = { 0.35f,0.6f };
// �U������
static const AnimationTime dragon_attack_claw_time = { 0.4f,1.05f };
// �U�����̐�����΂���
static const float dragon_impulse_power_rate = 10.0f;
// �U�����̐�����΂���
static const float dragon_impulse_power_y = 5.0f;

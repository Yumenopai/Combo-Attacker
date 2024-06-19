#pragma once
#include <string>
#include <DirectXMath.h>
#include "AnimationTimeStruct.h"

// Player::AttackType/�u��
#define SC_AT static_cast<Player::AttackType>
// Player::PlayerMessage/�u��
#define SC_PM static_cast<Player::PlayerMessage>

/******************** Player ********************/
// �������x��
static const int initial_level = 1;
// �U�����̓G�i���o�[/�N���U�����Ă��Ȃ�
static const int none_attacking = -1;
// �v���C���[�ƓG�̔��苗��/����
static const float judge_dist_find = 6.5f;
// �v���C���[�ƓG�̔��苗��/�U��
static const float judge_dist_attack = 2.5f;

// ���f���T�C�Y
static const float model_size = 0.02f;
// �ő�HP
static const int max_health = 100;
// �ړ����x
static const float move_speed = 8.0f;

// ���ڂ̃W�����v�X�s�[�h
static const float first_jump_speed = 150.0f;
// ���ڂ̃W�����v�X�s�[�h
static const float second_jump_speed = 15.0f;
// �ő�̃W�����v�X�s�[�h
static const float max_jump_speed = 17.5f;

// �X�s�A�[�W�����v�U���̍~��������
static const float spear_jamp_attack_velocity_rate = 0.25f;

// �񕜉\����
static const float recover_dist = 3.0f;
// HP�񕜗�
static const int recover_add_health = 40;

// �v���C���[����G�ւ̍U�����G����
static const float invincible_time = 0.0f;
// �v���C���[���x���ɂ��U��������
static const float damage_increase = 0.15f;
// �v���C���[�U���̐�����΂���ݒ�
static const int impulse_attack_count = 4;
// �v���C���[����G�̐�����΂��p���[
static const float impulse_power = 13.0f;
// �v���C���[����G�̐�����΂��p���[����_Y
static const float impulse_power_adjust_rate_y = 0.8f;

// �n���}�[1/�U�������蔻�莞��
static const float attack_time_hammer1_start = 0.4f;
// �n���}�[1/���̍U���Z���o���鎞��
static const AnimationTime next_attack_time_hammer1to2 = { 0.5f, 0.8f };
// �n���}�[2/�U�������蔻�莞��
static const float attack_time_hammer2_end = 0.7f;
// �n���}�[JUMP/�U�����̈ړ���
static const int attack_hammerJ_velocity = 800;
// �X�s�A�[1/���̍U���Z���o���鎞��
static const AnimationTime next_attack_time_spear1to2 = { 0.2f, 0.7f };
// �X�s�A�[1/�����O�i����
static const float add_velocity_time_spear1_end = 0.25f;
// �X�s�A�[1/�U�����̈ړ���
static const int attack_spear1_add_velocity = 40;
// �X�s�A�[2/�����O�i����
static const AnimationTime add_velocity_time_spear2 = { 0.30f, 0.45f };
// �X�s�A�[2/�U�������蔻�莞��
static const float attack_time_spear2_start = 0.30f;
// �X�s�A�[2/���̍U���Z���o���鎞��
static const AnimationTime next_attack_time_spear2to3 = { 0.37f, 0.6f };
// �X�s�A�[2/�U�����̈ړ���
static const int attack_spear2_add_velocity = 60;
// �X�s�A�[3/�U�������蔻�莞��
static const float attack_time_spear3_start = 0.30f;
// �X�s�A�[3/�����O�i����
static const float add_velocity_time_spear3_end = 0.43f;
// �X�s�A�[3/�U�����̈ړ���
static const int attack_spear3_add_velocity = 39;
// �X�s�A�[JUMP/�����O�i����
static const AnimationTime add_velocity_time_spearJ = { 0.15f, 0.5f };
// �X�s�A�[JUMP/�U�����̈ړ���
static const int attack_spearJ_velocity = 800;
// �\�[�h1/�����O�i����
static const float add_velocity_time_sword1_end = 0.2f;
// �\�[�h1/�U�����̈ړ���
static const int attack_sword1_add_velocity = 43;
// �\�[�h1/���̍U���Z���o���鎞��
static const AnimationTime next_attack_time_sword1to2 = { 0.3f, 0.7f };
// �\�[�h2/�����O�i����
static const float add_velocity_time_sword2_end = 0.25f;
// �\�[�h2/�U�����̈ړ���
static const int attack_sword2_add_velocity = 45;
// �\�[�h2/���̍U���Z���o���鎞��
static const AnimationTime next_attack_time_sword2to3 = { 0.35f, 0.6f };
// �\�[�h3/�U�������蔻�莞��
static const AnimationTime attack_time_sword3 = { 0.25f, 0.6f };
// �\�[�h3/�����O�i����
static const AnimationTime add_velocity_time_sword3 = { 0.25f, 0.5f };
// �\�[�h3/�U�����̈ړ���
static const int attack_sword3_add_velocity = 48;
// �\�[�h3/Y�����ˎ���
static const float add_velocity_y_time_sword3_end = 0.4f;
// �\�[�h3/Y�����˗�
static const float attack_sword3_add_velocity_y = 120.0f;
// �\�[�hJUMP/�U�����̈ړ���
static const int attack_swordJ_velocity = 800;
// �\�[�hJUMP/�U�������蔻�莞��
static const float attack_time_swordJump_end = 0.63f;

// �A���U���t�B�j�b�V���J�E���g
static const int attack_count_finish = 4;
// �U���G�t�F�N�g�T�C�Y
static const float attack_effect_size = 0.4f;

// �v���C���[�̖��O�\���ʒu�I�t�Z�b�g
static const DirectX::XMFLOAT2 name_offset = { -60.0f,5.0f };

// ���b�Z�[�W/�X�v���C�g�T�C�Y
static const DirectX::XMFLOAT2 message_sprite_size = { 600.0f, 100.f }; // 6:1
// ���b�Z�[�W/�\���ʒu�I�t�Z�b�g
static const DirectX::XMFLOAT2 message_offset = { -90.0f, -5.0f };
// ���b�Z�[�W/�T�C�Y
static const DirectX::XMFLOAT2 message_size = { 180.0f, 30.0f }; // 6:1
//���b�Z�[�W/�^�C�}�[������
static const float message_timer_increase = 0.2f;
//���b�Z�[�W/�^�C�}�[�����l
static const float message_timer_initial = 0.0f;
//���b�Z�[�W/�^�C�}�[�ő�l
static const float message_timer_max = 10.0f;

// HP�Q�[�W/�T�C�Y
static const DirectX::XMFLOAT2 hp_gauge_size = { 700.0f, 15.0f };
// HP�Q�[�W/�Q�[�W�J���[_�ʏ�
static const DirectX::XMFLOAT4 hp_gauge_color_normal = { 0.2f, 0.8f, 0.2f, 1.0f };
// HP�Q�[�W/�Q�[�W�J���[_�댯
static const DirectX::XMFLOAT4 hp_gauge_color_wornimg = { 0.8f, 0.2f, 0.2f, 1.0f };

// HP�Q�[�W/�w�i�g���T�C�Y_XY
static const float hp_gauge_frame_expansion = 6.0f;
// HP�Q�[�W/�Q�[�W���n�J���[
static const DirectX::XMFLOAT4 hp_gauge_frame_color = { 0.3f, 0.3f, 0.3f, 0.8f };

// HP�Q�[�W/���O�\���ʒu
static const DirectX::XMFLOAT2 hp_gauge_name_offset = { 190.0f, 3.0f };

// HP�Q�[�W/Lv�\���ʒu_X
static const float lv_display_position_x = 740.0f;
// HP�Q�[�W/HP�\���ʒu_X
static const float hp_display_position_x = 850.0f;
// HP�Q�[�W/HP�Q�[�W���e�L�X�g�I�t�Z�b�g_Y
static const float display_under_text_offset_y = 25.0f;

// HP�Q�[�W/HP�\������
static const int hp_display_digit = 3;

/******************** EachPlayer ********************/
// ���O
static const std::string p1_name = "PLAYER";
static const std::string ai_Name = " BUDDY";
// ���݂̃v���C���[�o�^��
static const int player_count = 2;
// �V���A���i���o�[
static const int p1_serial_number = 0;
static const int ai_serial_number = 1;
// ���O�F
static const DirectX::XMFLOAT4 p1_name_color = { 0.1f, 0.65f, 0.9f, 1.0f };
static const DirectX::XMFLOAT4 ai_name_color = { 1.0f, 1.0f, 1.0f, 1.0f };
// �����ʒu
static const DirectX::XMFLOAT3 p1_initial_position = { 28.0f, 5.0f, 19.0f };
static const DirectX::XMFLOAT3 ai_initial_position = { 25.0f, 5.0f, 14.0f };
// �����p�x
static const DirectX::XMFLOAT3 initial_angle = { 0.0f, 0.0f, 0.0f };
// ��]���x
static const float p1_turn_speed = DirectX::XMConvertToRadians(1200);
static const float ai_turn_speed = DirectX::XMConvertToRadians(360);
// HP�Q�[�W/�ʒu_Y
static const float p1_hp_gauge_position_y = 565.0f;
static const float ai_hp_gauge_position_y = 630.0f;

// ������悤�ɂȂ�Œ�S�_���[�W��
static const int ai_ran_away_min_damage = 50;
// ������_���[�W����
static const int ai_ran_away_damage_rate = 80;
// �c��͂��Ɣ��肷�鑊��̃_���[�W����
static const int ai_enemy_few_remain_damage_rate = 15;
// �Ƃǂ߂𑊎�ɏ���Œ�_���[�W����
static const int ai_concede_finish_min_damage_rate = 20;
// �U�����̖����v���C���[�̂ƂǂߍU����҂^�C�}�[
static const int ai_wait_timer_max = 120;

// �����ɂ��Ă������苗��
static const float ai_player_follow_dist = 2.5f;
// 1P���瓦���锻�苗��
static const float ai_ran_away_from_p1_dist = 3.5f;
// �G���瓦���锻�苗��
static const float ai_ran_away_from_enemy_dist = 4.0f;
// �G�֌��������苗��
static const float ai_go_toward_enemy_dist = 10.0f;

/** PlayerAI_Message **/
// �ʒu
static const DirectX::XMFLOAT2 message_frame_position = { 10.0f, 560.0f };
// �T�C�Y
static const DirectX::XMFLOAT2 message_frame_size = { 300.0f, 76.0f };
// �`��T�C�Y
static const DirectX::XMFLOAT2 message_frame_render_size = { 150.0f, 38.0f };



//TODO:����N���X�쐬�̍ۂɈړ�
/******************** Weapon ********************/
// ����̏����ʒu
static const DirectX::XMFLOAT3 WEAPON_initial_position = { 0.0f, 0.0f, 0.0f };
// ����̍��{�̃I�t�Z�b�g
static const DirectX::XMVECTOR WEAPON_initial_root_offset = { 0, 0, 0 };
// ����̐�̃I�t�Z�b�g/�n���}�[
static const DirectX::XMVECTOR WEAPON_hammer_initial_tip_offset = { 0, 35, 0 };
// ����̐�̃I�t�Z�b�g/�X�s�A�[
static const DirectX::XMVECTOR WEAPON_spear_initial_tip_offset = { 0, 90, 0 };
// ����̐�̃I�t�Z�b�g/�\�[�h
static const DirectX::XMVECTOR WEAPON_sword_initial_tip_offset = { 0, 30, 0 };
// ����̓����蔻�蔼�a/�n���}�[
static const float WEAPON_hammer_radius = 0.5f;
// ����̓����蔻�蔼�a/�X�s�A�[
static const float WEAPON_spear_radius = 0.4f;
// ����̓����蔻�蔼�a/�\�[�h
static const float WEAPON_sword_radius = 0.6f;
// ����̃_���[�W��/�n���}�[
static const int WEAPON_hammer_damage = 2;
// ����̃_���[�W��/�X�s�A�[
static const int WEAPON_spear_damage = 1;
// ����̃_���[�W��/�\�[�h
static const int WEAPON_sword_damage = 1;

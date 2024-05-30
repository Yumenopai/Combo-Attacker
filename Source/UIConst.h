#pragma once
#include <DirectXMath.h>

/******************** Icon ********************/
// ����A�C�R��/�X�v���C�g�T�C�Y
static const DirectX::XMFLOAT2 WeaponIcon_sprite_size = { 300.0f, 300.f };
// ����A�C�R��/�T�C�Y
static const DirectX::XMFLOAT2 WeaponIcon_size = { 45.0f, 45.0f };
// ����A�C�R��/�ʒu_X
static const float WeaponIcon_position_x = 1013.0f;
// ����A�C�R��/�I�t�Z�b�g_Y
static const float WeaponIcon_offset_y = 2.0f;
// ����A�C�R��/�F������_a
static const float WeaponIcon_color_translucent_a = 0.3f;
// ����A�C�R��/�F����_rb
static const float WeaponIcon_color_decrease_rb = 0.4f;

// ����t���[��/�ʒu_X
static const float WeaponFrame_position_x = 1000.0f;
// ����t���[��/�I�t�Z�b�g
static const DirectX::XMFLOAT2 WeaponFrame_offset = { 65.0f, -10.0f };
// ����t���[��/�T�C�Y
static const DirectX::XMFLOAT2 WeaponFrame_render_size = { 70.0f, 70.0f };

// �{�^���t���[��/�X�v���C�g�T�C�Y
static const DirectX::XMFLOAT2 ButtonFrame_sprite_size = { 300.0f, 300.f };
// �{�^���t���[��/�T�C�Y
static const DirectX::XMFLOAT2 ButtonFrame_size = { 80.0f, 80.0f };

// �{�^���t���[��/X�{�^���ʒu_X
static const float ButtonFrame_button_X_position_x = 1100.0f;
// �{�^���t���[��/YA�{�^���ʒu_X
static const float ButtonFrame_button_YA_position_x = 1150.0f;
// �{�^���t���[��/B�{�^���ʒu_X
static const float ButtonFrame_button_B_position_x = 1200.0f;
// �{�^���t���[��/Y�{�^���ʒu_Y
static const float ButtonFrame_button_Y_position_y = 250.0f;
// �{�^���t���[��/XB�{�^���ʒu_Y
static const float ButtonFrame_button_XB_position_y = 300.0f;
// �{�^���t���[��/A�{�^���ʒu_Y
static const float ButtonFrame_button_A_position_y = 350.0f;

// �{�^���A�C�R��/�t���[������A�C�R���̈ʒu�I�t�Z�b�g
static const  DirectX::XMFLOAT2 ButtonIcon_frame_to_icon_offset = { 15.0f, 10.0f };

// �{�^���A�C�R��/�X�v���C�g�T�C�Y
static const DirectX::XMFLOAT2 ButtonIcon_sprite_size = { 300.0f, 300.f };
// �{�^���A�C�R��/�T�C�Y
static const DirectX::XMFLOAT2 ButtonIcon_render_size = { 50.0f,50.0f };

// �{�^���A�C�R��/�񕜖�̃J�b�g�ʒu�{��
static const int ButtonIcon_cut_rate_recover = 0;
// �{�^���A�C�R��/�\�[�h�̃J�b�g�ʒu�{��
static const int ButtonIcon_cut_rate_sword = 1;
// �{�^���A�C�R��/�n���}�[�̃J�b�g�ʒu�{��
static const int ButtonIcon_cut_rate_hammer = 2;
// �{�^���A�C�R��/�X�s�A�[�̃J�b�g�ʒu�{��
static const int ButtonIcon_cut_rate_spear = 3;

// �{�^���ɕ\�����閼�O/�X�v���C�g�T�C�Y
static const DirectX::XMFLOAT2 ButtonName_sprite_size = { 300.0f, 150.f };
// �{�^���ɕ\�����閼�O/�T�C�Y
static const DirectX::XMFLOAT2 ButtonName_render_size = { 50.0f, 25.0f };

/******************** Mission ********************/
// �~�b�V����/�X�v���C�g�T�C�Y
static const DirectX::XMFLOAT2 Mission_sprite_size = { 900.0f, 600.f };
// �~�b�V����/�ʒu
static const DirectX::XMFLOAT3 Mission_position = { 1040.0f, 10.f, 0.0f };
// �~�b�V����/�T�C�Y
static const DirectX::XMFLOAT2 Mission_render_size = { 225.0f, 150.0f };

// �~�b�V����/�\������
static const int Mission_display_timer_max = 180;

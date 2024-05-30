#pragma once
#include <DirectXMath.h>

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

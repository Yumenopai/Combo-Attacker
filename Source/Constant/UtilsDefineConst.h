#pragma once
#include <d3d11.h>
#include <DirectXMath.h>

// int/�u��
#define SC_INT static_cast<int>

/******************** TextOut ********************/
// ���s/�f�t�H���g
static const float TEXT_depth_default = 0.0f;
// �T�C�Y/�f�t�H���g
static const DirectX::XMFLOAT2 TEXT_display_size_default = { 12.0f, 16.0f };
// �ؔ��ʒu/�f�t�H���g
static const DirectX::XMFLOAT2 TEXT_cut_position_default = { 32.0f, 32.0f };
// �ؔ��T�C�Y/�f�t�H���g
static const DirectX::XMFLOAT2 TEXT_cut_size_default = { 16.0f, 16.0f };
// �p�x/�f�t�H���g
static const float TEXT_angle_default = 0.0f;

/******************** Sprite ********************/
// ���s/�f�t�H���g
static const float SPRITE_position_default_z = 0.0f;
// �ؔ��ʒu/�f�t�H���g
static const DirectX::XMFLOAT2 SPRITE_cut_position_default = { 0.0f, 0.0f };
// �p�x/�f�t�H���g
static const float SPRITE_angle_default = 0.0f;
// �F/�f�t�H���g
static const DirectX::XMFLOAT4 SPRITE_color_default = { 1.0f, 1.0f, 1.0f, 1.0f };

/******************** Gizmos ********************/
// �f�o�b�O�~���̊p�x/�f�t�H���g
static const DirectX::XMFLOAT3 GIZMOS_cylinder_angle_default = { 0.0f, 0.0f, 0.0f };
// �J���[/��
static const DirectX::XMFLOAT4 GIZMOS_color_red = { 1.0f, 0.0f, 0.0f, 1.0f };
// �J���[/��
static const DirectX::XMFLOAT4 GIZMOS_color_green = { 0.0f, 1.0f, 0.0f, 1.0f };
// �J���[/��
static const DirectX::XMFLOAT4 GIZMOS_color_blue = { 0.0f, 0.0f, 1.0f, 1.0f };

/******************** Player ********************/
// Player1P/�V���A���i���o�[
static const int PL1P_NO = 0;
// PlayerAI/�V���A���i���o�[
static const int PLAI_NO = 1;

/******************** SceneGame ********************/
// �J����Y������
static const float SCENEGAME_camera_y = 0.5f;
// �V���h�E�}�b�v�F
static const DirectX::XMFLOAT3 SCENEGAME_shadow_color = { 0.5f, 0.5f, 0.5f };


#pragma once
#include <d3d11.h>
#include <DirectXMath.h>

// int/�u��
#define SC_INT static_cast<int>

/******************** TextOut ********************/
// ���s/�f�t�H���g
static const float TEXT_Depth_Default = 0.0f;
// ��/�f�t�H���g
static const float TEXT_SizeWidge_Default = 12.0f;
// ����/�f�t�H���g
static const float TEXT_SizeHeight_Default = 16.0f;
// �ؔ��ʒu_X/�f�t�H���g
static const float TEXT_CutPositionX_Default = 32.0f;
// �ؔ��ʒu_Y/�f�t�H���g
static const float TEXT_CutPositionY_Default = 32.0f;
// �ؔ��T�C�Y_��/�f�t�H���g
static const float TEXT_CutWidge_Default = 16.0f;
// �ؔ��T�C�Y_����/�f�t�H���g
static const float TEXT_CutHeight_Default = 16.0f;
// �p�x/�f�t�H���g
static const float TEXT_Angle_Default = 0.0f;

/******************** Sprite ********************/
// �e�N�X�`���Ȃ�
static const float Sprite_NoneTexture = 0.0f;
// ���s/�f�t�H���g
static const float Sprite_Position_Z_Default = 0.0f;
// ���s/�f�t�H���g
static const DirectX::XMFLOAT2 Sprite_CutPosition_Default = { 0.0f, 0.0f };
// �p�x/�f�t�H���g
static const float Sprite_Angle_Default = 0.0f;
// �F/�f�t�H���g
static const DirectX::XMFLOAT4 Sprite_Color_Default = { 1.0f, 1.0f, 1.0f, 1.0f };

/******************** Gizmos ********************/
// �f�o�b�O�~���̊p�x/�f�t�H���g
static const DirectX::XMFLOAT3 GIZMOS_CylinderAngle_Default = { 0.0f, 0.0f, 0.0f };
// �J���[/��
static const DirectX::XMFLOAT4 GIZMOS_Color_Red = { 1.0f, 0.0f, 0.0f, 1.0f };
// �J���[/��
static const DirectX::XMFLOAT4 GIZMOS_Color_Green = { 0.0f, 1.0f, 0.0f, 1.0f };
// �J���[/��
static const DirectX::XMFLOAT4 GIZMOS_Color_Blue = { 0.0f, 0.0f, 1.0f, 1.0f };

/******************** Player ********************/
static const int PLAYER_Count = 2;

/******************** SceneGame ********************/
// �J����Y������
static const float SCENEGAME_Camera_Y = 0.5f;
// �V���h�E�}�b�v�F
static const DirectX::XMFLOAT3 SCENEGAME_ShadowColor = { 0.5f, 0.5f, 0.5f };


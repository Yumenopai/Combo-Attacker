#pragma once
#include <DirectXMath.h>

/******************** Icon ********************/
// ����A�C�R��/�X�v���C�g�T�C�Y
static const DirectX::XMFLOAT2 WeaponIcon_SpriteSize = { 300.0f, 300.f };
// ����A�C�R��/�T�C�Y
static const DirectX::XMFLOAT2 WeaponIcon_Size = { 45.0f, 45.f };
// ����A�C�R��/�ʒu_X
static const float WeaponIcon_Position_X = 1013.0f;
// ����A�C�R��/�I�t�Z�b�g_Y
static const float WeaponIcon_Offset_Y = 2.0f;
// ����A�C�R��/�F������_a
static const float WeaponIcon_ColorTranslucent_a = 0.3f;
// ����A�C�R��/�F����_rb
static const float WeaponIcon_ColorDecrease_rb = 0.4f;

// ����t���[��/�ʒu_X
static const float WeaponFrame_Position_X = 1000.0f;
// ����t���[��/�I�t�Z�b�g_X
static const float WeaponFrame_Offset_X = 65.0f;
// ����t���[��/�I�t�Z�b�g_Y
static const float WeaponFrame_Offset_Y = -10.0f;
// ����t���[��/�T�C�Y
static const DirectX::XMFLOAT2 WeaponFrame_Size = { 70.0f, 70.0f };

// �{�^���t���[��/�X�v���C�g�T�C�Y
static const DirectX::XMFLOAT2 ButtonFrame_SpriteSize = { 300.0f, 300.f };
// �{�^���t���[��/�T�C�Y
static const DirectX::XMFLOAT2 ButtonFrame_Size = { 80.0f, 80.0f };

// �{�^���t���[��/X�{�^���ʒu_X
static const float ButtonFrame_ButtonX_Position_X = 1100.0f;
// �{�^���t���[��/YA�{�^���ʒu_X
static const float ButtonFrame_ButtonYA_Position_X = 1150.0f;
// �{�^���t���[��/B�{�^���ʒu_X
static const float ButtonFrame_ButtonB_Position_X = 1200.0f;
// �{�^���t���[��/Y�{�^���ʒu_Y
static const float ButtonFrame_ButtonY_Position_Y = 250.0f;
// �{�^���t���[��/XB�{�^���ʒu_Y
static const float ButtonFrame_ButtonXB_Position_Y = 300.0f;
// �{�^���t���[��/A�{�^���ʒu_Y
static const float ButtonFrame_ButtonA_Position_Y = 350.0f;

// �{�^���A�C�R��/�t���[������A�C�R���̈ʒu�I�t�Z�b�g_X
static const float ButtonIcon_FrameToIconOffset_X = 15.0f;
// �{�^���A�C�R��/�t���[������A�C�R���̈ʒu�I�t�Z�b�g_Y
static const float ButtonIcon_FrameToIconOffset_Y = 10.0f;

// �{�^���A�C�R��/�X�v���C�g�T�C�Y
static const DirectX::XMFLOAT2 ButtonIcon_SpriteSize = { 300.0f, 300.f };
// �{�^���A�C�R��/�T�C�Y
static const DirectX::XMFLOAT2 ButtonIcon_Size = { 50.0f,50.0f };

// �{�^���A�C�R��/�񕜖�̃J�b�g�ʒu�{��
static const int ButtonIcon_CutRate_Recover = 0;
// �{�^���A�C�R��/�\�[�h�̃J�b�g�ʒu�{��
static const int ButtonIcon_CutRate_Sword = 1;
// �{�^���A�C�R��/�X�s�A�[�̃J�b�g�ʒu�{��
static const int ButtonIcon_CutRate_Spear = 3;
// �{�^���A�C�R��/�n���}�[�̃J�b�g�ʒu�{��
static const int ButtonIcon_CutRate_Hammer = 2;

// �{�^���ɕ\�����閼�O/�X�v���C�g�T�C�Y
static const DirectX::XMFLOAT2 ButtonName_SpriteSize = { 300.0f, 150.f };
// �{�^���ɕ\�����閼�O/�T�C�Y
static const DirectX::XMFLOAT2 ButtonName_Size = { 50.0f, 25.0f };

/******************** Mission ********************/
// �~�b�V����/�X�v���C�g�T�C�Y
static const DirectX::XMFLOAT2 Mission_SpriteSize = { 900.0f, 600.f };
// �~�b�V����/�ʒu
static const DirectX::XMFLOAT3 Mission_Position = { 1040.0f, 10.f, 0.0f };
// �~�b�V����/�T�C�Y
static const DirectX::XMFLOAT2 Mission_Size = { 225.0f, 150.0f };

// �~�b�V����/�\������
static const int Mission_DisplayTimerMax = 180;

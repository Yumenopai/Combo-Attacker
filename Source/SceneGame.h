#pragma once

#include "UtilsDefineConst.h"
#include "EnemyConst.h"
#include "UIConst.h"
#include "Graphics/FontSprite.h"
#include "Stage.h"
#include "Light/Light.h"
#include "Camera/CameraController.h"
#include "Player1P.h"
#include "playerAI.h"
#include "EnemySlime.h"
#include "EnemyTurtleShell.h"
#include "EnemyDragon.h"
#include "Shader/Skybox.h"

//�Q�[���V�[��
class SceneGame : public Scene
{
private:
	// �~�b�V����
	enum class Mission
	{
		GetWeapon = 0,	// ����l��
		UpperAttack,	// �U���͑���
		SpecialAttack,	// �X�y�V�����Z
		Recover,		// ��

		MaxCount
	};

private:
	// �X�v���C�g/�{�^���t���[��
	std::unique_ptr<Sprite> spriteButtonFrame;
	// �X�v���C�g/����A�C�R��
	std::unique_ptr<Sprite> spriteWeaponIcon;
	// �X�v���C�g/�{�^����̖��O
	std::unique_ptr<Sprite> spriteNameOnButton;
	// �X�v���C�g/�~�b�V�����t���[��
	std::unique_ptr<Sprite> spriteMissionFrame;
	// �X�v���C�g/�~�b�V�����e�L�X�g
	std::unique_ptr<Sprite> spriteMissionText;
	// �X�v���C�g/PlayerAI���b�Z�[�W�e�L�X�g
	std::unique_ptr<Sprite> spriteMessageText;

	// �~�b�V�����e�L�X�g/�^�C�}�[
	int missionSpriteTimer = 0;
	// �~�b�V�����e�L�X�g/���݂̃i���o�[
	Mission missionSpriteNumber = Mission::GetWeapon;
	// �~�b�V�����X�v���C�g/�J�b�g�ʒu
	XMFLOAT2 missionSpriteCutPosition[SC_INT(Mission::MaxCount)] = {
		{ SPRITE_cut_position_default.x, SPRITE_cut_position_default.y },
		{ Mission_sprite_size.x,			Mission_sprite_size.y },
		{ Mission_sprite_size.x,			SPRITE_cut_position_default.y },
		{ SPRITE_cut_position_default.x, Mission_sprite_size.y },
	};

	// �J�����R���g���[���[
	std::unique_ptr<CameraController> cameraController;
	// �X�J�C�{�b�N�X
	std::unique_ptr<SkyBox> skyBox;
	// �X�e�[�W
	std::unique_ptr<Stage> stage;
	// �v���C���[/1P
	std::unique_ptr<Player1P> player1P;
	// �v���C���[/AI
	std::unique_ptr<PlayerAI> playerAI;
	// �G/�X���C��
	std::unique_ptr<EnemySlime> enemySlime[enemy_slime_count];
	// �G/�b��
	std::unique_ptr<EnemyTurtleShell> enemyTurtleShell[enemy_turtleShell_count];
	// �h���S��
	std::unique_ptr<EnemyDragon> enemyDragon;
	// �e�L�X�g�t�H���g
	std::unique_ptr<FontSprite> font;

	// �Q�[�W�S��/�G�E�v���C���[����
	std::unique_ptr<Sprite> gauge;

	// ���ʕ\��/�^�C�}�[
	float waterTimer = 0.0f;

	// ImGui�p/�ʒu
	XMFLOAT3 position = { 0,0,0 };
	// ImGui�p/�p�x
	XMFLOAT3 angle = { 0,0,0 };
	// ImGui�p/�X�P�[��
	XMFLOAT3 scale = { 1,1,1 };
	// ImGui�p/�I�𒆂̃m�[�h
	Model::Node* selectionNode = nullptr;

public:
	SceneGame() {}
	~SceneGame() override {}

	// ������
	void Initialize()override;

	// �I����
	void Finalize()override;

	// �X�V����
	void Update(float elapsedTime) override;
	// �`�揈��
	void Render() override;

	// 1P��AI����������������Ă��邩
	bool PlayersHaveAnySameWeapon();

private:
	// �G�l�~�[HP�Q�[�W�`��
	void RenderEnemyGauge(ID3D11DeviceContext* dc, const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection);
	// �{�^��UI�`��
	void RenderButtonUI(ID3D11DeviceContext* dc);
	// �~�b�V����UI�`��
	void RenderMissionUI(ID3D11DeviceContext* dc);

	// ����ɑΉ�����X���W�X�v���C�g�J�b�g�ʒu��Ԃ�
	float SpriteCutPositionX(Player::AttackType at);

	// �V���h�E�}�b�vGUI�`��
	void DrawShadowMapGUI();
	// �V�[��GUI�`��
	void DrawSceneGUI();
	// �v���p�e�BGUI�`��
	void DrawPropertyGUI();
};

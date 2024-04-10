#pragma once

#include "Graphics/FontSprite.h"
#include "Stage.h"
#include "Light/Light.h"
#include "Camera/CameraController.h"
#include "Player1P.h"
#include "playerAI.h"
#include "EnemySlime.h"
#include "EnemyTurtleShell.h"
#include "EnemyBlue.h"
#include "Shader/Skybox.h"

//�Q�[���V�[��
class SceneGame : public Scene
{
public:
	SceneGame(){}
	~SceneGame() override {}

	// ������
	void Initialize()override;

	// �I����
	void Finalize()override;

	// �X�V����
	void Update(float elapsedTime) override;
	//�`�揈��
	void Render() override;

private:
	//�G�l�~�[HP�Q�[�W�`��
	void RenderEnemyGauge(
		ID3D11DeviceContext* dc,
		const DirectX::XMFLOAT4X4& view,
		const DirectX::XMFLOAT4X4& projection
	);
	//�L�����N�^�[���O�`��
	void RenderCharacterName(
		const RenderContext& rc,
		const DirectX::XMFLOAT4X4& view,
		const DirectX::XMFLOAT4X4& projection
	);

	//�V�[��GUI�`��
	void DrawSceneGUI();
	//�v���p�e�BGUI�`��
	void DrawPropertyGUI();


private:
	static const int enemySlimeCount = 4;
	static const int enemyTurtleShellCount = 2;

	std::unique_ptr<Sprite> sprites[8];
	std::unique_ptr<SkyBox> skyBox;

	std::unique_ptr<Stage> stage;
	std::unique_ptr<Player1P> player1P;
	std::unique_ptr<PlayerAI> playerAI;
	std::unique_ptr<EnemySlime> enemySlime[enemySlimeCount];
	std::unique_ptr<EnemyTurtleShell> enemyTurtleShell[enemyTurtleShellCount];
	std::unique_ptr<EnemyBlue> enemyBlue;
	std::unique_ptr<FontSprite> font;

	std::unique_ptr<Sprite> gauge;

	XMFLOAT3	position = { 0,0,0 };
	XMFLOAT3	angle = { 0,0,0 };
	XMFLOAT3	scale = { 1,1,1 };

	XMFLOAT3	ViewPosition = { 0,0,0 };

	float gameTimer = 60.0f;

	float timer = 0.0f;
	float grav = 20.0f;

	float waterTimer = 0.0f;

	Model::Node* selectionNode = nullptr;
	std::unique_ptr<CameraController> cameraController;

	bool		animationLoop = false;
	float		animationBlendSeconds = 0;
};


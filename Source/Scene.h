#pragma once

#include <memory>
#include "Graphics/Sprite.h"
#include "Camera/Camera.h"
#include "Graphics/Model.h"
#include "Camera/CameraController.h"
#include "Camera/FreeCameraController.h"
#include "Light.h"
#include "PostEffect.h"

//�V�[�����
class Scene
{
public:
	Scene() {}
	virtual ~Scene() {}

	virtual void Initialize() = 0;
	virtual void Finalize() = 0;
	virtual void Update(float elapsedTime) = 0;
	virtual void Render() = 0;

	bool IsReady() const { return ready; }
	void SetReady() { ready = true; }

private:
	bool ready = false;
};










//�X�v���C�g�e�X�g�V�[��
class SpriteTestScene : public Scene
{
public:
	SpriteTestScene();
	~SpriteTestScene() override = default;

	//�`�揈��
	void Render() override;

private:
	std::unique_ptr<Sprite> sprites[8];
};

//�[�x�e�X�g�V�[��
class DepthTestScene : public Scene
{
public:
	DepthTestScene();
	~DepthTestScene() override = default;

	//�`�揈��
	void Render() override;

private:
	std::unique_ptr<Sprite> sprite;
};

//�u�����h�e�X�g�V�[��
class BlendTestScene : public Scene
{
public:
	BlendTestScene();
	~BlendTestScene() override = default;

	// �`�揈��
	void Render() override;

private:
	std::unique_ptr<Sprite> sprite;
};

//���X�^���C�Y�e�X�g�V�[��
class RasterizeTestScene : public Scene
{
public:
	RasterizeTestScene();
	~RasterizeTestScene() override = default;

	//�`�揈��
	void Render() override;

private:
	std::unique_ptr<Sprite> sprite;
};

//�M�Y���e�X�g�V�[��
class GizmosTestScene : public Scene
{
public:
	GizmosTestScene();
	~GizmosTestScene() override = default;

	//�`�揈��
	void Update(float elapsedTime) override;
	//�`�揈��
	void Render() override;

private:
	//Camera	camera;
	float	rotation = 0;
};

//���f���e�X�g�V�[��
class ModelTestScene : public Scene
{
public:
	ModelTestScene() {};
	~ModelTestScene() override {};

	// ������
	void Initialize()override;
	// �I����
	void Finalize()override {};

	//�`�揈��
	void Update(float elapsedTime) override;
	//�`�揈��
	void Render() override;

private:
	//�V�[��GUI�`��
	void DrawSceneGUI();
	//�v���p�e�BGUI�`��
	void DrawPropertyGUI();
	//�A�j���[�V����GUI�`��
	void DrawAnimationGUI();

	//Camera		camera;
	std::unique_ptr<Model> model;

	XMFLOAT3	position = { 0,0,0 };
	XMFLOAT3	angle = { 0,0,0 };
	XMFLOAT3	scale = { 1,1,1 };
	
	Model::Node* selectionNode = nullptr;
	FreeCameraController cameraController;

	bool		animationLoop = false;
	float		animationBlendSeconds = 0;
	LightManager lightManager;
};

//�|�X�g�G�t�F�N�g�e�X�g�V�[��
class PostEffectTestScene : public Scene
{
public:
	PostEffectTestScene();
	~PostEffectTestScene() override = default;

	//�`�揈��
	void Render() override;

private:
	//�|�X�g�G�t�F�N�gGUI�`��
	void DrawPostEffectGUI();

	std::unique_ptr<Sprite> sprite;
	std::unique_ptr<PostEffect> postEffect;
};

//�V���h�E�e�X�g�V�[��
class ShadowTestScene : public Scene
{
public:
	ShadowTestScene() {}
	~ShadowTestScene() override {}

	// ������
	void Initialize()override;

	// �I����
	void Finalize()override {};

	// �X�V����
	void Update(float elapsedTime) override;
	//�`�揈��
	void Render() override;

private:
	//�V���h�E�}�b�vGUI�`��
	void DrawShadowMapGUI();

	FreeCameraController cameraController;
	LightManager	lightManager;
	std::unique_ptr<Model>	stage;
	std::unique_ptr<Model>  character;
};
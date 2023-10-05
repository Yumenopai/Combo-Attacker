#pragma once

#include <memory>
#include "Graphics/Sprite.h"
#include "Camera/Camera.h"
#include "Graphics/Model.h"
#include "Camera/CameraController.h"
#include "Camera/FreeCameraController.h"
#include "Light.h"
#include "PostEffect.h"

//シーン基底
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










//スプライトテストシーン
class SpriteTestScene : public Scene
{
public:
	SpriteTestScene();
	~SpriteTestScene() override = default;

	//描画処理
	void Render() override;

private:
	std::unique_ptr<Sprite> sprites[8];
};

//深度テストシーン
class DepthTestScene : public Scene
{
public:
	DepthTestScene();
	~DepthTestScene() override = default;

	//描画処理
	void Render() override;

private:
	std::unique_ptr<Sprite> sprite;
};

//ブレンドテストシーン
class BlendTestScene : public Scene
{
public:
	BlendTestScene();
	~BlendTestScene() override = default;

	// 描画処理
	void Render() override;

private:
	std::unique_ptr<Sprite> sprite;
};

//ラスタライズテストシーン
class RasterizeTestScene : public Scene
{
public:
	RasterizeTestScene();
	~RasterizeTestScene() override = default;

	//描画処理
	void Render() override;

private:
	std::unique_ptr<Sprite> sprite;
};

//ギズモテストシーン
class GizmosTestScene : public Scene
{
public:
	GizmosTestScene();
	~GizmosTestScene() override = default;

	//描画処理
	void Update(float elapsedTime) override;
	//描画処理
	void Render() override;

private:
	//Camera	camera;
	float	rotation = 0;
};

//モデルテストシーン
class ModelTestScene : public Scene
{
public:
	ModelTestScene() {};
	~ModelTestScene() override {};

	// 初期化
	void Initialize()override;
	// 終了化
	void Finalize()override {};

	//描画処理
	void Update(float elapsedTime) override;
	//描画処理
	void Render() override;

private:
	//シーンGUI描画
	void DrawSceneGUI();
	//プロパティGUI描画
	void DrawPropertyGUI();
	//アニメーションGUI描画
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

//ポストエフェクトテストシーン
class PostEffectTestScene : public Scene
{
public:
	PostEffectTestScene();
	~PostEffectTestScene() override = default;

	//描画処理
	void Render() override;

private:
	//ポストエフェクトGUI描画
	void DrawPostEffectGUI();

	std::unique_ptr<Sprite> sprite;
	std::unique_ptr<PostEffect> postEffect;
};

//シャドウテストシーン
class ShadowTestScene : public Scene
{
public:
	ShadowTestScene() {}
	~ShadowTestScene() override {}

	// 初期化
	void Initialize()override;

	// 終了化
	void Finalize()override {};

	// 更新処理
	void Update(float elapsedTime) override;
	//描画処理
	void Render() override;

private:
	//シャドウマップGUI描画
	void DrawShadowMapGUI();

	FreeCameraController cameraController;
	LightManager	lightManager;
	std::unique_ptr<Model>	stage;
	std::unique_ptr<Model>  character;
};
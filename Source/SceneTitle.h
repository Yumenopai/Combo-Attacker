#include "Graphics/FontSprite.h"
#include "Stage.h"
#include "Player.h"

//ゲームシーン
class SceneTitle : public Scene
{
public:
	SceneTitle() {}
	~SceneTitle() override {}

	// 初期化
	void Initialize()override;

	// 終了化
	void Finalize()override;

	// 更新処理
	void Update(float elapsedTime) override;
	//描画処理
	void Render() override;

private:
	//シーンGUI描画
	void DrawSceneGUI();
	//プロパティGUI描画
	void DrawPropertyGUI();

	std::unique_ptr<Sprite> sprites[8];

	std::unique_ptr<Stage> stage;
	std::unique_ptr<Player> player;
	std::unique_ptr<FontSprite> font;

	XMFLOAT3	position = { 0,0,0 };
	XMFLOAT3	angle = { 0,0,0 };
	XMFLOAT3	scale = { 1,1,1 };

	XMFLOAT3	ViewPosition = { 0,0,0 };
	XMFLOAT3	blackPosition = { -1280,0,0 };

	bool isInit = false;
	float timer = 0.0f;
	XMVECTOR playerToTarget = { 0,0,0 };
	float zoomRate = 0.01f;
	XMVECTOR zoomRateCalculation = { 0,0,0 };

	float grav = 20.0f;

	Model::Node* selectionNode = nullptr;
	FreeCameraController freeCameraController;
	std::unique_ptr<CameraController> cameraController;

	int playCount;
	float		animationBlendSeconds = 0;
	LightManager lightManager;
};


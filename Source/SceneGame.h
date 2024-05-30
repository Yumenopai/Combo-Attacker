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

//ゲームシーン
class SceneGame : public Scene
{
private:
	// ミッション
	enum class Mission
	{
		GetWeapon = 0,	// 武器獲得
		UpperAttack,	// 攻撃力増加
		SpecialAttack,	// スペシャル技
		Recover,		// 回復

		MaxCount
	};

private:
	// スプライト/ボタンフレーム
	std::unique_ptr<Sprite> spriteButtonFrame;
	// スプライト/武器アイコン
	std::unique_ptr<Sprite> spriteWeaponIcon;
	// スプライト/ボタン上の名前
	std::unique_ptr<Sprite> spriteNameOnButton;
	// スプライト/ミッションフレーム
	std::unique_ptr<Sprite> spriteMissionFrame;
	// スプライト/ミッションテキスト
	std::unique_ptr<Sprite> spriteMissionText;
	// スプライト/PlayerAIメッセージテキスト
	std::unique_ptr<Sprite> spriteMessageText;

	// ミッションテキスト/タイマー
	int missionSpriteTimer = 0;
	// ミッションテキスト/現在のナンバー
	Mission missionSpriteNumber = Mission::GetWeapon;
	// ミッションスプライト/カット位置
	XMFLOAT2 missionSpriteCutPosition[SC_INT(Mission::MaxCount)] = {
		{ SPRITE_cut_position_default.x, SPRITE_cut_position_default.y },
		{ Mission_sprite_size.x,			Mission_sprite_size.y },
		{ Mission_sprite_size.x,			SPRITE_cut_position_default.y },
		{ SPRITE_cut_position_default.x, Mission_sprite_size.y },
	};

	// カメラコントローラー
	std::unique_ptr<CameraController> cameraController;
	// スカイボックス
	std::unique_ptr<SkyBox> skyBox;
	// ステージ
	std::unique_ptr<Stage> stage;
	// プレイヤー/1P
	std::unique_ptr<Player1P> player1P;
	// プレイヤー/AI
	std::unique_ptr<PlayerAI> playerAI;
	// 敵/スライム
	std::unique_ptr<EnemySlime> enemySlime[enemy_slime_count];
	// 敵/甲羅
	std::unique_ptr<EnemyTurtleShell> enemyTurtleShell[enemy_turtleShell_count];
	// ドラゴン
	std::unique_ptr<EnemyDragon> enemyDragon;
	// テキストフォント
	std::unique_ptr<FontSprite> font;

	// ゲージ全般/敵・プレイヤー共通
	std::unique_ptr<Sprite> gauge;

	// 水面表現/タイマー
	float waterTimer = 0.0f;

	// ImGui用/位置
	XMFLOAT3 position = { 0,0,0 };
	// ImGui用/角度
	XMFLOAT3 angle = { 0,0,0 };
	// ImGui用/スケール
	XMFLOAT3 scale = { 1,1,1 };
	// ImGui用/選択中のノード
	Model::Node* selectionNode = nullptr;

public:
	SceneGame() {}
	~SceneGame() override {}

	// 初期化
	void Initialize()override;

	// 終了化
	void Finalize()override;

	// 更新処理
	void Update(float elapsedTime) override;
	// 描画処理
	void Render() override;

	// 1PとAIが同じ武器を持っているか
	bool PlayersHaveAnySameWeapon();

private:
	// エネミーHPゲージ描画
	void RenderEnemyGauge(ID3D11DeviceContext* dc, const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection);
	// ボタンUI描画
	void RenderButtonUI(ID3D11DeviceContext* dc);
	// ミッションUI描画
	void RenderMissionUI(ID3D11DeviceContext* dc);

	// 武器に対応するX座標スプライトカット位置を返す
	float SpriteCutPositionX(Player::AttackType at);

	// シャドウマップGUI描画
	void DrawShadowMapGUI();
	// シーンGUI描画
	void DrawSceneGUI();
	// プロパティGUI描画
	void DrawPropertyGUI();
};

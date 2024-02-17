#pragma once

#include <memory>
#include "Shader/Shader.h"
#include "Graphics/Model.h"
#include "Character.h"
#include "Enemy.h"
#include "Effect.h"
#include "Player.h"


//プレイヤー
class Player1P : public Player
{
private:

public:
	Player1P();
	~Player1P() override;

	//インスタンス取得
	static Player1P& Instance();

	//更新
	void Update(float elapsedTime);

	//描画
	void ShadowRender(const RenderContext& rc, ShadowMap* shadowMap);
	//描画
	void Render(const RenderContext& rc, ModelShader* shader);
	void PrimitiveRender(const RenderContext& rc);
	void HPBarRender(const RenderContext& rc, Sprite* gauge);

	void UpdateJump(float elapsedTime);
	// 入力処理
	bool InputJumpButtonDown();
	bool InputJumpButton();
	bool InputJumpButtonUp();
	bool InputHammerButton();
	bool InputSwordButton();
	bool InputSpearButton();

private:
	// スティック入力値から移動ベクトルを取得
	XMFLOAT3 GetMoveVec() const;
	// 移動入力処理
	bool InputMove(float elapsedTime);
};
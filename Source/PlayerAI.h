#pragma once

#include <memory>
#include "Shader/Shader.h"
#include "Graphics/Model.h"
#include "Character.h"
#include "Player.h"
#include "Enemy.h"
#include "Effect.h"


//プレイヤー
class PlayerAI : public Player
{
private:
	enum class InputState
	{
		None,
		Run,
		Jump,
		Hammer,
		Sword,
		Spear,
	};
	InputState nowInput = InputState::None;
	InputState oldInput, nextInput;

	XMFLOAT3 targetPosition = { 0,0,0 };

public:
	PlayerAI();
	~PlayerAI() override;

	//インスタンス取得
	static PlayerAI& Instance();

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

	// 擬似ボタン判定
	bool InputButtonDown(InputState button)
	{
		if (oldInput == button) return false;
		if (nowInput == button) return true;
		return false;
	}
	bool InputButton(InputState button)
	{
		if (nowInput == button) return true;
		return false;
	}
	bool InputButtonUp(InputState button)
	{
		if (nowInput == button) return false;
		if (oldInput == button) return true;
		return false;
	}


protected:
	//着地した時に呼ばれる
	void OnLanding(float elapsedTime) override;

private:
	// スティック入力値から移動ベクトルを取得
	XMFLOAT3 GetMoveVec() const;
	// 移動入力処理
	bool InputMove(float elapsedTime);

	//デバッグ
	void DebugMenu();
};
#pragma once
#include "Player.h"

//プレイヤー
class PlayerAI : public Player
{
private:
	InputState nowInput = InputState::None;
	InputState oldInput, nextInput;

public:
	PlayerAI();
	~PlayerAI() override;

	//インスタンス取得
	static PlayerAI& Instance();

	//更新
	void Update(float elapsedTime);

	//描画
	void HPBarRender(const RenderContext& rc, Sprite* gauge);

	// ボタン判定
	bool InputButtonDown(InputState button) override;
	bool InputButton(InputState button) override;
	bool InputButtonUp(InputState button) override;

private:
	// 移動ベクトル
	XMFLOAT3 GetMoveVec() const;
};
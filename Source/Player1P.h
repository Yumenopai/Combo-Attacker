#pragma once
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
	void HPBarRender(const RenderContext& rc, Sprite* gauge);

	// ボタン判定
	bool InputButtonDown(InputState button) override;
	bool InputButton(InputState button) override;
	bool InputButtonUp(InputState button) override;

private:
	// 移動ベクトル
	XMFLOAT3 GetMoveVec() const;
};
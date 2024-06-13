#pragma once
#include "Core/Player.h"

//プレイヤー
class Player1P : public Player
{
private:

public:
	Player1P();
	~Player1P() override;

	// インスタンス取得
	static Player1P& Instance();

	// 更新
	void Update(float elapsedTime);

	// ボタン判定(押下時)
	bool InputButtonDown(InputState button) override;
	// ボタン判定(入力時)
	bool InputButton(InputState button) override;
	// ボタン判定(押上時)
	bool InputButtonUp(InputState button) override;

	// 武器変更処理
	void InputChangeWeapon(AttackType weapon = AttackType::None) override;
	// ターゲット回復処理
	void InputRecover() override;

private:
	// 移動ベクトル
	XMFLOAT3 GetMoveVec() const;
};
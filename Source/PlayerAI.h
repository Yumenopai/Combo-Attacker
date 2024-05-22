#pragma once
#include "Player.h"

//プレイヤー
class PlayerAI : public Player
{
private:
	InputState nowInput = InputState::None;
	InputState oldInput, nextInput;

	// 逃避
	Enemy* avoidEnemy = nullptr;
	bool ranAwayFromEnemy = false;
	bool ranAwayFromPlayer1P = false;

	int waitTimer = 0;

public:
	PlayerAI();
	~PlayerAI() override;

	// インスタンス取得
	static PlayerAI& Instance();

	// 更新
	void Update(float elapsedTime);
	// 攻撃時の更新処理
	void AttackUpdate();

	// ボタン判定(押下時)
	bool InputButtonDown(InputState button) override;
	// ボタン判定(入力時)
	bool InputButton(InputState button) override;
	// ボタン判定(押上時)
	bool InputButtonUp(InputState button) override;

	// 武器変更処理
	void InputChangeArm(AttackType arm = AttackType::None) override;
	// ターゲット回復処理
	void InputRecover() override;

private:
	// 移動ベクトル
	XMFLOAT3 GetMoveVec() const;
};
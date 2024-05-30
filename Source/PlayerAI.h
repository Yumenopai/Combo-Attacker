#pragma once
#include "Player.h"

//プレイヤー
class PlayerAI : public Player
{
private:
	// 現在入力中
	InputState nowInput = InputState::None;
	// ひとつ前の入力
	InputState oldInput;
	// 次入力する
	InputState nextInput;

	// 逃避する敵(単体)
	Enemy* currentAvoidEnemy = nullptr;
	// 敵から逃げる
	bool ranAwayFromEnemy = false;
	// 1Pから逃げる
	bool ranAwayFromPlayer1P = false;

	// とどめを刺すための待機タイマー
	int waitTimer = 0;
	// 直前の敵とどめ対象
	Enemy* lastAvoidEnemy = nullptr;

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
	void InputChangeWeapon(AttackType weapon = AttackType::None) override;
	// ターゲット回復処理
	void InputRecover() override;

private:
	// 移動ベクトル
	XMFLOAT3 GetMoveVec() const;
};
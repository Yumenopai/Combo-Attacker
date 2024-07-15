#pragma once
#include "Core/Player.h"

//プレイヤー
class PlayerAI : public Player
{
public:
	enum class Message
	{
		None = -1,
		Normal,			// 通常
		Find,			// 発見
		Attack,			// 攻撃
		KnockDown,		// とどめして
		RanAway,		// 逃げる
		Indifference,	// もう知らない
		Recover,		// 回復する
		Damage,			// ダメージを受ける
		LevelUp,		// レベルアップ
		Recovered,		// 回復してくれた
		
		MaxCount,
	};

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
	float waitTimer = 0;
	// 直前の敵とどめ対象
	Enemy* lastAvoidEnemy = nullptr;

	// 現在のメッセージ
	Message nowMessage[message_max_count];
	// メッセージ
	int nowMessageTypeNumber[message_max_count];
	// 次表示させるメッセージ
	Message nextMessage;
	// メッセージタイマー
	float messageTimer;

public:
	PlayerAI();
	~PlayerAI() override;

	// インスタンス取得
	static PlayerAI& Instance();

	// 更新
	void Update(float elapsedTime);
	// 攻撃時の更新処理
	void AttackUpdate(float elapsedTime);
	// メッセージの更新処理
	void MessageUpdate(float elapsedTime);
	// メッセージを更新するか
	bool IsSetMessageUpdate();
	// メッセージUI
	void RenderMessageUI(ID3D11DeviceContext* dc, Sprite* icon, Sprite* frame, Sprite* message);

	// レベルアップ時の処理
	void AddLevel(int lv) override;

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

	// Setter
	void SetRanAwayFromEnemy(bool isRanAway) { ranAwayFromEnemy = isRanAway; }
	void SetShowMessage(Message message) { nextMessage = message; }

private:
	// 移動ベクトル
	XMFLOAT3 GetMoveVec() const;
};
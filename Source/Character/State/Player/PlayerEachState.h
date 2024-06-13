#pragma once
#include "PlayerStateBase.h"

// 待機
class StateIdle : public PlayerStateBase
{
public:
	StateIdle(Player* player) : PlayerStateBase(player) {}

	// 遷移
	void Init() override;
	// 更新
	void Update(float elapsedTime) override;
};

// 待機->移動
class StateIdleToRun : public PlayerStateBase
{
public:
	StateIdleToRun(Player* player) : PlayerStateBase(player) {}

	// 遷移
	void Init() override;
	// 更新
	void Update(float elapsedTime) override;
};

// 移動
class StateRun : public PlayerStateBase
{
public:
	StateRun(Player* player) : PlayerStateBase(player) {}

	// 遷移
	void Init() override;
	// 更新
	void Update(float elapsedTime) override;
};

// 移動->待機
class StateRunToIdle : public PlayerStateBase
{
public:
	StateRunToIdle(Player* player) : PlayerStateBase(player) {}

	// 遷移
	void Init() override;
	// 更新
	void Update(float elapsedTime) override;
};

// ジャンプ開始
class StateJumpStart : public PlayerStateBase
{
public:
	StateJumpStart(Player* player) : PlayerStateBase(player) {}

	// 遷移
	void Init() override;
	// 更新
	void Update(float elapsedTime) override;
};

// ジャンプ中
class StateJumpLoop : public PlayerStateBase
{
public:
	StateJumpLoop(Player* player) : PlayerStateBase(player) {}

	// 遷移
	void Init() override;
	// 更新
	void Update(float elapsedTime) override;
};

// ジャンプ2段目
class StateJumpAir : public PlayerStateBase
{
public:
	StateJumpAir(Player* player) : PlayerStateBase(player) {}

	// 遷移
	void Init() override;
	// 更新
	void Update(float elapsedTime) override;
};

// ジャンプ着地
class StateJumpEnd : public PlayerStateBase
{
public:
	StateJumpEnd(Player* player) : PlayerStateBase(player) {}

	// 遷移
	void Init() override;
	// 更新
	void Update(float elapsedTime) override;
};

// ダメージ
class StateDamage : public PlayerStateBase
{
public:
	StateDamage(Player* player) : PlayerStateBase(player) {}

	// 遷移
	void Init() override;
	// 更新
	void Update(float elapsedTime) override;
};

// 死亡
class StateDead : public PlayerStateBase
{
public:
	StateDead(Player* player) : PlayerStateBase(player) {}

	// 遷移
	void Init() override;
	// 更新
	void Update(float elapsedTime) override;
};

// 回復
class StateRecover : public PlayerStateBase
{
public:
	StateRecover(Player* player) : PlayerStateBase(player) {}

	// 遷移
	void Init() override;
	// 更新
	void Update(float elapsedTime) override;
};

// 攻撃ハンマー
class StateAttackHammer1 : public PlayerStateBase
{
public:
	StateAttackHammer1(Player* player) : PlayerStateBase(player) {}

	// 遷移
	void Init() override;
	// 更新
	void Update(float elapsedTime) override;
};

// 攻撃ハンマー
class StateAttackHammer2 : public PlayerStateBase
{
public:
	StateAttackHammer2(Player* player) : PlayerStateBase(player) {}

	// 遷移
	void Init() override;
	// 更新
	void Update(float elapsedTime) override;
};

// ジャンプ攻撃ハンマー
class StateAttackHammerJump : public PlayerStateBase
{
public:
	StateAttackHammerJump(Player* player) : PlayerStateBase(player) {}

	// 遷移
	void Init() override;
	// 更新
	void Update(float elapsedTime) override;
};

// 攻撃スピアー
class StateAttackSpear1 : public PlayerStateBase
{
public:
	StateAttackSpear1(Player* player) : PlayerStateBase(player) {}

	// 遷移
	void Init() override;
	// 更新
	void Update(float elapsedTime) override;
};

// 攻撃スピアー
class StateAttackSpear2 : public PlayerStateBase
{
public:
	StateAttackSpear2(Player* player) : PlayerStateBase(player) {}

	// 遷移
	void Init() override;
	// 更新
	void Update(float elapsedTime) override;
};

// 攻撃スピアー
class StateAttackSpear3 : public PlayerStateBase
{
public:
	StateAttackSpear3(Player* player) : PlayerStateBase(player) {}

	// 遷移
	void Init() override;
	// 更新
	void Update(float elapsedTime) override;
};

// ジャンプ攻撃スピアー
class StateAttackSpearJump : public PlayerStateBase
{
public:
	StateAttackSpearJump(Player* player) : PlayerStateBase(player) {}

	// 遷移
	void Init() override;
	// 更新
	void Update(float elapsedTime) override;
};

// 攻撃ソード
class StateAttackSword1 : public PlayerStateBase
{
public:
	StateAttackSword1(Player* player) : PlayerStateBase(player) {}

	// 遷移
	void Init() override;
	// 更新
	void Update(float elapsedTime) override;
};

// 攻撃ソード
class StateAttackSword2 : public PlayerStateBase
{
public:
	StateAttackSword2(Player* player) : PlayerStateBase(player) {}

	// 遷移
	void Init() override;
	// 更新
	void Update(float elapsedTime) override;
};

// 攻撃ソード
class StateAttackSword3 : public PlayerStateBase
{
public:
	StateAttackSword3(Player* player) : PlayerStateBase(player) {}

	// 遷移
	void Init() override;
	// 更新
	void Update(float elapsedTime) override;
};

// ジャンプ攻撃ソード
class StateAttackSwordJump : public PlayerStateBase
{
public:
	StateAttackSwordJump(Player* player) : PlayerStateBase(player) {}

	// 遷移
	void Init() override;
	// 更新
	void Update(float elapsedTime) override;
};
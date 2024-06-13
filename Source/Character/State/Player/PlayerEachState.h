#pragma once
#include "PlayerStateBase.h"

// �ҋ@
class StateIdle : public PlayerStateBase
{
public:
	StateIdle(Player* player) : PlayerStateBase(player) {}

	// �J��
	void Init() override;
	// �X�V
	void Update(float elapsedTime) override;
};

// �ҋ@->�ړ�
class StateIdleToRun : public PlayerStateBase
{
public:
	StateIdleToRun(Player* player) : PlayerStateBase(player) {}

	// �J��
	void Init() override;
	// �X�V
	void Update(float elapsedTime) override;
};

// �ړ�
class StateRun : public PlayerStateBase
{
public:
	StateRun(Player* player) : PlayerStateBase(player) {}

	// �J��
	void Init() override;
	// �X�V
	void Update(float elapsedTime) override;
};

// �ړ�->�ҋ@
class StateRunToIdle : public PlayerStateBase
{
public:
	StateRunToIdle(Player* player) : PlayerStateBase(player) {}

	// �J��
	void Init() override;
	// �X�V
	void Update(float elapsedTime) override;
};

// �W�����v�J�n
class StateJumpStart : public PlayerStateBase
{
public:
	StateJumpStart(Player* player) : PlayerStateBase(player) {}

	// �J��
	void Init() override;
	// �X�V
	void Update(float elapsedTime) override;
};

// �W�����v��
class StateJumpLoop : public PlayerStateBase
{
public:
	StateJumpLoop(Player* player) : PlayerStateBase(player) {}

	// �J��
	void Init() override;
	// �X�V
	void Update(float elapsedTime) override;
};

// �W�����v2�i��
class StateJumpAir : public PlayerStateBase
{
public:
	StateJumpAir(Player* player) : PlayerStateBase(player) {}

	// �J��
	void Init() override;
	// �X�V
	void Update(float elapsedTime) override;
};

// �W�����v���n
class StateJumpEnd : public PlayerStateBase
{
public:
	StateJumpEnd(Player* player) : PlayerStateBase(player) {}

	// �J��
	void Init() override;
	// �X�V
	void Update(float elapsedTime) override;
};

// �_���[�W
class StateDamage : public PlayerStateBase
{
public:
	StateDamage(Player* player) : PlayerStateBase(player) {}

	// �J��
	void Init() override;
	// �X�V
	void Update(float elapsedTime) override;
};

// ���S
class StateDead : public PlayerStateBase
{
public:
	StateDead(Player* player) : PlayerStateBase(player) {}

	// �J��
	void Init() override;
	// �X�V
	void Update(float elapsedTime) override;
};

// ��
class StateRecover : public PlayerStateBase
{
public:
	StateRecover(Player* player) : PlayerStateBase(player) {}

	// �J��
	void Init() override;
	// �X�V
	void Update(float elapsedTime) override;
};

// �U���n���}�[
class StateAttackHammer1 : public PlayerStateBase
{
public:
	StateAttackHammer1(Player* player) : PlayerStateBase(player) {}

	// �J��
	void Init() override;
	// �X�V
	void Update(float elapsedTime) override;
};

// �U���n���}�[
class StateAttackHammer2 : public PlayerStateBase
{
public:
	StateAttackHammer2(Player* player) : PlayerStateBase(player) {}

	// �J��
	void Init() override;
	// �X�V
	void Update(float elapsedTime) override;
};

// �W�����v�U���n���}�[
class StateAttackHammerJump : public PlayerStateBase
{
public:
	StateAttackHammerJump(Player* player) : PlayerStateBase(player) {}

	// �J��
	void Init() override;
	// �X�V
	void Update(float elapsedTime) override;
};

// �U���X�s�A�[
class StateAttackSpear1 : public PlayerStateBase
{
public:
	StateAttackSpear1(Player* player) : PlayerStateBase(player) {}

	// �J��
	void Init() override;
	// �X�V
	void Update(float elapsedTime) override;
};

// �U���X�s�A�[
class StateAttackSpear2 : public PlayerStateBase
{
public:
	StateAttackSpear2(Player* player) : PlayerStateBase(player) {}

	// �J��
	void Init() override;
	// �X�V
	void Update(float elapsedTime) override;
};

// �U���X�s�A�[
class StateAttackSpear3 : public PlayerStateBase
{
public:
	StateAttackSpear3(Player* player) : PlayerStateBase(player) {}

	// �J��
	void Init() override;
	// �X�V
	void Update(float elapsedTime) override;
};

// �W�����v�U���X�s�A�[
class StateAttackSpearJump : public PlayerStateBase
{
public:
	StateAttackSpearJump(Player* player) : PlayerStateBase(player) {}

	// �J��
	void Init() override;
	// �X�V
	void Update(float elapsedTime) override;
};

// �U���\�[�h
class StateAttackSword1 : public PlayerStateBase
{
public:
	StateAttackSword1(Player* player) : PlayerStateBase(player) {}

	// �J��
	void Init() override;
	// �X�V
	void Update(float elapsedTime) override;
};

// �U���\�[�h
class StateAttackSword2 : public PlayerStateBase
{
public:
	StateAttackSword2(Player* player) : PlayerStateBase(player) {}

	// �J��
	void Init() override;
	// �X�V
	void Update(float elapsedTime) override;
};

// �U���\�[�h
class StateAttackSword3 : public PlayerStateBase
{
public:
	StateAttackSword3(Player* player) : PlayerStateBase(player) {}

	// �J��
	void Init() override;
	// �X�V
	void Update(float elapsedTime) override;
};

// �W�����v�U���\�[�h
class StateAttackSwordJump : public PlayerStateBase
{
public:
	StateAttackSwordJump(Player* player) : PlayerStateBase(player) {}

	// �J��
	void Init() override;
	// �X�V
	void Update(float elapsedTime) override;
};
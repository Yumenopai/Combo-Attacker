#pragma once
#include "Player.h"

//�v���C���[
class PlayerAI : public Player
{
private:
	InputState nowInput = InputState::None;
	InputState oldInput, nextInput;

	// ����
	Enemy* avoidEnemy = nullptr;
	bool ranAwayFromEnemy = false;
	bool ranAwayFromPlayer1P = false;

	int waitTimer = 0;

public:
	PlayerAI();
	~PlayerAI() override;

	// �C���X�^���X�擾
	static PlayerAI& Instance();

	// �X�V
	void Update(float elapsedTime);
	// �U�����̍X�V����
	void AttackUpdate();

	// �{�^������(������)
	bool InputButtonDown(InputState button) override;
	// �{�^������(���͎�)
	bool InputButton(InputState button) override;
	// �{�^������(���㎞)
	bool InputButtonUp(InputState button) override;

	// ����ύX����
	void InputChangeArm(AttackType arm = AttackType::None) override;
	// �^�[�Q�b�g�񕜏���
	void InputRecover() override;

private:
	// �ړ��x�N�g��
	XMFLOAT3 GetMoveVec() const;
};
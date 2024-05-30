#pragma once
#include "Player.h"

//�v���C���[
class PlayerAI : public Player
{
private:
	// ���ݓ��͒�
	InputState nowInput = InputState::None;
	// �ЂƂO�̓���
	InputState oldInput;
	// �����͂���
	InputState nextInput;

	// ��������G(�P��)
	Enemy* currentAvoidEnemy = nullptr;
	// �G���瓦����
	bool ranAwayFromEnemy = false;
	// 1P���瓦����
	bool ranAwayFromPlayer1P = false;

	// �Ƃǂ߂��h�����߂̑ҋ@�^�C�}�[
	int waitTimer = 0;
	// ���O�̓G�ƂǂߑΏ�
	Enemy* lastAvoidEnemy = nullptr;

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
	void InputChangeWeapon(AttackType weapon = AttackType::None) override;
	// �^�[�Q�b�g�񕜏���
	void InputRecover() override;

private:
	// �ړ��x�N�g��
	XMFLOAT3 GetMoveVec() const;
};
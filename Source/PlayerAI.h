#pragma once
#include "Player.h"

//�v���C���[
class PlayerAI : public Player
{
private:
	InputState nowInput = InputState::None;
	InputState oldInput, nextInput;

public:
	PlayerAI();
	~PlayerAI() override;

	// �C���X�^���X�擾
	static PlayerAI& Instance();

	// �X�V
	void Update(float elapsedTime);

	// �`��
	void HPBarRender(const RenderContext& rc, Sprite* gauge);

	// �{�^������(������)
	bool InputButtonDown(InputState button) override;
	// �{�^������(���͎�)
	bool InputButton(InputState button) override;
	// �{�^������(���㎞)
	bool InputButtonUp(InputState button) override;

private:
	// �ړ��x�N�g��
	XMFLOAT3 GetMoveVec() const;
};
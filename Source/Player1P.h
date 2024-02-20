#pragma once
#include "Player.h"

//�v���C���[
class Player1P : public Player
{
private:

public:
	Player1P();
	~Player1P() override;

	//�C���X�^���X�擾
	static Player1P& Instance();

	//�X�V
	void Update(float elapsedTime);

	//�`��
	void HPBarRender(const RenderContext& rc, Sprite* gauge);

	// �{�^������
	bool InputButtonDown(InputState button) override;
	bool InputButton(InputState button) override;
	bool InputButtonUp(InputState button) override;

private:
	// �ړ��x�N�g��
	XMFLOAT3 GetMoveVec() const;
};
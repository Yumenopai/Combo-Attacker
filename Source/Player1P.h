#pragma once

#include <memory>
#include "Shader/Shader.h"
#include "Graphics/Model.h"
#include "Character.h"
#include "Enemy.h"
#include "Effect.h"
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
	void ShadowRender(const RenderContext& rc, ShadowMap* shadowMap);
	//�`��
	void Render(const RenderContext& rc, ModelShader* shader);
	void PrimitiveRender(const RenderContext& rc);
	void HPBarRender(const RenderContext& rc, Sprite* gauge);

	void UpdateJump(float elapsedTime);
	// ���͏���
	bool InputJumpButtonDown();
	bool InputJumpButton();
	bool InputJumpButtonUp();
	bool InputHammerButton();
	bool InputSwordButton();
	bool InputSpearButton();

private:
	// �X�e�B�b�N���͒l����ړ��x�N�g�����擾
	XMFLOAT3 GetMoveVec() const;
	// �ړ����͏���
	bool InputMove(float elapsedTime);
};
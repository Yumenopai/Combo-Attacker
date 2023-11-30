#pragma once

#include <memory>
#include "Shader/Shader.h"
#include "Graphics/Model.h"
#include "Character.h"
#include "Player.h"
#include "Enemy.h"
#include "Effect.h"


//�v���C���[
class PlayerAI : public Player
{
private:
	enum class InputState
	{
		None,
		Run,
		Jump,
		Hammer,
		Sword,
		Spear,
	};
	InputState nowInput = InputState::None;
	InputState oldInput, nextInput;

	XMFLOAT3 targetPosition = { 0,0,0 };

public:
	PlayerAI();
	~PlayerAI() override;

	//�C���X�^���X�擾
	static PlayerAI& Instance();

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

	// �[���{�^������
	bool InputButtonDown(InputState button)
	{
		if (oldInput == button) return false;
		if (nowInput == button) return true;
		return false;
	}
	bool InputButton(InputState button)
	{
		if (nowInput == button) return true;
		return false;
	}
	bool InputButtonUp(InputState button)
	{
		if (nowInput == button) return false;
		if (oldInput == button) return true;
		return false;
	}


protected:
	//���n�������ɌĂ΂��
	void OnLanding(float elapsedTime) override;

private:
	// �X�e�B�b�N���͒l����ړ��x�N�g�����擾
	XMFLOAT3 GetMoveVec() const;
	// �ړ����͏���
	bool InputMove(float elapsedTime);

	//�f�o�b�O
	void DebugMenu();
};
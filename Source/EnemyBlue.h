#pragma once

#include <memory>
#include "Graphics/Model.h"
#include "Enemy.h"

class EnemyBlue :public Enemy
{
public:
	EnemyBlue();
	~EnemyBlue() override;

	//�X�V����
	void Update(float elapsedTime) override;

	//�`�揈��
	void ShadowRender(const RenderContext& rc, ShadowMap* shadowMap) override;
	void Render(const RenderContext& rc, ModelShader* shader) override;

	//�꒣��ݒ�
	void SetTerritory(const DirectX::XMFLOAT3& origin, float range);

protected:
	//���S�������ɌĂ΂��
	void OnDead() override;

private:
	//�^�[�Q�b�g�ʒu�������_���ݒ�
	void SetRandomTargetPosition();

	//�ڕW�n�_�ֈړ�
	void MoveToTarget(float elapsedTime, float speedRate);

	//�v���C���[���G
	bool SearchPlayer();

	//�p�j�X�e�[�g�֑J��
	void TransitionWanderState();
	//�p�j�X�e�[�g�X�V����
	void UpdateWanderState(float elapsedTime);
	//�ҋ@�X�e�[�g�֑J��
	void TransitionIdleState();
	//�ҋ@�X�e�[�g�X�V����
	void UpdateIdleState(float elapsedTime);
	//�ǐՃX�e�[�g�֑J��
	void TransitionPursuitState();
	//�ǐՃX�e�[�g�X�V����
	void UpdatePursuitState(float elapsedTime);
	//�U���X�e�[�g�֑J��
	void TransitionAttackState();
	//�U���X�e�[�g�X�V����
	void UpdateAttackState(float elapsedTime);
	//�퓬�ҋ@�X�e�[�g�֑J��
	void TransitionIdleBattleState();
	//�퓬�ҋ@�X�e�[�g�X�V����
	void UpdateIdleBattleState(float elapsedTime);

private:
	//�X�e�[�g
	enum class State
	{
		Wander,
		Idle,
		Pursuit,
		Attack,
		IdleBattle
	};

	//�A�j���[�V����
	enum Animation
	{
		Anim_Idle1,
		Anim_FlyForward,
		Anim_Attack01,
		Anim_TakeOff,
		Anim_Land,
		Anim_Scream,
		Anim_AttackClaw,
		Anim_AttackFlame,
		Anim_Defend,
		Anim_GetDamage,
		Anim_Sleep,
		Anim_Walk,
		Anim_Run,
		Anim_FlyAttackFlame,
		Anim_FlyGlide,
		Anim_Idle02,
		Anim_Die,
		Anim_FlyFloat,
	};

private:
	std::unique_ptr<Model> model;
	State	state = State::Wander;
	DirectX::XMFLOAT3 targetPosition = { 0,0,0 };
	DirectX::XMFLOAT3 territoryOrigin = { 0,0,0 };
	float	territoryRange = 10.0f;
	float	moveSpeed = 3.0f;
	float	turnSpeed = DirectX::XMConvertToRadians(360);
	float	searchRange = 5.0f;
	float	attackRange = 1.5f;
};
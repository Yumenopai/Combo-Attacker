#pragma once

#include <memory>
#include "Graphics/Model.h"
#include "Enemy.h"

class EnemySlime :public Enemy
{
protected:
	//�A�j���[�V����
	enum Animation
	{
		Anim_IdleNormal,
		Anim_IdleBattle,
		Anim_Attack1,
		Anim_Attack2,
		Anim_WalkFWD,
		Anim_WalkBWD,
		Anim_WalkLeft,
		Anim_WalkRight,
		Anim_RunFWD,
		Anim_SenseSomthingST,
		Anim_SenseSomthingPRT,
		Anim_Taunt,
		Anim_Victory,
		Anim_GetHit,
		Anim_Dizzy,
		Anim_Die
	};
	//�X�e�[�g
	enum class State
	{
		Idle = Animation::Anim_IdleNormal,
		Wander = Animation::Anim_WalkFWD,
		Pursuit = Animation::Anim_RunFWD,
		Attack = Animation::Anim_Attack1,
		IdleBattle = Animation::Anim_IdleBattle,
	};

protected:
	std::unique_ptr<Model> model;
	State	state = State::Wander;
	DirectX::XMFLOAT3 territoryOrigin = { 0,0,0 };
	float	territoryRange = 10.0f;
	float	moveSpeed = 3.0f;
	float	turnSpeed = DirectX::XMConvertToRadians(360);
	float	searchRange = 5.0f;
	float	attackRange = 3.0f;

public:
	EnemySlime();
	~EnemySlime() override;

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

	//�^�[�Q�b�g�ʒu�������_���ݒ�
	void SetRandomTargetPosition();

	//�^�[�Q�b�g�ʒu��ݒ�
	void UpdateTargetPosition();

	//�ڕW�n�_�ֈړ�
	void MoveToTarget(float elapsedTime, float speedRate);
	
	//�ŋ�Player�ւ̉�]
	void TurnToTarget(float elapsedTime, float speedRate);

	//�v���C���[���G
	virtual bool SearchPlayer();

	//�m�[�h�ƃv���C���[�̏Փˏ���
	void CollisionNodeVsPlayer(const char* nodeName, float nodeRadius);

	//�p�j�X�e�[�g�X�V����
	void UpdateWanderState(float elapsedTime);
	//�ҋ@�X�e�[�g�X�V����
	void UpdateIdleState(float elapsedTime);
	//�ǐՃX�e�[�g�X�V����
	void UpdatePursuitState(float elapsedTime);
	//�U���X�e�[�g�X�V����
	void UpdateAttackState(float elapsedTime);
	//�퓬�ҋ@�X�e�[�g�X�V����
	void UpdateIdleBattleState(float elapsedTime);

	//�p�j�X�e�[�g�֑J��
	void TransitionState(State nowState);

	//�p�j�X�e�[�g�֑J��
	void TransitionWanderState();
	//�ҋ@�X�e�[�g�֑J��
	void TransitionIdleState();
	//�ǐՃX�e�[�g�֑J��
	void TransitionPursuitState();
	//�U���X�e�[�g�֑J��
	void TransitionAttackState();
	//�퓬�ҋ@�X�e�[�g�֑J��
	void TransitionIdleBattleState();
};
#pragma once

#include <memory>
#include "Graphics/Model.h"
#include "Enemy.h"

class EnemySlime :public Enemy
{
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

private:
	//�^�[�Q�b�g�ʒu�������_���ݒ�
	void SetRandomTargetPosition();

	//�ڕW�n�_�ֈړ�
	void MoveToTarget(float elapsedTime, float speedRate);

	//�v���C���[���G
	bool SearchPlayer();

	//�m�[�h�ƃv���C���[�̏Փˏ���
	void CollisionNodeVsPlayer(const char* nodeName, float nodeRadius);

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
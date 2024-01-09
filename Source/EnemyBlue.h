#pragma once

#include <memory>
#include "Graphics/Model.h"
#include "Enemy.h"
#include "Player.h"

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

	//�f�o�b�O
	void DebugMenu();

protected:
	//�_���[�W���ɌĂ΂��
	void OnDamaged() override;
	//���S�������ɌĂ΂��
	void OnDead() override;

private:
	//�^�[�Q�b�g�ʒu�������_���ݒ�
	void SetRandomTargetPosition();

	//�ڕW�n�_�ֈړ�
	void MoveToTarget(float elapsedTime, float speedRate);

	//�v���C���[���G
	bool SearchPlayer();

	//�ŋ߃v���C���[�̓o�^
	void UpdateTargetPosition();
	
	//�m�[�h�ƃv���C���[�̏Փˏ���
	void CollisionNodeVsPlayer(const char* nodeName, float nodeRadius);

	// �ŋ߃v���C���[�̃X�e�[�g�擾
	Player::EnemySearch GetNearestPlayerES();

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
	//�_���[�W�X�e�[�g�֑J��
	void TransitionGetHitState();
	//�_���[�W�X�e�[�g�X�V����
	void UpdateGetHitState(float elapsedTime);
	//���K�X�e�[�g�֑J��
	void TransitionScreamState();
	//���K�X�e�[�g�X�V����
	void UpdateScreamState(float elapsedTime);
	//���U���X�e�[�g�֑J��
	void TransitionAttackClawState();
	//���U���X�e�[�g�X�V����
	void UpdateAttackClawState(float elapsedTime);
	//���S�X�e�[�g�֑J��
	void TransitionDieState();
	//���S�X�e�[�g�X�V����
	void UpdateDieState(float elapsedTime);

private:
	//�X�e�[�g
	enum class State
	{
		Wander,
		Idle,
		Pursuit,
		Attack,
		IdleBattle,
		GetHit,
		Scream,
		AttackClaw,
		Die,
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

	bool isHalfHP = false;
	bool isDead = false;

	DirectX::XMFLOAT3 territoryOrigin = { 0,0,0 };
	float	territoryRange = 10.0f;
	float	moveSpeed = 3.0f;
	float	turnSpeed = DirectX::XMConvertToRadians(360);
	float	searchRange = 10.0f;
	float	attackRange = 5.0f;

	std::string nearestPlayerStr = "";
	float minLen = 0.0f;
	float nowLen = 0.0f;
};
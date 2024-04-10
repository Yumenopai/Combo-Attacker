#pragma once

#include <memory>
#include "Graphics/Model.h"
#include "Enemy.h"
#include "Player.h"

class EnemySlime :public Enemy
{
private:
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

public:
	//�X�e�[�g
	enum class State
	{
		Idle,
		Wander,
		Pursuit,
		Attack,
		IdleBattle,
		HitDamage
	};


protected:
	std::unique_ptr<Model> model;
	State	state = State::Wander;
	
	float effectOffset_Y = 0.8f;

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
	//Getter
	const float GetEffectOffset_Y() override { return effectOffset_Y; }

protected:
	//�_���[�W���ɌĂ΂��
	void OnDamaged() override;
	//���S�������ɌĂ΂��
	void OnDead() override;

	//�^�[�Q�b�g�ʒu�������_���ݒ�
	void SetRandomTargetPosition();

	//�^�[�Q�b�g�ʒu��ݒ�
	void UpdateTargetPosition();

	Player::EnemySearch GetNearestPlayerES();

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
	//�_���[�W�X�e�[�g�X�V����
	void UpdateHitDamageState(float elapsedTime);

	// �e�X�e�[�W���Ƃ̍X�V����
	virtual void UpdateEachState(float elapsedTime);

	//�X�e�[�g�J��
	void TransitionState(State nowState);
	virtual void TransitionPlayAnimation(State nowState);
};
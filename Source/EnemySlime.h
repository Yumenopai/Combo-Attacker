#pragma once

#include <memory>
#include "Graphics/Model.h"
#include "Enemy.h"
#include "Player.h"

class EnemySlime :public Enemy
{
private:
	//�A�j���[�V����
	enum class Animation
	{
		IdleNormal,
		IdleBattle,
		Attack1,
		Attack2,
		WalkFWD,
		WalkBWD,
		WalkLeft,
		WalkRight,
		RunFWD,
		SenseSomthingST,
		SenseSomthingPRT,
		Taunt,
		Victory,
		GetHit,
		Dizzy,
		Die
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
	// ���f��
	std::unique_ptr<Model> model;
	// �X�e�[�g
	State state = State::Wander;
	// �G�t�F�N�gY���I�t�Z�b�g
	float effectOffset_Y = 0.8f;

	// �꒣��Ώۈʒu
	DirectX::XMFLOAT3 territoryOrigin = { 0,0,0 };
	// �꒣��͈�
	float	territoryRange = 10.0f;
	// �ړ��X�s�[�h
	float	moveSpeed = 3.0f;
	// ��]�X�s�[�h
	float	turnSpeed = DirectX::XMConvertToRadians(360);
	// �T�m�͈�
	float	searchRange = 5.0f;
	// �U���͈�
	float	attackRange = 3.0f;

	// �ڋʃm�[�h�̖��O
	const char* eyeBallNodeName = "";

public:
	EnemySlime();
	~EnemySlime() override;

	// �X�V����
	void Update(float elapsedTime) override;

	// �V���h�E�}�b�v�`��
	void ShadowRender(const RenderContext& rc, ShadowMap* shadowMap) override;
	// �`��
	void Render(const RenderContext& rc, ModelShader* shader) override;

	// �꒣��ݒ�
	void SetTerritory(const DirectX::XMFLOAT3& origin, float range);
	// Getter
	const float GetEffectOffset_Y() override { return effectOffset_Y; }

protected:
	// �_���[�W���ɌĂ΂��
	void OnDamaged() override;
	// ���S�������ɌĂ΂��
	void OnDead() override;

	// �^�[�Q�b�g�ʒu�������_���ݒ�
	void SetRandomTargetPosition();

	// �^�[�Q�b�g�ʒu��ݒ�
	void UpdateTargetPosition();

	// Getter_�ŋ߃v���C���[�̓G�T���X�e�[�g
	Player::EnemySearch GetNearestPlayer_EnemySearch();

	// �ڕW�n�_�ֈړ�
	void MoveToTarget(float elapsedTime, float speedRate);
	
	// �ŋ�Player�ւ̉�]
	void TurnToTarget(float elapsedTime, float speedRate);

	// �v���C���[���G
	virtual bool SearchPlayer();

	// �m�[�h�ƃv���C���[�̏Փˏ���
	void CollisionNodeVsPlayer(const char* nodeName, float nodeRadius);

	// �p�j�X�e�[�g�X�V����
	void UpdateWanderState(float elapsedTime);
	// �ҋ@�X�e�[�g�X�V����
	void UpdateIdleState(float elapsedTime);
	// �ǐՃX�e�[�g�X�V����
	void UpdatePursuitState(float elapsedTime);
	// �U���X�e�[�g�X�V����
	void UpdateAttackState(float elapsedTime);
	// �퓬�ҋ@�X�e�[�g�X�V����
	void UpdateIdleBattleState(float elapsedTime);
	// �_���[�W�X�e�[�g�X�V����
	void UpdateHitDamageState(float elapsedTime);

	// �e�X�e�[�W���Ƃ̍X�V����
	virtual void UpdateEachState(float elapsedTime);

	// �X�e�[�g�J��
	void TransitionState(State nowState);
	// �A�j���[�V�����J��
	virtual void TransitionPlayAnimation(State nowState);
};
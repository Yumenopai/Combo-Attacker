#pragma once

#include <memory>
#include "Graphics/Model.h"
#include "Enemy.h"
#include "Player.h"

class EnemyBlue :public Enemy
{
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
	// �G�t�F�N�g�ԍ�
	enum class EffectNumber
	{
		dead,

		MaxCount
	};

private:
	// Effect
	Effect EffectArray[(int)EffectNumber::MaxCount] =
	{
		 "Data/Effect/cyanBroken.efk",
	};


public:
	EnemyBlue();
	~EnemyBlue() override;

	//�X�e�[�g�J��
	void TransitionState(State nowState);
	virtual void TransitionPlayAnimation(State nowState);
	//�X�V����
	void Update(float elapsedTime) override;

	//�`�揈��
	void ShadowRender(const RenderContext& rc, ShadowMap* shadowMap) override;
	void Render(const RenderContext& rc, ModelShader* shader) override;

	//�꒣��ݒ�
	void SetTerritory(const DirectX::XMFLOAT3& origin, float range);
	//Getter
	const float GetEffectOffset_Y() override { return effectOffset_Y; }

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
	void UpdateGetHitState(float elapsedTime);
	//���K�X�e�[�g�X�V����
	void UpdateScreamState(float elapsedTime);
	//���U���X�e�[�g�X�V����
	void UpdateAttackClawState(float elapsedTime);
	//���S�X�e�[�g�X�V����
	void UpdateDieState(float elapsedTime);

	// �G�t�F�N�g�Đ�
	void PlayEffect(EffectNumber num, const XMFLOAT3& position, float scale = 1.0f) {
		EffectArray[static_cast<int>(num)].Play(position, scale);
	}


private:
	//�A�j���[�V����
	enum class Animation
	{
		Idle1,
		FlyForward,
		Attack01,
		TakeOff,
		Land,
		Scream,
		AttackClaw,
		AttackFlame,
		Defend,
		GetDamage,
		Sleep,
		Walk,
		Run,
		FlyAttackFlame,
		FlyGlide,
		Idle02,
		Die,
		FlyFloat,
	};

private:
	std::unique_ptr<Model> model;
	State	state = State::Wander;

	float effectOffset_Y = 0.8f;

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
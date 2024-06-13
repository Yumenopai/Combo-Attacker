#pragma once

#include <memory>
#include "Constant/EnemyConst.h"
#include "Graphics/Model.h"
#include "Character/Core/Enemy.h"
#include "Character/Core/Player.h"

class EnemyBoss :public Enemy
{
private:
	// �G�t�F�N�g�ԍ�
	enum class EffectNumber
	{
		dead,

		MaxCount
	};

protected:
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
protected:
	// ���f��
	std::unique_ptr<Model> model;

private:
	// �X�e�[�g
	State state = State::Wander;

	// HP�������ȉ���
	bool isHalfHP = false;
	// ���S���肩
	bool isDead = false;
	// ���S����N���A�\���܂ł̃^�C�}�[
	int clearTimer = 0;

	// �꒣��Ώۈʒu
	DirectX::XMFLOAT3 territoryOrigin = { 0,0,0 };

	// ImGui�p/�ŋ߃v���C���[�̖��O�X�g�����O
	std::string nearestPlayerStr = "";
	// �v���C���[�̍ŋߋ���
	float minLen = 0.0f;
	// ���݂̃^�[�Q�b�g�̋���
	float nowLen = 0.0f;

	// Effect
	Effect EffectArray[(int)EffectNumber::MaxCount] =
	{
		 "Data/Effect/cyanBroken.efk",
	};

public:
	EnemyBoss();
	~EnemyBoss() override;

	//�X�e�[�g�J��
	void TransitionState(State nowState);
	//�X�V����
	void Update(float elapsedTime) override;

	//�`�揈��
	void ShadowRender(const RenderContext& rc, ShadowMap* shadowMap) override;
	void Render(const RenderContext& rc, ModelShader* shader) override;

	//Getter
	const float GetEffectOffset_Y() override { return dragon_effect_offset_y; }

	//�f�o�b�O
	void DebugMenu();

protected:
	// ��������
	void Init();

	// �A�j���[�V�����J��
	virtual void TransitionPlayAnimation(State nowState) = 0;

	//�_���[�W���ɌĂ΂��
	void OnDamaged() override;
	//���S�������ɌĂ΂��
	void OnDead() override;

	//���S�X�e�[�g�X�V����
	virtual void UpdateDieState(float elapsedTime);

private:
	//�^�[�Q�b�g�ʒu�������_���ݒ�
	void SetRandomTargetPosition();

	//�ڕW�n�_�ֈړ�
	void MoveToTarget(float elapsedTime, float speedRate = 1);

	//�v���C���[���G
	bool SearchPlayer();

	//�ŋ߃v���C���[�̓o�^
	void UpdateTargetPosition();
	
	//�m�[�h�ƃv���C���[�̏Փˏ���
	void CollisionNodeVsPlayer(const char* nodeName, float nodeRadius);

	// �ŋ߃v���C���[�̃X�e�[�g�擾
	Player::EnemySearch GetNearestPlayer_EnemySearch();

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

	// �G�t�F�N�g�Đ�
	void PlayEffect(EffectNumber num, const XMFLOAT3& position, float scale = 1.0f) {
		EffectArray[static_cast<int>(num)].Play(position, scale);
	}
};
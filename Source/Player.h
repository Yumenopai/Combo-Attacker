#pragma once

#include <memory>
#include "Shader/Shader.h"
#include "Graphics/Model.h"
#include "Character.h"
#include "State/Player/PlayerStateMachine.h"
#include "Enemy.h"
#include "Effect.h"
#include "Input/GamePad.h"

//�v���C���[
class Player : public Character
{
public:
	// �X�e�[�g
	enum class State
	{
		Idle,
		IdleToRun,
		Run,
		RunToIdle,
		JumpStart,
		JumpLoop,
		JumpAir,
		JumpEnd,
		Damage,
		Dead,
		Recover,
		AttackHammer1,
		AttackHammer2,
		AttackHammerJump,
		AttackSpear1,
		AttackSpear2,
		AttackSpear3,
		AttackSpearJump,
		AttackSword1,
		AttackSword2,
		AttackSword3,
		AttackSwordJump,
		CliffGrab,

		None,
	};

	// �A�j���[�V����
	enum class Animation
	{
		None,
		CliffGrab,
		Damage,
		Death,
		AttackHammer1,
		AttackHammer2,
		AttackHammerJump,
		Idle,
		IdleToRun,
		JumpAir,
		JumpEnd,
		JumpLoop,
		JumpStart,
		Running,
		RunToIdle,
		AttackSpear1,
		AttackSpear2,
		AttackSpear3,
		AttackSpearJump,
		AttackSword1,
		AttackSword2,
		AttackSword3,
		AttackSwordJump,
	};

	// �U���^�C�v
	enum class AttackType
	{
		None = -1,

		Hammer = 0,
		Spear,
		Sword,

		MaxCount
	};
	// �G�T�m�X�e�[�g
	enum class EnemySearch
	{
		None = -1,
		Find,
		Attack,

		Max,
	};
	// ����
	enum class InputState : unsigned int
	{
		None = 0,
		Run = 1,
		Jump = GamePad::BTN_A,
		Attack = GamePad::BTN_B,
		Change = GamePad::BTN_X,
		Other = GamePad::BTN_Y,
	};
	// �W�����v�X�e�[�g
	enum class JumpState
	{
		CanJump,
		CanDoubleJump,
		CannotJump
	};
	// �G�t�F�N�g�ԍ�
	enum class EffectNumber
	{
		Hit,
		BlueVortex,
		GreenVortex,
		RedVortex,
		Recovery,

		MaxCount
	};

	// ����
	struct Arms
	{
		const char*		nodeName;	// ����m�[�h�̖��O
		XMFLOAT3		position;	// �ʒu
		const XMVECTOR	rootOffset;	// ���{�̃I�t�Z�b�g
		const XMVECTOR	tipOffset;	// ��̃I�t�Z�b�g
		const float		radius;		// ���a
		const int		damage;		// �_���[�W��
		bool			flag1;		// �U���^�C�v1���U������
		bool			flag2;		// �U���^�C�v2���U������
		bool			flag3;		// �U���^�C�v3���U������
		bool			flagJump;	// �W�����v�U�����U������
	};

private:
	// �L�����N�^�[���f��
	std::unique_ptr<Model> model;
	// �X�e�[�g�}�V��
	PlayerStateMachine* stateMachine = nullptr;

	// �G���̋���
	std::unordered_map<Enemy*, float> enemyDist;
	// �G���̓G�T���X�e�[�g
	std::unordered_map<Enemy*, EnemySearch> enemySearch;

	/*
	��������̓o�^
		����I���Ń��X�g�擪�̕����I�сA�I���������Ԍ��ɒu���B
		����l���̍ۂ̓��X�g�擪�ɑ}�����A���̑I���ł��̕���ɂȂ�B
	*/
	std::list<AttackType> HaveArms;
	std::list<AttackType> HaventArms;
	// ��������
	const AttackType InitialArm = AttackType::Sword;
	// �g�p���̕���
	AttackType CurrentUseArm = AttackType::None;

	// ���ݍU�����Ă��邩
	bool isAttacking = false;
	// ���݂̘A���U����
	int attackCount = 0;
	// �U�����̓G�i���o�[
	int attackingEnemyNumber = -1;
	// �U������ ����̓G�̘A������
	bool isAttackJudge = true;

	// �W�����v�J�ڏ��
	JumpState jumpTrg = JumpState::CanJump;
	// �W�����v�U����������
	bool isMoveAttack = false;

	// SwordAttack
	static const int MAX_POLYGON = 32;
	XMFLOAT3 trailPositions[2][MAX_POLYGON];
	XMFLOAT4 color = { 1, 1, 1, 1 };
	bool isAddVertex = true;

	// Effect
	Effect EffectArray[(int)EffectNumber::MaxCount] = 
	{
		 "Data/Effect/hit.efk",
		 "Data/Effect/blueVortex.efk",
		 "Data/Effect/greenVortex.efk",
		 "Data/Effect/redVortex.efk",
		 "Data/Effect/recovery.efk"
	};

	// ***************** const *****************

	// �v���C���[�ƓG�̔��苗��
	static inline const float playerVSenemyJudgeDist[(int)EnemySearch::Max] =
	{
		6.5f,	// Find
		2.5f,	// Attack
	};

protected:
	// ���݂̓G�T���X�e�[�g
	EnemySearch currentEnemySearch = EnemySearch::None;
	// �ł��߂��G
	Enemy* nearestEnemy = nullptr;
	// �ł��߂��G�Ƃ̋���
	float nearestDist = FLT_MAX;
	// �ł��߂��G�Ƃ̃x�N�g��
	XMFLOAT3 nearestVec = {};

	// ***************** const *****************

	const float playerModelSize = 0.02f;
	const int playerMaxHealth = 100;

	const float moveSpeed = 8.0f;
	const float turnSpeed = XMConvertToRadians(1200);

protected:
	// �X�V
	void UpdateUtils(float elapsedTime);
	// �G�Ƃ̋����X�V
	void UpdateEnemyDistance(float elapsedTime);
	// �W�����v�X�e�[�g�X�V
	void UpdateJumpState(float elapsedTime);

	// �������͍X�V
	void UpdateVerticalVelocity(float elapsedFrame) override;

	// �v���C���[�ƃG�l�~�[�Ƃ̏Փˏ���
	void CollisionPlayerVsEnemies();

	// �_���[�W�󂯂����ɌĂ΂��
	void OnDamaged() override;
	// ���S�������ɌĂ΂��
	void OnDead() override;

	// �O�Քz��Y����
	void ShiftTrailPositions();
	// �O�Օ`��
	void RenderTrail();

	// HP�o�[�`��
	void HPBarRender(const RenderContext& rc, Sprite* gauge, bool is1P);

	// �f�o�b�O�v���~�e�B�u�`��
	void DrawDebugPrimitive();
	// �f�o�b�O
	void DebugMenu();

	// �ړ��x�N�g��
	virtual XMFLOAT3 GetMoveVec() const = 0;

public:
	Player();
	~Player() override;

	// ������
	void Init();

	// �X�V
	virtual void Update(float elapsedTime) = 0;

	// ���n�������ɌĂ΂��
	void OnLanding(float elapsedTime) override;

	// �ߋ����U�����̊p�x����
	void ForceTurnByAttack(float elapsedTime);
	// ���퓖���蔻��ʒu�̍X�V
	void UpdateArmPositions(Model* model, Arms& arm);
	// �U�����̐��������x�X�V
	void HorizontalVelocityByAttack(bool plus, int velo, float elapsedTime);

	// ����ƃG�l�~�[�̏Փˏ���
	void CollisionArmsVsEnemies(Arms arm);

	// �񕜑J�ڊm�F����
	bool IsRecoverTransition();

	// �V���h�E�}�b�v�p�`��
	void ShadowRender(const RenderContext& rc, ShadowMap* shadowMap);
	// �`��
	void Render(const RenderContext& rc, ModelShader* shader);
	// �U���̋O�Օ`��
	void PrimitiveRender(const RenderContext& rc);
	// HP�o�[�`��
	virtual void HPBarRender(const RenderContext& rc, Sprite* gauge) = 0;

	// �ړ����͏���
	bool InputMove(float elapsedTime);
	// �U�����͏���
	bool InputAttackFromNoneAttack();
	// �W�����v���̍U�����͏���
	bool InputAttackFromJump(float elapsedTime);
	// ����ύX����
	void InputChangeArm(AttackType arm = AttackType::None);

	// �{�^������(������)
	virtual bool InputButtonDown(InputState button) = 0;
	// �{�^������(���͎�)
	virtual bool InputButton(InputState button) = 0;
	// �{�^������(���㎞)
	virtual bool InputButtonUp(InputState button) = 0;

	// �ȗ����֐�
	void ChangeState(State newState) {
		stateMachine->ChangeState(static_cast<int>(newState));
	}
	void PlayAnimation(Animation anim, bool loop) {
		model->PlayAnimation(static_cast<int>(anim), loop);
	}
	void PlayEffect(EffectNumber num, const XMFLOAT3& position, float scale = 1.0f) {
		EffectArray[static_cast<int>(num)].Play(position, scale);
	}

	// ***************** GETTER & SETTER *****************

	PlayerStateMachine* GetStateMachine() const { return stateMachine; }

	Model* GetModel() const { return model.get(); }

	Enemy* GetNearestEnemy() { return nearestEnemy; }
	EnemySearch GetEachEnemySearch(Enemy* enemy) { return enemySearch[enemy]; }
	float GetEachEnemyDist(Enemy* enemy) { return enemyDist[enemy]; }

	int GetAttackCount() const { return attackCount; }
	void SetAttackCount(int count) { attackCount = count; }
	void AddAttackCount() { attackCount++; }

	bool GetAttacking() const { return isAttacking; }
	void SetAttacking(bool _isAttacking) { isAttacking = _isAttacking; }

	EnemySearch GetEnemySearch() const { return currentEnemySearch; }

	bool GetAttackJudge() const { return isAttackJudge; }
	void SetAttackJadge(bool aj) { isAttackJudge = aj; }

	bool GetMoveAttack() const { return isMoveAttack; }
	void SetMoveAttack(bool ma) { isMoveAttack = ma; }
	
	XMFLOAT3 GetVelocity() const { return velocity; }
	void SetVelocity(float x = FLT_MAX, float y = FLT_MAX, float z = FLT_MAX)
	{
		if(x != FLT_MAX) velocity.x = x;
		if(y != FLT_MAX) velocity.y = y;
		if(z != FLT_MAX) velocity.z = z;
	}
	void AddVelocity(XMFLOAT3 velo)
	{
		velocity.x += velo.x;
		velocity.y += velo.y;
		velocity.z += velo.z;
	}

	Arms GetHammer() const { return Hammer; }
	Arms GetSpear() const { return Spear; }
	Arms GetSword() const { return Sword; }

	const std::list<AttackType> GetHaveArm() { return HaveArms; }
	void AddHaveArm(AttackType arm) { HaveArms.push_front(arm); }
	const std::list<AttackType> GetHaventArm() { return HaventArms; }
	// list<AttackType>����w�肳�ꂽ������폜����
	void ListEraseArm(std::list<AttackType> list, AttackType arm)
	{
		for (auto it = list.begin(); it != list.end();) {
			if (*it == arm) {
				it = list.erase(it);
				break;
			}
		}
	}


protected:
	Arms Hammer
	{
		"Hammer",
		{0,0,0},
		{0,0,0},
		{0,35,0},
		0.5f,
		3,
		false,
		false,
		false,
		false,
	};
	Arms Spear
	{
		"Spear",
		{0,0,0},
		{0,0,0},
		{0,90,0},
		0.4f,
		1,
		false,
		false,
		false,
		false,
	};
	Arms Sword
	{
		"Sword",
		{0,0,0},
		{0,0,0},
		{0,30,0},
		0.4f,
		1,
		false,
		false,
		false,
		false,
	};
};
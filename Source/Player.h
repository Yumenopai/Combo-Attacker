#pragma once

#include <memory>
#include "Shader/Shader.h"
#include "Graphics/Model.h"
#include "Graphics/Graphics.h"
#include "Graphics/FontSprite.h"
#include "Character.h"
#include "State/Player/PlayerStateMachine.h"
#include "Effect.h"
#include "Input/GamePad.h"

#define SC_INT static_cast<int>
#define SC_AT static_cast<Player::AttackType>

class Enemy;

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

		Sword = 0,
		Hammer,
		Spear,

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
		Player = GamePad::BTN_X,
		Buddy = GamePad::BTN_Y,
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

	/// <summary>
	/// ��������̓o�^
	///		first:�����ł��镐��
	///		second : �������Ă��邩
	/// </summary>
	std::unordered_map<AttackType, bool> HaveArms;

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

	const float playerModelSize = 0.02f;
	const int playerMaxHealth = 100;

	const float moveSpeed = 8.0f;
	const float turnSpeed = XMConvertToRadians(1200);
	
	// UI
	const float hpGuageWidth = 700.0f;
	const float hpGuageHeight = 15.0f;
	const XMFLOAT4 hpGuageBack = { 0.3f, 0.3f, 0.3f, 0.8f };

protected:
	// ��������
	AttackType InitialArm;
	// �g�p���̕���
	AttackType CurrentUseArm = AttackType::None;

	// ���݂̓G�T���X�e�[�g
	EnemySearch currentEnemySearch = EnemySearch::None;
	// �ł��߂��G
	Enemy* nearestEnemy = nullptr;
	// �ł��߂��G�Ƃ̋���
	float nearestDist = FLT_MAX;
	// �ł��߂��G�Ƃ̃x�N�g��
	XMFLOAT3 nearestVec = {};

	// �񕜑J�ډ\��
	bool enableRecoverTransition = false;

	// ***************** const *****************
	
	Player* targetPlayer;
	std::string characterName;
	XMFLOAT4 nameColor;
	// UI //1P & AI
	float hpGuage_Y;
	XMFLOAT4 hpColorNormal;
	XMFLOAT4 hpColorWorning;

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

	// �񕜑J�ڊm�F����
	bool EnableRecoverTransition();

	// �_���[�W�󂯂����ɌĂ΂��
	void OnDamaged() override;
	// ���S�������ɌĂ΂��
	void OnDead() override;

	// �O�Քz��Y����
	void ShiftTrailPositions();
	// �O�Օ`��
	void RenderTrail();

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

	// ���̑I�𕐊�擾
	Player::AttackType GetNextArm();

	// �ߋ����U�����̊p�x����
	void ForceTurnByAttack(float elapsedTime);
	// ���퓖���蔻��ʒu�̍X�V
	void UpdateArmPositions(Model* model, Arms& arm);
	// �U�����̐��������x�X�V
	void HorizontalVelocityByAttack(bool plus, int velo, float elapsedTime);

	// ����ƃG�l�~�[�̏Փˏ���
	void CollisionArmsVsEnemies(Arms arm);

	// �V���h�E�}�b�v�p�`��
	void ShadowRender(const RenderContext& rc, ShadowMap* shadowMap);
	// �`��
	void Render(const RenderContext& rc, ModelShader* shader);
	// �U���̋O�Օ`��
	void PrimitiveRender(const RenderContext& rc);
	// HP�o�[�`��
	void RenderHPBar(ID3D11DeviceContext* dc, Sprite* gauge, FontSprite* font);
	//�L�����N�^�[���O�`��
	void RenderCharacterName(const RenderContext& rc, FontSprite* font);
	// ��������`��
	void RenderHaveArms(ID3D11DeviceContext* dc, Sprite* frame, Sprite* ArmSprite);

	// �ړ����͏���
	bool InputMove(float elapsedTime);
	// �U�����͏���
	bool InputAttackFromNoneAttack();
	// �W�����v���̍U�����͏���
	bool InputAttackFromJump(float elapsedTime);
	// ����ύX����
	virtual void InputChangeArm(AttackType arm = AttackType::None) = 0;
	// �^�[�Q�b�g�񕜏���
	virtual void InputRecover() = 0;
	// ������g�p�\�ɂ���
	void AddHaveArm(AttackType arm = AttackType::None);

	// �{�^������(������)
	virtual bool InputButtonDown(InputState button) = 0;
	// �{�^������(���͎�)
	virtual bool InputButton(InputState button) = 0;
	// �{�^������(���㎞)
	virtual bool InputButtonUp(InputState button) = 0;

	// �ȗ����֐�
	void ChangeState(State newState) {
		stateMachine->ChangeState(SC_INT(newState));
	}
	void PlayAnimation(Animation anim, bool loop) {
		model->PlayAnimation(SC_INT(anim), loop);
	}
	void PlayEffect(EffectNumber num, const XMFLOAT3& position, float scale = 1.0f) {
		EffectArray[SC_INT(num)].Play(position, scale);
	}

#pragma region GETTER & SETTER
	// ***************** GETTER & SETTER *****************

	PlayerStateMachine* GetStateMachine() const { return stateMachine; }

	Model* GetModel() const { return model.get(); }

	Player* GetTargetPlayer() const { return targetPlayer; }

	Enemy* GetNearestEnemy() { return nearestEnemy; }
	EnemySearch GetEachEnemySearch(Enemy* enemy) { return enemySearch[enemy]; }
	float GetEachEnemyDist(Enemy* enemy) { return enemyDist[enemy]; }

	int GetAttackCount() const { return attackCount; }
	void SetAttackCount(int count) { attackCount = count; }
	void AddAttackCount() { attackCount++; }

	bool GetAttacking() const { return isAttacking; }
	void SetAttacking(bool _isAttacking) { isAttacking = _isAttacking; }

	bool GetEnableRecoverTransition() const { return enableRecoverTransition; }

	AttackType GetCurrentUseArm() const { return CurrentUseArm; }
	bool GetHpWorning() const { return GetHealthRate() <= 20; }

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
#pragma endregion

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
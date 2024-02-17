#pragma once

#include <memory>
#include "Shader/Shader.h"
#include "Graphics/Model.h"
#include "Character.h"
#include "State/Player/PlayerStateMachine.h"
#include "Enemy.h"
#include "Effect.h"


//�v���C���[
class Player : public Character
{
public:
	//�X�e�[�g
	enum class AttackType
	{
		Hammer,
		Spear,
		Sword,

		None,
	};
	//�X�e�[�g
	enum class EnemySearch
	{
		None = -1,
		Find,
		Attack,

		Max,
	};

	struct Arms
	{
		const char* nodeName;
		XMFLOAT3	position;
		const XMVECTOR	rootOffset;
		const XMVECTOR	tipOffset;
		float		radius;
		bool flag1;
		bool flag2;
		bool flag3;
		bool flagJump;
	};

protected:
	std::unique_ptr<Model> model;
	AttackType	Atype = AttackType::None;
	EnemySearch ESState = EnemySearch::None;

	PlayerStateMachine* stateMachine = nullptr;

	// VS-Enemy-Search
	std::map<Enemy*, EnemySearch> enemySearch;
	std::map<Enemy*, float> enemyDist;

	Enemy* nearestEnemy = nullptr;
	float nearestDist = FLT_MAX;
	XMFLOAT3 nearestVec = {};

	bool isAttackjudge = true;
	int attackingEnemyNumber = -1;

	//�W�����v�U�����Ɏg�p
	bool isMoveAttack = false;
	int attackCount = 0;
	const float playerVSenemyJudgeDist[(int)EnemySearch::Max] = { 6.5f, 2.5f, };

	// SwordAttack
	static const int MAX_POLYGON = 32;
	XMFLOAT3 trailPositions[2][MAX_POLYGON];
	XMFLOAT4 color = { 1, 1, 1, 1 };
	bool isAddVertex = true;

	float moveSpeed = 5.0f;
	float turnSpeed = XMConvertToRadians(720);

	bool isDamaged = false;
	bool isDead = false;
	float stateTimer = 0.0f;
	float DamageFlash = 0.1f;
	int flashCount = 0;

	float jumpSpeed = 17.5f;

	DirectX::XMFLOAT3 offset;

	std::unique_ptr<Effect> hitEffect;

public:
	Player();
	~Player() override;

	// ������
	void Init();

	//�X�V
	virtual void Update(float elapsedTime) = 0;
	virtual void UpdateJump(float elapsedTime) = 0;

	//�`��
	virtual void ShadowRender(const RenderContext& rc, ShadowMap* shadowMap) = 0;
	//�`��
	virtual void Render(const RenderContext& rc, ModelShader* shader) = 0;
	void PrimitiveRender(const RenderContext& rc);
	virtual void HPBarRender(const RenderContext& rc, Sprite* gauge) = 0;

	// �X�e�[�g�}�V���擾 
	PlayerStateMachine* GetStateMachine() const { return stateMachine; }

	int GetAttackCount() const { return attackCount; }
	void SetAttackCount(int count) { attackCount = count; }
	void AddAttackCount() { attackCount++; }

	AttackType GetAttackType() const { return Atype; }
	void SetAttackType(AttackType at) { Atype = at; }

	EnemySearch GetESState() const { return ESState; }

	bool GetAttackJudge() const { return isAttackjudge; }
	void SetAttackJadge(bool aj) { isAttackjudge = aj; }

	bool GetMoveAttack() const { return isMoveAttack; }
	void SetMoveAttack(bool ma) { isMoveAttack = ma; }
	
	XMFLOAT3 GetVelocity() { return velocity; }
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

	Arms GetHammer() { return Hammer; }
	Arms GetSpear() { return Spear; }
	Arms GetSword() { return Sword; }

	// �U������
	bool InputAttackFromNoneAttack(float elapsedTime);
	bool InputAttackFromJump(float elapsedTime);

	//�񕜑J�ڊm�F����
	bool IsRecoverTransition();
	
	// ���͏���
	virtual bool InputJumpButtonDown() = 0;
	virtual bool InputJumpButton() = 0;
	virtual bool InputJumpButtonUp() = 0;
	virtual bool InputHammerButton() = 0;
	virtual bool InputSwordButton() = 0;
	virtual bool InputSpearButton() = 0;

	//�f�o�b�O�v���~�e�B�u�`��
	void DrawDebugPrimitive();


	Model* GetModel() const { return model.get(); }
	
	Enemy* GetNearestEnemy() { return nearestEnemy; }
	EnemySearch GetEachEnemySearchState(Enemy* enemy) { return enemySearch[enemy]; }
	float GetEachEnemyDist(Enemy* enemy) { return enemyDist[enemy]; }

	// ���퓖���蔻��ʒu�̍X�V
	void UpdateArmPositions(Model* model, Arms& arm);
	void HorizontalVelocityByAttack(bool plus, int velo, float elapsedTime);

	// �v���C���[�ƃG�l�~�[�Ƃ̏Փˏ���
	void CollisionPlayerVsEnemies();
	// ����ƂƃG�l�~�[�̏Փˏ���
	void CollisionArmsVsEnemies(Arms arm);

	// �ړ����͏���
	virtual bool InputMove(float elapsedTime);

	//�f�o�b�O
	void DebugMenu();

	//���n�������ɌĂ΂��
	void OnLanding(float elapsedTime) override;

	//���S�������ɌĂ΂��
	void OnDead();

protected:
	//�_���[�W�󂯂����ɌĂ΂��
	void OnDamaged();

	//�������͍X�V �I�[�o�[���C�h
	void UpdateVerticalVelocity(float elapsedFrame);

	//�O��
	void ShiftTrailPositions();
	void RenderTrail();

protected:
	enum class JumpState
	{
		CanJump,
		CanDoubleJump,
		CannotJump
	};
	JumpState jumpTrg = JumpState::CanJump;

	Arms Hammer
	{
		"Hammer",
		{0,0,0},
		{0,0,0},
		{0,35,0},
		0.5f,
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
		false,
		false,
		false,
		false,
	};
public:
	//�X�e�[�g
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

	//�A�j���[�V����
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
};
#pragma once

#include <memory>
#include <iostream>
#include "UtilsDefineConst.h"
#include "PlayerConst.h"
#include "Shader/Shader.h"
#include "Graphics/Model.h"
#include "Graphics/Graphics.h"
#include "Graphics/FontSprite.h"
#include "Character.h"
#include "State/Player/PlayerStateMachine.h"
#include "Effect.h"
#include "Input/GamePad.h"

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
		None = -1,	// �Ȃ�
		Find = 0,	// ����
		Attack,		// �U��

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
		CanJump,		// �W�����v�\
		CanDoubleJump,	// ��i�K�ڃW�����v�\
		CannotJump		// �W�����v�s�\
	};
	// �G�t�F�N�g�ԍ�
	enum class EffectNumber
	{
		Hit,		// �q�b�g
		BlueVortex,	// �Q
		GreenVortex,// �ΉQ
		RedVortex,	// �ԉQ
		Recovery,	// ��

		MaxCount
	};
	// �ʒm���b�Z�[�W
	enum class PlayerMessage
	{
		None = -1,		// �Ȃ�
		WeaponGet = 0,	// ����l��
		LevelUp,		// ���x���A�b�v
		Attack,			// �Ƃǂ߂���
		RanAway,		// ������
		Indifference,	// �����m��Ȃ�

		MaxCount
	};

	// ����
	struct Weapon
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

protected:
	Weapon Hammer
	{
		"Hammer",
		WEAPON_initial_position,
		WEAPON_initial_root_offset,
		WEAPON_hammer_initial_tip_offset,
		WEAPON_hammer_radius,
		WEAPON_hammer_damage,
		false,
		false,
		false,
		false,
	};
	Weapon Spear
	{
		"Spear",
		WEAPON_initial_position,
		WEAPON_initial_root_offset,
		WEAPON_spear_initial_tip_offset,
		WEAPON_spear_radius,
		WEAPON_spear_damage,
		false,
		false,
		false,
		false,
	};
	Weapon Sword
	{
		"Sword",
		WEAPON_initial_position,
		WEAPON_initial_root_offset,
		WEAPON_sword_initial_tip_offset,
		WEAPON_sword_radius,
		WEAPON_sword_damage,
		false,
		false,
		false,
		false,
	};
private:
	// �L�����N�^�[���f��
	std::unique_ptr<Model> model;
	// �X�e�[�g�}�V��
	PlayerStateMachine* stateMachine = nullptr;
	// ���݂̃v���C���[���x��
	int currentLevel = initial_level;
	/// <summary>
	/// ��������̓o�^
	///		first:�����ł��镐��
	///		second : �������Ă��邩
	/// </summary>
	std::unordered_map<AttackType, bool> HaveWeapons;
	// �W�����v�J�ڏ��
	JumpState jumpTrg = JumpState::CanJump;
	// �W�����v�U����������
	bool isMoveAttack = false;

	/****** ATTACK ******/
	// ���ݍU�����Ă��邩
	bool isAttacking = false;
	// ���݂̘A���U����
	int attackCount = 0;
	// �U�����̓G�i���o�[
	int attackingEnemyNumber = none_attacking;
	// �U������ ����̓G�̘A������
	bool isAttackJudge = true;
	// �X�y�V�����Z
	bool enableSpecialAttack = false;

	/****** VS ENEMY ******/
	// �G���̋���
	std::unordered_map<Enemy*, float> enemyDist;
	// �G���̓G�T���X�e�[�g
	std::unordered_map<Enemy*, EnemySearch> enemySearch;

	/****** UI ******/
	// ���b�Z�[�W
	PlayerMessage messageNumber = PlayerMessage::None;
	// ���b�Z�[�W�\���p�^�C�}�[
	float messageYTimer = 0.0f;
	// ���b�Z�[�W
	bool enableShowMessage[SC_INT(PlayerMessage::MaxCount)] = {};

	// Effect
	Effect EffectArray[SC_INT(EffectNumber::MaxCount)] =
	{
		 "Data/Effect/hit.efk",
		 "Data/Effect/blueVortex.efk",
		 "Data/Effect/greenVortex.efk",
		 "Data/Effect/redVortex.efk",
		 "Data/Effect/recovery.efk"
	};

protected:
	// �^�������_���[�W
	int allDamage = 0;

	// �g�p���̕���
	AttackType CurrentUseWeapon = AttackType::None;
	// ���݂̓G�T���X�e�[�g
	EnemySearch currentEnemySearch = EnemySearch::None;
	// �U�����̓G
	Enemy* currentAttackEnemy;

	// �ł��߂��G�Ƃ̃x�N�g��
	XMFLOAT3 nearestVec = {};
	// �ł��߂��G
	Enemy* nearestEnemy = nullptr;
	// �ł��߂��G�Ƃ̋���
	float nearestDist = FLT_MAX;

	// ��Ԗڂɋ߂��G�Ƃ̋���
	float secondDist = FLT_MAX;
	// ��Ԗڂɋ߂��G�Ƃ̃x�N�g��
	XMFLOAT3 secondDistEnemyVec = {};

	// �񕜑J�ډ\��
	bool enableRecoverTransition = false;

	// ***************** �h���N���XInit�œo�^ *****************
	// �v���C���[�̖��O
	std::string playerName;
	// �v���C���[�̃V���A���i���o�[
	int serialNumber;
	// �Ώۂ̃v���C���[//TODO:�����𑝂₷�ۂɗv���C
	Player* targetPlayer;
	// ���O�̕\���F
	XMFLOAT4 nameColor;

	// ��]�X�s�[�h
	float turnSpeed;
	// ��������
	AttackType InitialWeapon;

	// HP�Q�[�W/Y���W
	float hpGaugePosition_Y;
	
private:
	// �L�����N�^�[���b�Z�[�W�`��
	void RenderCharacterMessage(ID3D11DeviceContext* dc, Sprite* message, DirectX::XMFLOAT2 position);

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
	void RenderTrail() const;

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
	Player::AttackType GetNextWeapon();

	// �ߋ����U�����̊p�x����
	void ForceTurnByAttack(float elapsedTime);
	// ���퓖���蔻��ʒu�̍X�V
	void UpdateWeaponPositions(Model* model, Weapon& weapon);
	// �U�����̐��������x�X�V
	void HorizontalVelocityByAttack(bool plus, int velo, float elapsedTime);

	// ����ƃG�l�~�[�̏Փˏ���
	void CollisionWeaponsVsEnemies(Weapon weapon);

	// �V���h�E�}�b�v�p�`��
	void ShadowRender(const RenderContext& rc, ShadowMap* shadowMap);
	// �`��
	void Render(const RenderContext& rc, ModelShader* shader);
	// HP�o�[�`��
	void RenderHPBar(ID3D11DeviceContext* dc, Sprite* gauge, FontSprite* font);
	// �L�����N�^�[���O�`��
	void RenderCharacterOverHead(const RenderContext& rc, FontSprite* font, Sprite* message);
	// ��������`��
	void RenderHaveWeapons(ID3D11DeviceContext* dc, Sprite* frame, Sprite* weaponSprite);
	// �����`��/�v���C���[UI�p
	void StringRender(ID3D11DeviceContext* dc, FontSprite* font, 
		std::string str,			// �e�L�X�g
		DirectX::XMFLOAT2 position,	// �ʒu
		DirectX::XMFLOAT4 color);	// �F

	// �ړ����͏���
	bool InputMove(float elapsedTime);
	// �U�����͏���
	bool InputAttackFromNoneAttack();
	// �W�����v���̍U�����͏���
	bool InputAttackFromJump();
	// ����ύX����
	virtual void InputChangeWeapon(AttackType weapon = AttackType::None) = 0;
	// �^�[�Q�b�g�񕜏���
	virtual void InputRecover() = 0;
	// ������g�p�\�ɂ���
	void AddHaveWeapon(AttackType weapon = AttackType::None);

	// �{�^������(������)
	virtual bool InputButtonDown(InputState button) = 0;
	// �{�^������(���͎�)
	virtual bool InputButton(InputState button) = 0;
	// �{�^������(���㎞)
	virtual bool InputButtonUp(InputState button) = 0;

	// �ȗ����֐�
	// �X�e�[�g�ύX
	void ChangeState(State newState) {
		stateMachine->ChangeState(SC_INT(newState));
	}
	// �A�j���[�V�����Đ�
	void PlayAnimation(Animation anim, bool loop) {
		model->PlayAnimation(SC_INT(anim), loop);
	}
	// �G�t�F�N�g�Đ�
	void PlayEffect(EffectNumber num, const XMFLOAT3& position, float scale = 1.0f) {
		EffectArray[SC_INT(num)].Play(position, scale);
	}

	// �f�o�b�O�v���~�e�B�u�`��
	void DrawDebugPrimitive() const;
	// �f�o�b�O
	void DebugMenu();

#pragma region GETTER & SETTER
	// ***************** GETTER & SETTER *****************

	PlayerStateMachine* GetStateMachine() const { return stateMachine; }
	Model* GetModel() const { return model.get(); }

	int GetSerialNumber() const { return serialNumber; }
	Player* GetTargetPlayer() const { return targetPlayer; }
	Enemy* GetCurrentAttackEnemy() const { return currentAttackEnemy; }
	void SetCurrentAttackEnemy(Enemy* enemy) { currentAttackEnemy = enemy; }

	Enemy* GetNearestEnemy() { return nearestEnemy; }
	EnemySearch GetEachEnemySearch(Enemy* enemy) { return enemySearch[enemy]; }
	float GetEachEnemyDist(Enemy* enemy) { return enemyDist[enemy]; }

	int GetAttackCount() const { return attackCount; }
	void SetAttackCount(int count) { attackCount = count; }
	void AddAttackCount() { attackCount++; }
	
	int GetAllDamage() const { return allDamage; }

	bool GetAttacking() const { return isAttacking; }
	void SetAttacking(bool _isAttacking) { isAttacking = _isAttacking; }

	int GetLevel() const { return currentLevel; }
	void SetLevel(int lv) { currentLevel = lv; }
	void AddLevel(int lv) { currentLevel += lv; SetEnableShowMessage(Player::PlayerMessage::LevelUp, true); }

	bool GetHaveEachWeapon(AttackType weapon) { return HaveWeapons[weapon]; }
	int GetHaveWeaponCount() {
		int haveCount = 0;
		for (int i = 0; i < HaveWeapons.size(); i++)
		{
			if (HaveWeapons[SC_AT(i)]) haveCount++;
		}
		return haveCount;
	}

	void SetEnableShowMessage(PlayerMessage number, bool isShow) { enableShowMessage[SC_INT(number)] = isShow; }

	bool GetEnableRecoverTransition() const { return enableRecoverTransition; }

	AttackType GetCurrentUseWeapon() const { return CurrentUseWeapon; }
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

	Weapon GetHammer() const { return Hammer; }
	Weapon GetSpear() const { return Spear; }
	Weapon GetSword() const { return Sword; }
#pragma endregion
};
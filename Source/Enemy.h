#pragma once

#include "Shader/Shader.h"
#include "Character.h"

#define PL1P 0
#define PLAI 1

class Player;

class Enemy :public Character
{
public:
	Enemy(){}
	~Enemy() override{}

	//�X�V����
	virtual void Update(float elapsedTime) = 0;

	//�`�揈��
	virtual void ShadowRender(const RenderContext& rc, ShadowMap* shadowMap) = 0;
	virtual void Render(const RenderContext& rc, ModelShader* shader) = 0;

	//�j��
	void Destroy();

	//�f�o�b�O�v���~�e�B�u�`��
	virtual void DrawDebugPrimitive();

	//�_���[�W���Z
	bool ApplyDamage(int damage, float invincibleTime, Player* attacker, int playerNo);

	//Setter
	void SetTargetPosition(DirectX::XMFLOAT3& pos) { targetPosition = pos; }
	void SetFirstAttacker(Player* player) { FirstAttacker = player; }
	void SetLastAttacker(Player* player) { LastAttacker = player; }
	void AddAttackedDamage(int playerNo, int addDamage) { attackedDamage[playerNo] += addDamage; }

	//Getter
	virtual const float GetEffectOffset_Y() = 0;
	Player* GetFirstAttacker() const { return FirstAttacker; }
	Player* GetCurrentAttacker() const { return CurrentAttacker; }
	Player* GetLastAttacker() const { return LastAttacker; }
	int GetAttackedDamage(int playerNo) const {	return attackedDamage[playerNo]; }
	int GetAttackedDamagePersent(int playerNo) const {
		return 100 * attackedDamage[playerNo] / (attackedDamage[PL1P] + attackedDamage[PLAI]);
	}
	// �U������Ă��Ȃ��ꍇ��nullptr�ŕԂ�
	Player* GetMostAttackPlayer() const;

protected:
	DirectX::XMFLOAT3 targetPosition = { 0,0,0 };

	int attackDamage = 0; // �^����_���[�W

	// �p�����[�^
	Player* FirstAttacker = nullptr;
	Player* CurrentAttacker = nullptr;
	Player* LastAttacker = nullptr;
	int attackedDamage[2] = { 0,0 };
};

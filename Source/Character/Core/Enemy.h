#pragma once

#include "Constant/UtilsDefineConst.h"
#include "Constant/PlayerConst.h"
#include "Shader/Shader.h"
#include "Character.h"


class Player;

class Enemy :public Character
{
protected:
	// �Ώۈʒu
	DirectX::XMFLOAT3 targetPosition = { 0,0,0 };

	// �^����_���[�W
	int attackDamage = 0;

	// ��ԖڂɍU�������v���C���[
	Player* FirstAttacker = nullptr;
	// ���߂ɍU�������v���C���[
	Player* CurrentAttacker = nullptr;
	// �Ƃǂ߂������v���C���[
	Player* LastAttacker = nullptr;
	// ���g���_���[�W���󂯂���(�v���C���[��)
	int attackedDamage[player_count] = { 0,0 };

public:
	Enemy(){}
	~Enemy() override{}

	// �X�V����
	virtual void Update(float elapsedTime) = 0;

	// �V���h�E�}�b�v�`��
	virtual void ShadowRender(const RenderContext& rc, ShadowMap* shadowMap) = 0;
	// �`��
	virtual void Render(const RenderContext& rc, ModelShader* shader) = 0;

	// �j��
	void Destroy();

	// �f�o�b�O�v���~�e�B�u�`��
	virtual void DrawDebugPrimitive();

	// �U������Ă��Ȃ��ꍇ��nullptr�ŕԂ�
	Player* GetMostAttackPlayer() const;

	// �_���[�W���Z
	bool ApplyDamage(int damage, float invincibleTime, Player* attacker, int playerNo);

	//Setter
	void SetTargetPosition(DirectX::XMFLOAT3& pos) { targetPosition = pos; }
	void SetFirstAttacker(Player* player) { FirstAttacker = player; }
	void SetLastAttacker(Player* player) { LastAttacker = player; }
	void AddAttackedDamage(int playerNo, int addDamage) { attackedDamage[playerNo] += addDamage; }

	//Getter
	virtual const float GetEffectOffset_Y() = 0;
	const Player* GetFirstAttacker() const { return FirstAttacker; }
	const Player* GetCurrentAttacker() const { return CurrentAttacker; }
	const Player* GetLastAttacker() const { return LastAttacker; }
	const int GetAttackedDamage(int playerNo) const {	return attackedDamage[playerNo]; }
	const int GetAttackedDamagePersent(int playerNo) const {
		return 100 * attackedDamage[playerNo] / (attackedDamage[p1_serial_number] + attackedDamage[ai_serial_number]);
	}
};

#include "Enemy.h"
#include "Graphics/Graphics.h"
#include "Character/Manager/EnemyManager.h"
#include "Character/Manager/PlayerManager.h"

//�j��
void Enemy::Destroy()
{
	EnemyManager::Instance().Remove(this);
}

//�f�o�b�O�v���~�e�B�u�`��
void Enemy::DrawDebugPrimitive()
{
	Gizmos* gizmos = Graphics::Instance().GetGizmos();

	//�Փ˔���p�̃f�o�b�O����`��
	gizmos->DrawCylinder(position, radius, height, {}, DirectX::XMFLOAT4(0, 0, 0, 1));
}

bool Enemy::ApplyDamage(int damage, float invincibleTime, Player* attacker, int playerNo)
{
	// ���ߍU���̃v���C���[
	CurrentAttacker = attacker;
	if (health == maxHealth)
	{
		// ��ԏ��߂ɍU�������v���C���[
		FirstAttacker = attacker;
	}
	if (health - damage <= 0)
	{
		// �Ƃǂ߂��h�����v���C���[
		LastAttacker = attacker;
		// ���݂̍U���Ώۃ��Z�b�g
		attacker->SetCurrentAttackEnemy(nullptr);
	}
	bool success = Character::ApplyDamage(damage, invincibleTime);
	if (success)
	{
		AddAttackedDamage(playerNo, damage);
	}
	return success;
}

Player* Enemy::GetMostAttackPlayer() const
{
	Player* attackManager = nullptr;
	int mostDamage = 0;
	int i = 0;
	for (Player* player : PlayerManager::Instance().players)
	{
		if (attackedDamage[i] > mostDamage) {
			attackManager = player;
		}
		i++;
	}
	return attackManager;
}

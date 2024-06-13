#include "Enemy.h"
#include "Graphics/Graphics.h"
#include "Character/Manager/EnemyManager.h"
#include "Character/Manager/PlayerManager.h"

//破棄
void Enemy::Destroy()
{
	EnemyManager::Instance().Remove(this);
}

//デバッグプリミティブ描画
void Enemy::DrawDebugPrimitive()
{
	Gizmos* gizmos = Graphics::Instance().GetGizmos();

	//衝突判定用のデバッグ球を描画
	gizmos->DrawCylinder(position, radius, height, {}, DirectX::XMFLOAT4(0, 0, 0, 1));
}

bool Enemy::ApplyDamage(int damage, float invincibleTime, Player* attacker, int playerNo)
{
	// 直近攻撃のプレイヤー
	CurrentAttacker = attacker;
	if (health == maxHealth)
	{
		// 一番初めに攻撃したプレイヤー
		FirstAttacker = attacker;
	}
	if (health - damage <= 0)
	{
		// とどめを刺したプレイヤー
		LastAttacker = attacker;
		// 現在の攻撃対象リセット
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

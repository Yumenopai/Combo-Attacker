#include "Enemy.h"
#include "Graphics/Graphics.h"
#include "EnemyManager.h"
#include "Player.h"

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
	}
	bool success = Character::ApplyDamage(damage, invincibleTime);
	if (success)
	{
		AddAttackedDamage(playerNo, damage);
	}
	return success;
}

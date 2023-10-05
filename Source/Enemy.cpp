#include "Enemy.h"
#include "Graphics/Graphics.h"
#include "EnemyManager.h"

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
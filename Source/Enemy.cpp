#include "Enemy.h"
#include "Graphics/Graphics.h"
#include "EnemyManager.h"

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
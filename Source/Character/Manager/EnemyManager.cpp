#include "EnemyManager.h"
#include "Graphics/Graphics.h"
#include "Collision.h"

//�X�V����
void EnemyManager::Update(float elapsedTime)
{
	for (Enemy* enemy : enemies)
	{
		enemy->Update(elapsedTime);
	}

	//projectile���Q�l�ɔj������
	for (Enemy* enemy : removes)
	{
		auto it = std::find(enemies.begin(), enemies.end(), enemy);
		if (it != enemies.end()) {
			enemies.erase(it);
		}
	}
	removes.clear();

	//�G���m�̏Փˏ���
	CollisionEnemyVsEnemies();
}

//�`�揈��
void EnemyManager::ShadowRender(const RenderContext& rc, ShadowMap* shadowMap)
{
	for (Enemy* enemy : enemies)
	{
		//�V���h�E�}�b�v�Ƀ��f���`��
		enemy->ShadowRender(rc, shadowMap);
	}
}
void EnemyManager::Render(const RenderContext& rc, ModelShader* shader)
{
	for (Enemy* enemy : enemies)
	{
		enemy->Render(rc, shader);
	}
}

//�f�o�b�O�v���~�e�B�u�`��
void EnemyManager::DrawDebugPrimitive()
{
	for (Enemy* enemy : enemies)
	{
		enemy->DrawDebugPrimitive();
	}
}

//�G�l�~�[�o�^
void EnemyManager::Register(Enemy* enemy)
{
	enemies.emplace_back(enemy);
}

//�G�l�~�[�폜�\��
void EnemyManager::Remove(Enemy* enemy)
{
	//�j�����X�g�ɒǉ�
	removes.insert(enemy);
}

//�G�l�~�[�S�폜
void EnemyManager::Clear()
{
	enemies.clear();
}

//�G�l�~�[���m�̏Փˏ���
void EnemyManager::CollisionEnemyVsEnemies()
{
	int count = enemies.size();

	for (int i = 0; i < count; i++) {
		Enemy* enemyA = enemies.at(i);

		for (int j = i + 1; j < count; j++) {
			Enemy* enemyB = enemies.at(j);

			DirectX::XMFLOAT3 outPosition;
			if (Collision::IntersectCylinderVsCylinder(
				enemyA->GetPosition(), enemyA->GetRadius(), enemyA->GetHeight(),
				enemyB->GetPosition(), enemyB->GetRadius(), enemyB->GetHeight(),
				XMFLOAT3{},outPosition))
			{
				//�����o����̈ʒu�ݒ�
				enemyB->SetPosition(outPosition);
			}
		}
	}
}
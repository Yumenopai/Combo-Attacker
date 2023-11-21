#include "ItemManager.h"
#include "Graphics/Graphics.h"
#include "Collision.h"
#include "Player1P.h"

//�X�V����
void ItemManager::Update(float elapsedTime)
{
	for (Enemy* enemy : enemies)
	{
		if (enemy->GetPosition().x <= Player1P::Instance().GetPosition().x + 25
			&& enemy->GetPosition().x >= Player1P::Instance().GetPosition().x -25)
			
		enemy->Update(elapsedTime);
	}

	//projectile���Q�l�ɔj������
	for (Enemy* enemy : removes)
	{
		auto it = std::find(enemies.begin(), enemies.end(), enemy);
		if (it != enemies.end())	enemies.erase(it);

		delete enemy;
	}
	removes.clear();

	//�G���m�̏Փˏ���
	CollisionEnemyVsEnemies();
}

//�`�揈��
void ItemManager::ShadowRender(const RenderContext& rc, ShadowMap* shadowMap)
{
	for (Enemy* enemy : enemies)
	{
		//�V���h�E�}�b�v�Ƀ��f���`��
		enemy->ShadowRender(rc, shadowMap);
	}
}
void ItemManager::Render(const RenderContext& rc, ModelShader* shader)
{
	for (Enemy* enemy : enemies)
	{
		enemy->Render(rc, shader);
	}
}

//�G�l�~�[�o�^
void ItemManager::Register(Enemy* enemy)
{
	enemies.emplace_back(enemy);
}

//�G�l�~�[�폜�\��
void ItemManager::Remove(Enemy* enemy)
{
	//�j�����X�g�ɒǉ�
	removes.insert(enemy);
}

//�G�l�~�[�S�폜
void ItemManager::Clear()
{
	for (Enemy* enemy : enemies)
	{
		delete enemy;
	}
	enemies.clear();
}

//�G�l�~�[���m�̏Փˏ���
void ItemManager::CollisionEnemyVsEnemies()
{
	size_t enemyCount = enemies.size();

	for (size_t i = 0; i < enemyCount; i++) {
		Enemy* enemyA = enemies.at(i);

		for (size_t j = i + 1; j < enemyCount; j++) {
			Enemy* enemyB = enemies.at(j);

			DirectX::XMFLOAT3 outPosition;
			if (Collision::IntersectCylinderVsCylinder(
				enemyA->GetPosition(), enemyA->GetRadius(), enemyA->GetHeight(),
				enemyB->GetPosition(), enemyB->GetRadius(), enemyB->GetHeight(),
				XMFLOAT3{}, outPosition))
			{
				//�����o����̈ʒu�ݒ�
				enemyB->SetPosition(outPosition);
			}
		}
	}
}
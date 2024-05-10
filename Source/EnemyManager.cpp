#include "EnemyManager.h"
#include "Graphics/Graphics.h"
#include "Collision.h"
#include "Player.h"

//更新処理
void EnemyManager::Update(float elapsedTime)
{
	for (Enemy* enemy : enemies)
	{
		enemy->Update(elapsedTime);
	}

	//projectileを参考に破棄処理
	for (Enemy* enemy : removes)
	{
		auto it = std::find(enemies.begin(), enemies.end(), enemy);
		if (it != enemies.end())	enemies.erase(it);

		delete enemy;
	}
	removes.clear();

	//敵同士の衝突処理
	CollisionEnemyVsEnemies();
}

//描画処理
void EnemyManager::ShadowRender(const RenderContext& rc, ShadowMap* shadowMap)
{
	for (Enemy* enemy : enemies)
	{
		//シャドウマップにモデル描画
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

//デバッグプリミティブ描画
void EnemyManager::DrawDebugPrimitive()
{
	for (Enemy* enemy : enemies)
	{
		enemy->DrawDebugPrimitive();
	}
}

//エネミー登録
void EnemyManager::Register(Enemy* enemy)
{
	enemies.emplace_back(enemy);
}

//エネミー削除予約
void EnemyManager::Remove(Enemy* enemy)
{
	//破棄リストに追加
	removes.insert(enemy);
}

//エネミー全削除
void EnemyManager::Clear()
{
	// uniquePointerのためDeleteは不要
	//for (Enemy* enemy : enemies)
	//{
	//	delete enemy;
	//}
	enemies.clear();
}

//エネミー同士の衝突処理
void EnemyManager::CollisionEnemyVsEnemies()
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
				XMFLOAT3{},outPosition))
			{
				//押し出し後の位置設定
				enemyB->SetPosition(outPosition);
			}
		}
	}
}
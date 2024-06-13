#include "EnemyManager.h"
#include "Graphics/Graphics.h"
#include "Collision.h"

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
		if (it != enemies.end()) {
			enemies.erase(it);
		}
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
	enemies.clear();
}

//エネミー同士の衝突処理
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
				//押し出し後の位置設定
				enemyB->SetPosition(outPosition);
			}
		}
	}
}
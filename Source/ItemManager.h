#pragma once

#include <vector>
#include <set>
#include "Enemy.h"

class ItemManager
{
private:
	ItemManager() {}
	~ItemManager() {}

public:
	//唯一のインスタンス取得
	static ItemManager& Instance()
	{
		static ItemManager instance;
		return instance;
	}

	//更新処理
	void Update(float elapsedTime);

	//描画処理
	void ShadowRender(const RenderContext& rc, ShadowMap* shadowMap);
	void Render(const RenderContext& rc, ModelShader* shader);

	//エネミー登録
	void Register(Enemy* enemy);

	//エネミー数取得
	int GetEnemyCount() const { return static_cast<int>(enemies.size()); }

	//エネミー取得
	Enemy* GetEnemy(int index) { return enemies.at(index); }

	//エネミー削除
	void Remove(Enemy* enemy);

	//エネミー全削除
	void Clear();
	void CollisionEnemyVsEnemies();

	//複数エネミー管理のため、エネミーポインターをここで管理
	std::vector<Enemy*>		enemies;

private:
	std::set<Enemy*> removes;
};
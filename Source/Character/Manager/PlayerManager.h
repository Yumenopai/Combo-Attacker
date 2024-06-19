#pragma once

#include <vector>
#include <set>
#include "Character/Core/Player.h"

class PlayerManager
{
private:
	PlayerManager() {}
	~PlayerManager() {}

public:
	//唯一のインスタンス取得
	static PlayerManager& Instance()
	{
		static PlayerManager instance;
		return instance;
	}

	//更新処理
	void Update(float elapsedTime);

	//描画処理
	void ShadowRender(const RenderContext& rc, ShadowMap* shadowMap);
	void Render3d(const RenderContext& rc, ModelShader* shader);

	//デバッグプリミティブ描画
	void Render2d(const RenderContext & rc, Sprite* gauge, FontSprite* font,
		Sprite* frame, Sprite* weapon, Sprite* notification);

	//プレイヤー登録
	void Register(Player* player);

	//プレイヤー数取得
	int GetPlayerCount() const { return static_cast<int>(players.size()); }

	//プレイヤー取得
	Player* GetPlayer(int index) { return players.at(index); }

	//プレイヤー削除
	void Remove(Player* player);

	//プレイヤー全削除
	void Clear();
	void CollisionPlayerVsPlayer();

	//複数管理でポインターをここで管理
	std::vector<Player*> players;

private:
	//削除予約
	std::set<Player*> removes;
};
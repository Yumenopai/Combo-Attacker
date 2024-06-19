#include "PlayerManager.h"
#include "Graphics/Graphics.h"
#include "Collision.h"

//更新処理
void PlayerManager::Update(float elapsedTime)
{
	for (Player* player : players)
	{
		player->Update(elapsedTime);
	}

	//破棄処理
	for (Player* player : removes)
	{
		auto it = std::find(players.begin(), players.end(), player);
		if (it != players.end()) {
			players.erase(it);
		}
	}
	removes.clear();

	//プレイヤー同士の衝突処理
	CollisionPlayerVsPlayer();
}

//描画処理
void PlayerManager::ShadowRender(const RenderContext& rc, ShadowMap* shadowMap)
{
	for (Player* player : players)
	{
		//シャドウマップにモデル描画
		player->ShadowRender(rc, shadowMap);
	}
}
void PlayerManager::Render3d(const RenderContext& rc, ModelShader* shader)
{
	for (Player* player : players)
	{
		player->Render3d(rc, shader);
	}
}

//プリミティブ描画
void PlayerManager::Render2d(const RenderContext& rc, Sprite* gauge, FontSprite* font, 
	Sprite* button, Sprite* weapon, Sprite* notification)
{
	for (Player* player : players)
	{
		player->Render2d(rc, gauge, font, button, weapon, notification);
	}
}

//player登録
void PlayerManager::Register(Player* player)
{
	players.emplace_back(player);
}

//player削除予約
void PlayerManager::Remove(Player* player)
{
	//破棄リストに追加
	removes.insert(player);
}

//player全削除
void PlayerManager::Clear()
{
	players.clear();
}

//player同士の衝突処理
void PlayerManager::CollisionPlayerVsPlayer()
{
	int count = SC_INT(players.size());

	for (int i = 0; i < count; i++) {
		Player* playerA = players.at(i);

		for (int j = i + 1; j < count; j++) {
			Player* playerB = players.at(j);

			DirectX::XMFLOAT3 outPosition;
			if (Collision::IntersectCylinderVsCylinder(
				playerA->GetPosition(), playerA->GetRadius(), playerA->GetHeight(),
				playerB->GetPosition(), playerB->GetRadius(), playerB->GetHeight(),
				XMFLOAT3{}, outPosition))
			{
				//押し出し後の位置設定
				playerB->SetPosition(outPosition);
			}
		}
	}
}
#include "PlayerManager.h"
#include "Graphics/Graphics.h"
#include "Collision.h"
#include "Character/Player.h"

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
		if (it != players.end())	players.erase(it);

		delete player;
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
void PlayerManager::Render(const RenderContext& rc, ModelShader* shader)
{
	for (Player* player : players)
	{
		player->Render(rc, shader);
	}
}

//プリミティブ描画
void PlayerManager::Render2d(const RenderContext& rc, Sprite* gauge, FontSprite* font, Sprite* frame, Sprite* weapon, Sprite* message)
{
	for (Player* player : players)
	{
		player->RenderHPBar(rc.deviceContext, gauge, font);
		player->RenderCharacterOverHead(rc, font, message);
		player->RenderHaveWeapons(rc.deviceContext, frame, weapon);
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
	// uniquePointerのためDeleteは不要
	//for (Player* player : players)
	//{
	//	delete player;
	//}
	players.clear();
}

//player同士の衝突処理
void PlayerManager::CollisionPlayerVsPlayer()
{
	Player1P& player1P = Player1P::Instance();
	PlayerAI& playerAI = PlayerAI::Instance();

	DirectX::XMFLOAT3 outPosition;
	if (Collision::IntersectCylinderVsCylinder(
		player1P.GetPosition(), player1P.GetRadius(), player1P.GetHeight(),
		playerAI.GetPosition(), playerAI.GetRadius(), playerAI.GetHeight(),
		XMFLOAT3{}, outPosition))
	{
		//押し出し後の位置設定
		playerAI.SetPosition(outPosition);
	}
}
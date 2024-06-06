#include "PlayerManager.h"
#include "Graphics/Graphics.h"
#include "Collision.h"
#include "Character/Player.h"

//�X�V����
void PlayerManager::Update(float elapsedTime)
{
	for (Player* player : players)
	{
		player->Update(elapsedTime);
	}

	//�j������
	for (Player* player : removes)
	{
		auto it = std::find(players.begin(), players.end(), player);
		if (it != players.end())	players.erase(it);

		delete player;
	}
	removes.clear();

	//�v���C���[���m�̏Փˏ���
	CollisionPlayerVsPlayer();
}

//�`�揈��
void PlayerManager::ShadowRender(const RenderContext& rc, ShadowMap* shadowMap)
{
	for (Player* player : players)
	{
		//�V���h�E�}�b�v�Ƀ��f���`��
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

//�v���~�e�B�u�`��
void PlayerManager::Render2d(const RenderContext& rc, Sprite* gauge, FontSprite* font, Sprite* frame, Sprite* weapon, Sprite* message)
{
	for (Player* player : players)
	{
		player->RenderHPBar(rc.deviceContext, gauge, font);
		player->RenderCharacterOverHead(rc, font, message);
		player->RenderHaveWeapons(rc.deviceContext, frame, weapon);
	}
}

//player�o�^
void PlayerManager::Register(Player* player)
{
	players.emplace_back(player);
}

//player�폜�\��
void PlayerManager::Remove(Player* player)
{
	//�j�����X�g�ɒǉ�
	removes.insert(player);
}

//player�S�폜
void PlayerManager::Clear()
{
	// uniquePointer�̂���Delete�͕s�v
	//for (Player* player : players)
	//{
	//	delete player;
	//}
	players.clear();
}

//player���m�̏Փˏ���
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
		//�����o����̈ʒu�ݒ�
		playerAI.SetPosition(outPosition);
	}
}
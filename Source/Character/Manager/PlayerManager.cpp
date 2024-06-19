#include "PlayerManager.h"
#include "Graphics/Graphics.h"
#include "Collision.h"

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
		if (it != players.end()) {
			players.erase(it);
		}
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
void PlayerManager::Render3d(const RenderContext& rc, ModelShader* shader)
{
	for (Player* player : players)
	{
		player->Render3d(rc, shader);
	}
}

//�v���~�e�B�u�`��
void PlayerManager::Render2d(const RenderContext& rc, Sprite* gauge, FontSprite* font, 
	Sprite* button, Sprite* weapon, Sprite* notification)
{
	for (Player* player : players)
	{
		player->Render2d(rc, gauge, font, button, weapon, notification);
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
	players.clear();
}

//player���m�̏Փˏ���
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
				//�����o����̈ʒu�ݒ�
				playerB->SetPosition(outPosition);
			}
		}
	}
}
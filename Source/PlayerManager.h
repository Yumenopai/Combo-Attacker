#pragma once

#include <vector>
#include <set>
#include "Player.h"
#include "Player1P.h"
#include "PlayerAI.h"

class PlayerManager
{
private:
	PlayerManager() {}
	~PlayerManager() {}

public:
	//�B��̃C���X�^���X�擾
	static PlayerManager& Instance()
	{
		static PlayerManager instance;
		return instance;
	}

	//�X�V����
	void Update(float elapsedTime);

	//�`�揈��
	void ShadowRender(const RenderContext& rc, ShadowMap* shadowMap);
	void Render(const RenderContext& rc, ModelShader* shader);

	//�f�o�b�O�v���~�e�B�u�`��
	void Render2d(const RenderContext & rc, Sprite* gauge, FontSprite* font, Sprite* frame, Sprite* arm, Sprite* message);

	//�v���C���[�o�^
	void Register(Player* player);

	//�v���C���[���擾
	int GetPlayerCount() const { return static_cast<int>(players.size()); }

	//�v���C���[�擾
	Player* GetPlayer(int index) { return players.at(index); }

	//�v���C���[�폜
	void Remove(Player* player);

	//�v���C���[�S�폜
	void Clear();
	void CollisionPlayerVsPlayer();

	//�����Ǘ��Ń|�C���^�[�������ŊǗ�
	std::vector<Player*> players;

private:

	std::set<Player*> removes;
};
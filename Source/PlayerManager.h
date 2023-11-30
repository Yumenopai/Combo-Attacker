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
	void Render2d(const RenderContext & rc, Sprite* gauge);

	//�G�l�~�[�o�^
	void Register(Player* player);

	//�G�l�~�[���擾
	int GetPlayerCount() const { return static_cast<int>(players.size()); }

	//�G�l�~�[�擾
	Player* GetPlayer(int index) { return players.at(index); }

	//�G�l�~�[�폜
	void Remove(Player* player);

	//�G�l�~�[�S�폜
	void Clear();
	void CollisionPlayerVsPlayer();

	//�����Ǘ��Ń|�C���^�[�������ŊǗ�
	std::vector<Player*> players;

private:

	std::set<Player*> removes;
};
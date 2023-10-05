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
	//�B��̃C���X�^���X�擾
	static ItemManager& Instance()
	{
		static ItemManager instance;
		return instance;
	}

	//�X�V����
	void Update(float elapsedTime);

	//�`�揈��
	void ShadowRender(const RenderContext& rc, ShadowMap* shadowMap);
	void Render(const RenderContext& rc, ModelShader* shader);

	//�G�l�~�[�o�^
	void Register(Enemy* enemy);

	//�G�l�~�[���擾
	int GetEnemyCount() const { return static_cast<int>(enemies.size()); }

	//�G�l�~�[�擾
	Enemy* GetEnemy(int index) { return enemies.at(index); }

	//�G�l�~�[�폜
	void Remove(Enemy* enemy);

	//�G�l�~�[�S�폜
	void Clear();
	void CollisionEnemyVsEnemies();

	//�����G�l�~�[�Ǘ��̂��߁A�G�l�~�[�|�C���^�[�������ŊǗ�
	std::vector<Enemy*>		enemies;

private:
	std::set<Enemy*> removes;
};
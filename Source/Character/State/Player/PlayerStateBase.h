#pragma once

class Player;
class PlayerStateBase
{
protected:
	Player* player;

public:
	PlayerStateBase(Player* pl) : player(pl){}
	virtual ~PlayerStateBase() {}

	// �X�e�[�g�J��
	virtual void Init() = 0;

	// �X�e�[�g�X�V
	virtual void Update(float elapsedTime) = 0;
};
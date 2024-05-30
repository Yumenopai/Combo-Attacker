#pragma once

class Player;
class PlayerStateBase
{
protected:
	Player* player;

public:
	PlayerStateBase(Player* pl) : player(pl){}
	virtual ~PlayerStateBase() {}

	// ステート遷移
	virtual void Init() = 0;

	// ステート更新
	virtual void Update(float elapsedTime) = 0;
};
#pragma once

#include <vector> 
#include "PlayerStateBase.h" 

class PlayerStateMachine
{
private:
	// 現在のステート 
	PlayerStateBase* currentState = nullptr;
	// 各ステートを保持する配列 
	std::vector<PlayerStateBase*> statePool;

public:
	~PlayerStateMachine();
	// 更新処理 
	void Update(float elapsedTime);
	// Getter
	int GetStateNumber();
	// ステートセット 
	void SetState(int newState);
	// ステート変更 
	void ChangeState(int newState);
	// ステート登録 
	void RegisterState(PlayerStateBase* state);
};
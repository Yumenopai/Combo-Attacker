#include "PlayerStateMachine.h"

// デストラクタ 
PlayerStateMachine::~PlayerStateMachine()
{
	// 登録したステートを削除する 
	for (auto* state : statePool)
	{
		delete state;
	}
	statePool.clear();
}

// 更新処理
void PlayerStateMachine::Update(float elapsedTime)
{
	currentState->Update(elapsedTime);
}

// Getter
int PlayerStateMachine::GetStateNumber()
{
	int stateNumber = 0;
	for (auto* state : statePool)
	{
		if (currentState == state) break;

		stateNumber++;
	}

	return stateNumber;
}

// ステートセット
void PlayerStateMachine::SetState(int newState)
{
	currentState = statePool.at(newState);
	currentState->Init();
}

// ステート変更
void PlayerStateMachine::ChangeState(int newState)
{
	SetState(newState);
}

// ステート登録
void PlayerStateMachine::RegisterState(PlayerStateBase* state)
{
	// ステート登録
	statePool.emplace_back(state);
}

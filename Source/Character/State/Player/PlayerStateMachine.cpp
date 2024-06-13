#include "PlayerStateMachine.h"

// �f�X�g���N�^ 
PlayerStateMachine::~PlayerStateMachine()
{
	// �o�^�����X�e�[�g���폜���� 
	for (auto* state : statePool)
	{
		delete state;
	}
	statePool.clear();
}

// �X�V����
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

// �X�e�[�g�Z�b�g
void PlayerStateMachine::SetState(int newState)
{
	currentState = statePool.at(newState);
	currentState->Init();
}

// �X�e�[�g�ύX
void PlayerStateMachine::ChangeState(int newState)
{
	SetState(newState);
}

// �X�e�[�g�o�^
void PlayerStateMachine::RegisterState(PlayerStateBase* state)
{
	// �X�e�[�g�o�^
	statePool.emplace_back(state);
}

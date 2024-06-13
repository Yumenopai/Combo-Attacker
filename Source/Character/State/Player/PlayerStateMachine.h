#pragma once

#include <vector> 
#include "PlayerStateBase.h" 

class PlayerStateMachine
{
private:
	// ���݂̃X�e�[�g 
	PlayerStateBase* currentState = nullptr;
	// �e�X�e�[�g��ێ�����z�� 
	std::vector<PlayerStateBase*> statePool;

public:
	~PlayerStateMachine();
	// �X�V���� 
	void Update(float elapsedTime);
	// Getter
	int GetStateNumber();
	// �X�e�[�g�Z�b�g 
	void SetState(int newState);
	// �X�e�[�g�ύX 
	void ChangeState(int newState);
	// �X�e�[�g�o�^ 
	void RegisterState(PlayerStateBase* state);
};
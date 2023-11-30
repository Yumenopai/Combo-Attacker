#include "EnemyTurtleShell.h"
#include "Graphics/Graphics.h"
#include "PlayerManager.h"
#include "Collision.h"
#include "MathScript.h"

//�R���X�g���N�^
EnemyTurtleShell::EnemyTurtleShell()
{
	ID3D11Device* device = Graphics::Instance().GetDevice();
	model = std::make_unique<Model>(device, "Data/Model/RPG_TurtleShell/TurtleShellPBR.fbx", 0.01f);

	angle.y = Math::RandomRange(-360, 360);
	//radius = 0.5f;
	height = 1.0f;
	//�ҋ@�X�e�[�g�֑J��
	TransitionState(State::Idle);
}

//�v���C���[���G
bool EnemyTurtleShell::SearchPlayer()
{
	XMVECTOR pPos = XMLoadFloat3(&targetPosition);
	XMVECTOR ePos = XMLoadFloat3(&position);
	float dist = XMVectorGetX(XMVector3LengthSq(XMVectorSubtract(pPos, ePos)));

	if (dist < searchRange * searchRange)
	{
		return true;
	}
	return false;
}

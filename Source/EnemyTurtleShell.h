#pragma once

#include <memory>
#include "Graphics/Model.h"
#include "EnemySlime.h"

class EnemyTurtleShell :public EnemySlime
{
public:
	EnemyTurtleShell();

	//�v���C���[���G
	bool SearchPlayer();
};
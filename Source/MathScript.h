#pragma once

class Math
{
public:
	//�w��͈͂̃����_���l���v�Z����
	static float RandomRange(float min, float max)
	{
		return min + (max - min) * (rand() / static_cast<float>(RAND_MAX));
	}
};
#pragma once

class Math
{
public:
	//指定範囲のランダム値を計算する
	static float RandomRange(float min, float max)
	{
		return min + (max - min) * (rand() / static_cast<float>(RAND_MAX));
	}
};
#pragma once

struct AnimationTime
{
	float start;
	float end;
};

static bool IsDuringTime(float currentTimer, AnimationTime designatedTime)
{
	return (currentTimer >= designatedTime.start && currentTimer <= designatedTime.end);
}
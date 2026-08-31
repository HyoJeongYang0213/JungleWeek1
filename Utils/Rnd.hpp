#pragma once 

#include <cstdlib>

namespace Rnd {
	inline float GetRandomFloat(float min, float max)
	{
		float random = ((float)rand()) / (float)RAND_MAX; // 0.0 ~ 1.0
		float range = max - min;
		return (random * range) + min; // min ~ max
	}

	inline int GetRandomInt(int min, int max) {
		return rand() % (max - min + 1) + min; // min ~ max
	}
}


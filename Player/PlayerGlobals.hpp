#pragma once
#include "Ball.h"

namespace PlayerGlobals {
	inline Vector3 PLAYERLOCATION = Vector3(0.0f, 0.0f, 0.0f);
	inline Ball* PLAYERBALL = nullptr;
	inline float HIGH_SCORE = 0.f;

	constexpr float PLAYER_DRAG_IMPULSE_MULTIPLIER = 5.f;
}
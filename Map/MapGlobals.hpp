#pragma once 
#include "../Utils/Math.hpp"

namespace MapGlobals {
	constexpr bool BOUND_BALL_TO_SCREEN = true;
	constexpr bool ENABLE_GRAVITY = true;


	constexpr Vector3 LEFT_BORDER_POINT		= Vector3(0.0f, 0.0f, 0.0f);
	constexpr Vector3 RIGHT_BORDER_POINT	= Vector3(15.0f, 0.0f, 0.0f);
	constexpr Vector3 TOP_BORDER_POINT		= Vector3(0.0f, 30.0f, 0.0f);
	constexpr Vector3 BOTTOM_BORDER_POINT	= Vector3(0.0f, 2.0f, 0.0f);

	constexpr Vector3 LEFT_BORDER_NORMAL	= Vector3(1.0f, 0.0f, 0.0f);
	constexpr Vector3 RIGHT_BORDER_NORMAL	= Vector3(-1.0f, 0.0f, 0.0f);
	constexpr Vector3 TOP_BORDER_NORMAL		= Vector3(0.0f, -1.0f, 0.0f);
	constexpr Vector3 BOTTOM_BORDER_NORMAL	= Vector3(0.0f, 1.0f, 0.0f);


	constexpr float LEFT_BORDER = 0.0f;
	constexpr float RIGHT_BORDER = 15.0f;
	constexpr float TOP_BORDER = 30.0f;
	constexpr float BOTTOM_BORDER = 0.0f;

	constexpr float WORLD_WIDTH = 15.0f;
	constexpr float VIEW_HEIGHT = 30.0f;
	constexpr float CHUNK_HEIGHT = 30.0f;
}
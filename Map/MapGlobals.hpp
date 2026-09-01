#pragma once 
#include "../Utils/Math.hpp"

namespace Globals {
	constexpr bool BOUND_BALL_TO_SCREEN = true;
	constexpr bool ENABLE_GRAVITY = true;


	constexpr Vector3 LEFT_BORDER_POINT		= Vector3(-2.0f, 0.0f, 0.0f);
	constexpr Vector3 RIGHT_BORDER_POINT	= Vector3(2.0f, 0.0f, 0.0f);
	constexpr Vector3 TOP_BORDER_POINT		= Vector3(0.0f, 2.0f, 0.0f);
	constexpr Vector3 BOTTOM_BORDER_POINT	= Vector3(0.0f, -2.0f, 0.0f);

	constexpr Vector3 LEFT_BORDER_NORMAL	= Vector3(1.0f, 0.0f, 0.0f);
	constexpr Vector3 RIGHT_BORDER_NORMAL	= Vector3(-1.0f, 0.0f, 0.0f);
	constexpr Vector3 TOP_BORDER_NORMAL		= Vector3(0.0f, -1.0f, 0.0f);
	constexpr Vector3 BOTTOM_BORDER_NORMAL	= Vector3(0.0f, 1.0f, 0.0f);


	constexpr float LEFT_BORDER = 2.0f;
	constexpr float RIGHT_BORDER = -2.0f;
	constexpr float TOP_BORDER = 2.0f;
	constexpr float BOTTOM_BORDER = -2.0f;

	constexpr int WINDOW_WIDTH = 384;
	constexpr int WINDOW_HEIGHT = 1000;

	constexpr float MAP_WIDTH = 1536.0f;
	constexpr float MAP_HEIGHT = 4000.0f;

	constexpr float VIEW_HEIGHT_PX = 2000.0f;
}
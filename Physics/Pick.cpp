#include "Pick.h"

#include "../Map/MapGlobals.hpp"

#include "../Player/Ball.h"
#include "../Player/CameraGlobals.hpp"
#include "../Player/PlayerGlobals.hpp" 

#include "../Renderer/WindowGlobals.hpp"

bool Pick::IsBallClicked(__int32 screenX, __int32 screenY)
{
	WindowSize worldPos = ScreenToWorld(screenX, screenY);


	// 공의위치, 마우스 위치 사이의 거리 계산
	Ball* ball = PlayerGlobals::PLAYERBALL;

	float dx = worldPos.Width - ball->GetLocation().x;
	float dy = worldPos.Height - ball->GetLocation().y;
	float dir = sqrtf(dx * dx + dy * dy);

	return dir <= ball->GetRadius();
}

WindowSize Pick::ScreenToWorld(__int32 screenX, __int32 screenY) {
	float screenWidth = WindowGlobals::SCREENSIZE.Width;
	float screenHeight = WindowGlobals::SCREENSIZE.Height;

	// screen -> NDC
	float ndcX = (screenX / (screenWidth / 2.0f)) - 1.0f;
	float ndcY = 1.0f - (screenY / (screenHeight / 2.0f));

	// NDC -> local (orthographic)
	float localX = (ndcX * (MapGlobals::RIGHT_BORDER - MapGlobals::LEFT_BORDER) + (MapGlobals::RIGHT_BORDER + MapGlobals::LEFT_BORDER)) / 2.0f;
	float localY = (ndcY * (MapGlobals::TOP_BORDER - MapGlobals::BOTTOM_BORDER) + (MapGlobals::TOP_BORDER + MapGlobals::BOTTOM_BORDER)) / 2.0f;

	// world = local + camera
	float worldX = localX + CameraGlobals::CAMERA_POSITION.x;
	float worldY = localY + CameraGlobals::CAMERA_POSITION.y;

	return { worldX, worldY };
}

POINT Pick::ScreenToWorld(POINT screenPos)
{
	WindowSize worldPos = ScreenToWorld(screenPos.x, screenPos.y);

	return { LONG(worldPos.Width), LONG(worldPos.Height) };
}

WindowSize Pick::WorldToScreen(Vector3 worldPos)
{
	float width = WindowGlobals::SCREENSIZE.Width;
	float height = WindowGlobals::SCREENSIZE.Height;

	// world -> local (orthographic)
	float localX = worldPos.x - CameraGlobals::CAMERA_POSITION.x;
	float localY = worldPos.y - CameraGlobals::CAMERA_POSITION.y;

	// local -> NDC 
	using namespace MapGlobals;
	float ndcX = (2.0f * localX - (RIGHT_BORDER + LEFT_BORDER)) / (RIGHT_BORDER - LEFT_BORDER);
	float ndcY = (2.0f * localY - (TOP_BORDER + BOTTOM_BORDER)) / (TOP_BORDER - BOTTOM_BORDER);

	// NDC -> screen
	float screenX = (ndcX + 1.0f) / 2.0f * width;
	float screenY = (1.0f - ndcY) / 2.0f * height;

	return { screenX, screenY };
}
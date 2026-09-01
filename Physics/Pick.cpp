#include "Pick.h"

#include "Ball.h"
#include "../Player/PlayerGlobals.hpp" 

#include "../Renderer/WindowGlobals.hpp"

bool Pick::IsBallClicked(__int32 screenX, __int32 screenY) {
	float width = WindowGlobals::SCREENSIZE.Width;
	float height = WindowGlobals::SCREENSIZE.Height;

	// projection -> NDC(-1~+1) -> view -> world
	// 원래는 카메라 projection/view 행렬 거쳐야지만
	// 아직 카메라 없으므로 화면중앙 = world 원점 가정!
	float worldX = (screenX - width / 2.0f) / (width / 2.0f);
	float worldY = -(screenY - height / 2.0f) / (height / 2.0f);

	// 공의위치, 마우스 위치 사이의 거리 계산
	Ball* ball = PlayerGlobals::PLAYERBALL;

	float dx = worldX - ball->GetLocation().x;
	float dy = worldY - ball->GetLocation().y;
	float dir = sqrtf(dx * dx + dy * dy);

	return dir <= ball->GetRadius();
}
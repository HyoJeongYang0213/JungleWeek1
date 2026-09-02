#include "Water.h"
#include "WaterGlobals.hpp"
#include "../Player/PlayerGlobals.hpp"
#include "MapGlobals.hpp"

Water::Water(ID3D11Buffer* vb, UINT numVertices)
	: mVertexBuffer{ vb }, mNumVertices{ numVertices } {
	mCenterX = (MapGlobals::LEFT_BORDER + MapGlobals::RIGHT_BORDER) / 2;
	mScaleX = (MapGlobals::RIGHT_BORDER - MapGlobals::LEFT_BORDER) / 1.6;
	mBaseY = MapGlobals::BOTTOM_BORDER;
}

Water::~Water()
{
}

void Water::Tick(float t)
{
	WaterGlobals::WATER_Y_SCALE += WaterGlobals::WATER_SPEED * t;
	if (IsGameOver())
	{
		WaterGlobals::B_GAME_OVER = true;
		OutputDebugStringA("게임오버!\n");
	}
} 

bool Water::IsGameOver() const
{
	const float wSurfaceY = mBaseY + WaterGlobals::WATER_Y_SCALE*0.8;

	if (PlayerGlobals::PLAYERLOCATION.y - PlayerGlobals::PLAYERBALL->GetRadius() <= wSurfaceY)
	{
		return true;
	}
	return false;
}


void Water::Render(IRenderer& renderer)
{;
	renderer.UpdateConstant(Vector3{ mCenterX, mBaseY, 0.0f }, Vector3{ mScaleX, WaterGlobals::WATER_Y_SCALE, 0.0f }, 0.0f);
	renderer.RenderPrimitive(mVertexBuffer, mNumVertices);
}

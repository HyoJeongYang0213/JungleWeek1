#pragma once
#include <iostream>
#include "../Player/Ball.h"
#include "../Renderer/WindowGlobals.hpp"
#include <Windows.h>

using namespace std;

class Pick
{
public:
	bool IsBallClicked(__int32 screenX, __int32 screenY);

	WindowSize ScreenToWorld(__int32 screenX, __int32 screenY);
	POINT ScreenToWorld(POINT screenPos);

	WindowSize WorldToScreen(Vector3 worldPos);
};


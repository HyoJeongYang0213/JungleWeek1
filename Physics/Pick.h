#pragma once
#include <iostream>
#include "Ball.h"
#include "../Renderer/WindowGlobals.hpp"

using namespace std;

class Pick
{
public:
	bool IsBallClicked(__int32 screenX, __int32 screenY);

	WindowSize ScreenToWorld(__int32 screenX, __int32 screenY);
	WindowSize WorldToScreen(Vector3 worldPos);
};


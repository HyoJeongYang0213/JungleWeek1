#pragma once 

struct WindowSize
{
	float Width;
	float Height;
};
	
namespace WindowGlobals {
	inline WindowSize SCREENSIZE = { 1024.0f, 1024.0f };
}
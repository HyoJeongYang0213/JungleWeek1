#pragma once 

struct WindowSize
{
	float Width;
	float Height;
};
	
namespace WindowGlobals {
	inline WindowSize SCREENSIZE = { 384.0f, 1024.0f };
}
#pragma once 
#include <Windows.h>
#include <vector>

enum class KEY_TYPE{
	LBUTTON = VK_LBUTTON,
	END,
};

enum class KEY_STATE {
	NONE,
	PRESS,
	DOWN,
	UP,
	END
};

class Input {
public:
	Input() : _states(static_cast<int>(KEY_TYPE::END), KEY_STATE::NONE) {} 

	const POINT& GetMousePos() const { return _mousePos; } 

	bool GetButton(KEY_TYPE key) { return GetKeyState(key) == KEY_STATE::PRESS; }
	bool GetButtonDown(KEY_TYPE key) { return GetKeyState(key) == KEY_STATE::DOWN; }
	bool GetButtonUp(KEY_TYPE key) { return GetKeyState(key) == KEY_STATE::UP; }

	void Update();

private:
	inline KEY_STATE GetKeyState(KEY_TYPE key)
	{
		return _states[static_cast<int>(key)];
	}

private:
	std::vector <KEY_STATE> _states;
	POINT _mousePos = {};
};
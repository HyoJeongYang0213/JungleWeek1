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
	Input() : _mStates(static_cast<int>(KEY_TYPE::END), KEY_STATE::NONE) {} 

	const POINT& GetMousePos() const { return _mMousePos; } 

	bool GetButton(KEY_TYPE key) { return GetKeyState(key) == KEY_STATE::PRESS; }
	bool GetButtonDown(KEY_TYPE key) { return GetKeyState(key) == KEY_STATE::DOWN; }
	bool GetButtonUp(KEY_TYPE key) { return GetKeyState(key) == KEY_STATE::UP; }

	POINT GetWorldPos(POINT screenPos) const;
	POINT GetTargetPos() const;

	void Update();
	void DragBall();

private:
	inline KEY_STATE GetKeyState(KEY_TYPE key)
	{
		return _mStates[static_cast<int>(key)];
	}

private:
	std::vector <KEY_STATE> _mStates;
	POINT _mMousePos = {};
	POINT _mTargetPos = {};
	bool _mIsDragging = false;
};
#include "Input.h"
#include <Windows.h>
#include "../Physics/Pick.h"

void Input::Update()
{
	::GetCursorPos(&_mousePos);
	::ScreenToClient(GetActiveWindow(), &_mousePos);

	bool isDown = (::GetAsyncKeyState(static_cast<int>(KEY_TYPE::LBUTTON)) & 0x8000) != 0;
	KEY_STATE prev = _states[static_cast<int>(KEY_TYPE::LBUTTON)];

	if (isDown) {
		// 누르고 있는 중이었고 왼쪽 버튼이 눌린 상태라면
		// 이전에도 누르고 있었다면 PRESS
		_states[static_cast<int>(KEY_TYPE::LBUTTON)] 
			= (prev == KEY_STATE::NONE || prev == KEY_STATE::UP) ? KEY_STATE::DOWN : KEY_STATE::PRESS;
	}
	else {
		// 이전에 누르고 있었고 지금은 눌리지 않은 상태라면 UP
		_states[static_cast<int>(KEY_TYPE::LBUTTON)] 
			= (prev == KEY_STATE::PRESS || prev == KEY_STATE::DOWN) ? KEY_STATE::UP : KEY_STATE::NONE;
	}
}

#include "Input.h"
#include <Windows.h>
#include "../Physics/Pick.h"

void Input::Update()
{
	::GetCursorPos(&_mMousePos);
	::ScreenToClient(GetActiveWindow(), &_mMousePos);

	bool isDown = (::GetAsyncKeyState(static_cast<int>(KEY_TYPE::LBUTTON)) & 0x8000) != 0;
	KEY_STATE prev = _mStates[static_cast<int>(KEY_TYPE::LBUTTON)];

	if (isDown) {
		// 누르고 있는 중이었고 왼쪽 버튼이 눌린 상태라면
		// 이전에도 누르고 있었다면 PRESS
		_mStates[static_cast<int>(KEY_TYPE::LBUTTON)] 
			= (prev == KEY_STATE::NONE || prev == KEY_STATE::UP) ? KEY_STATE::DOWN : KEY_STATE::PRESS;
	}
	else {
		// 이전에 누르고 있었고 지금은 눌리지 않은 상태라면 UP
		_mStates[static_cast<int>(KEY_TYPE::LBUTTON)] 
			= (prev == KEY_STATE::PRESS || prev == KEY_STATE::DOWN) ? KEY_STATE::UP : KEY_STATE::NONE;
	}

	if (GetButton(KEY_TYPE::LBUTTON))
	{
		Pick pick;
		POINT mousePos = GetMousePos();
		if (pick.IsBallClicked(mousePos.x, mousePos.y) == false) return;

		// Ball 이 클릭되었을 때 처리 시작! (화살표 UI, 운동 등)
		// 임시
		OutputDebugStringA("Ball Clicked!\n");
	}
}

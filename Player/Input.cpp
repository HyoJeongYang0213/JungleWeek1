#include "Input.h"
#include <Windows.h>
#include "../Physics/Pick.h"
#include "../Player/PlayerGlobals.hpp" 
#include "../ImGui/imgui.h"

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

	if (GetKeyState(KEY_TYPE::LBUTTON) == KEY_STATE::DOWN) {
		Pick pick;
		POINT _mScreenPos = GetMousePos();
		_mTargetPos = GetWorldPos(_mScreenPos);

		if (_mIsDragging) {
			OutputDebugStringA("Ball Released!\n");
			// 공발사 함수 여기서 호출하시면 됩니당~~~ 
			
			std::invoke(_mDragCallback, _mTargetPos);

			_mIsDragging = false;
			return;
		}
		else {
			if (pick.IsBallClicked(_mMousePos.x, _mMousePos.y) == true) {
				OutputDebugStringA("Ball Clicked!\n");
				_mIsDragging = !_mIsDragging;
			}
		}
	}
}

POINT Input::GetWorldPos(POINT screenPos) const
{
	Pick pick;
	return pick.ScreenToWorld(screenPos);
}

POINT Input::GetTargetPos() const
{
	Pick pick;
	return pick.ScreenToWorld(_mMousePos);
}

void Input::DragBall()
{
	if (_mIsDragging == false) return;

	Pick pick;
	POINT mousePos = GetMousePos();

	WindowSize ballScreen = pick.WorldToScreen(PlayerGlobals::PLAYERLOCATION);

	ImGui::GetForegroundDrawList()->AddLine(
		ImVec2(ballScreen.Width, ballScreen.Height),
		ImVec2((float)mousePos.x, (float)mousePos.y),
		IM_COL32(255, 0, 0, 255),
		3.0f
	);
}

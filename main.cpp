#include <windows.h>
#include <algorithm>
// 여기에 아래 코드를 추가 합니다.

// D3D 사용에 필요한 라이브러리들을 링크합니다.
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

// D3D 사용에 필요한 헤더파일들을 포함합니다.
#include <d3d11.h>
#include <d3dcompiler.h>

#include "UI/GameState.hpp"
#include "UI/GameButtonUI.hpp"
#include "UI/UIManager.h"

#include "Map/PlatformManager.h"
#include "Map/TextureLoader.hpp"
#include "Map/MapGenerator.h"
#include "Map/WaterGlobals.hpp"

#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_impl_dx11.h"
#include "imGui/imgui_impl_win32.h"


#include "Utils/Math.hpp"
#include "Physics/RigidBody.h"
#include "Physics/CollisionMask.h"
#include "Physics/Collider.h"
#include "Physics/CollisionManifold.hpp"
#include "Physics/CollisionDetector.h"
#include "Physics/CollisionResolver.h"
#include "Physics/Pick.h"

#include "Resource/vertexSimple.hpp" 
#include "Resource/Sphere.h"
#include "Resource/Square.hpp"
#include "Resource/PolygonGeometry.hpp"

#include "Player/Ball.h"
#include "Player/Input.h"
#include "PlayerGlobals.hpp"

#include "Map/Platform.h"
#include "Map/Polygon.h"

#include "Renderer/Renderer.h"
#include "Renderer/WindowGlobals.hpp"

#include "Scene/GameScene.h"
#include "Scene/SceneManager.h"

#include "Audio/SoundManager.h"

#include "UI/UIManager.h"


#include <iostream>
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>

// 삼각형을 하드 코딩
VertexSimple triangle_vertices[] =
{
	{  0.0f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f }, // Top vertex (red)
	{  1.0f, -1.0f, 0.0f,  0.0f, 1.0f, 0.0f, 1.0f }, // Bottom-right vertex (green)
	{ -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f }  // Bottom-left vertex (blue)
};

VertexTexture platform_vertices[] = {
	{ -1.0f,  1.0f, 0.0f,  0.0f, 0.0f },
	{  1.0f,  1.0f, 0.0f,  1.0f, 0.0f },
	{  1.0f, -1.0f, 0.0f,  1.0f, 1.0f },

	{ -1.0f,  1.0f, 0.0f,  0.0f, 0.0f },
	{  1.0f, -1.0f, 0.0f,  1.0f, 1.0f },
	{ -1.0f, -1.0f, 0.0f,  0.0f, 1.0f }
};
UINT numVerticesPlatform = 6;


extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
// 각종 메시지를 처리할 함수
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
	{
		return true;
	}

	switch (message)
	{
	case WM_DESTROY:
		// Signal that the app should quit
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// 윈도우 클래스 이름
	WCHAR WindowClass[] = L"JungleWindowClass";

	// 윈도우 타이틀바에 표시될 이름
	WCHAR Title[] = L"Game Tech Lab";

	// 각종 메시지를 처리할 함수인 WndProc의 함수 포인터를 WindowClass 구조체에 넣는다.
	WNDCLASSW wndclass = { 0, WndProc, 0, 0, 0, 0, 0, 0, 0, WindowClass };

	// 윈도우 클래스 등록
	RegisterClassW(&wndclass);

	// WindowGlobals::SCREENSIZE 크기의 윈도우 생성
	HWND hWnd = CreateWindowExW(0, WindowClass, Title, WS_POPUP | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, static_cast<int>(WindowGlobals::SCREENSIZE.Width), static_cast<int>(WindowGlobals::SCREENSIZE.Height),
		nullptr, nullptr, hInstance, nullptr);

	RECT clientRect;
	GetClientRect(hWnd, &clientRect);
	WindowGlobals::SCREENSIZE.Width = static_cast<float>(clientRect.right - clientRect.left);
	WindowGlobals::SCREENSIZE.Height = static_cast<float>(clientRect.bottom - clientRect.top);

	srand((UINT)GetTickCount64());

	bool bIsExit = false;

	//SoundManager 초기화
	SoundManager& soundManager =
		SoundManager::GetInstance();

	HRESULT hr =
		soundManager.Initialize();

	if (FAILED(hr))
	{
		MessageBoxA(
			hWnd,
			"SoundManager Initialize Failed",
			"Audio Error",
			MB_OK | MB_ICONERROR
		);

		return -1;
	}

	//여기에 메모리에 올리고 싶은 wavfile을 추가하세요
	hr = soundManager.LoadSound(L"Asset\\magic-forest.wav", "BGM");
	if (FAILED(hr))
	{
		MessageBoxA(
			hWnd,
			"BGM Load Failed",
			"Audio Error",
			MB_OK | MB_ICONERROR
		);

		return -1;
	}
	hr = soundManager.LoadSound(L"Asset\\metallic-ball.wav", "BallHit");
	if (FAILED(hr))
	{
		MessageBoxA(
			hWnd,
			"BallHit Load Failed",
			"Audio Error",
			MB_OK | MB_ICONERROR
		);

		return -1;
	}
	hr = soundManager.LoadSound(L"Asset\\game-over.wav", "GameOver");
	if (FAILED(hr))
	{
		MessageBoxA(
			hWnd,
			"GameOver Load Failed",
			"Audio Error",
			MB_OK | MB_ICONERROR
		);

		return -1;
	}



	// 각종 생성하는 코드를 여기에 추가합니다.
	Renderer renderer;
	renderer.Create(hWnd);
	renderer.CreateShader();
	renderer.CreateConstantBuffer();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();

	static const ImWchar customGlyphRanges[] = {
		0x0020, 0x00FF,
		0x2600, 0x26FF,
		0x3131, 0x318E,
		0xAC00, 0xD7A3,
		0,
	};

	ImFont* font = io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\malgun.ttf", 18.0f, nullptr, customGlyphRanges);
	if (!font)
	{
		io.Fonts->AddFontDefault();
	}

	ImGui_ImplWin32_Init((void*)hWnd);
	ImGui_ImplDX11_Init(renderer.Device, renderer.DeviceContext);


	//std::vector<std::tuple<ID3D11Buffer*, UINT, std::vector<Vector3>>> polygonVertexBuffers{};

	//{
	//	auto AddPolygonVertexBuffer = [&](auto& vertices, const auto& positions)
	//		{
	//			polygonVertexBuffers.emplace_back(
	//				renderer.CreateVertexBuffer(
	//					vertices,
	//					sizeof(vertices)
	//				),
	//				static_cast<UINT>(std::size(vertices)),
	//				positions
	//			);
	//		};

	//	// Convex
	//	AddPolygonVertexBuffer(convex_triangle_vertices, convex_triangle_positions);
	//	AddPolygonVertexBuffer(convex_quad_vertices, convex_quad_positions);
	//	AddPolygonVertexBuffer(convex_trapezoid_vertices, convex_trapezoid_positions);
	//	AddPolygonVertexBuffer(convex_pentagon_vertices, convex_pentagon_positions);
	//	AddPolygonVertexBuffer(convex_hexagon_vertices, convex_hexagon_positions);
	//	AddPolygonVertexBuffer(convex_octagon_vertices, convex_octagon_positions);
	//	AddPolygonVertexBuffer(convex_dodecagon_vertices, convex_dodecagon_positions);

	//	// Concave
	//	AddPolygonVertexBuffer(concave_arrow_vertices, concave_arrow_positions);
	//	AddPolygonVertexBuffer(concave_l_vertices, concave_l_positions);
	//	AddPolygonVertexBuffer(concave_u_vertices, concave_u_positions);
	//	AddPolygonVertexBuffer(concave_plus_vertices, concave_plus_positions);
	//	AddPolygonVertexBuffer(concave_c_vertices, concave_c_positions);
	//	AddPolygonVertexBuffer(concave_star_vertices, concave_star_positions);
	//	AddPolygonVertexBuffer(concave_lightning_vertices, concave_lightning_positions);
	//	AddPolygonVertexBuffer(concave_comb_vertices, concave_comb_positions);
	//	AddPolygonVertexBuffer(concave_spiral_vertices, concave_spiral_positions);
	//	AddPolygonVertexBuffer(concave_star16_vertices, concave_star16_positions);
	//}



	//// 고성능 타이머 초기화
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);

	LARGE_INTEGER previousTime;
	QueryPerformanceCounter(&previousTime);

	constexpr double FixedPhysicsStep = 1.0 / 120.0;
	constexpr double MaxFrameTime = 0.05;
	double physicsAccumulator = 0.0;



	//for (auto& pbuffer : polygonVertexBuffers)
	//{
	//	renderer.CreatePrimitive<PPolygon>(std::get<0>(pbuffer), std::get<1>(pbuffer), Vector3{Rnd::GetRandom(0.f, 15.f), Rnd::GetRandom(0.f, 30.f), 0.f}, std::get<2>(pbuffer));
	//}

	SceneManager sceneManager(renderer);
	//sceneManager.NextScene(); 
	UIManager::Get().Init(renderer.Device, &sceneManager);


	//메인 bgm play
	soundManager.PlaySound("BGM", 0.5f, true);
	while (bIsExit == false and not UIManager::Get().ShouldExit())
	{
		MSG msg;

		// 처리할 메시지가 더 이상 없을때 까지 수행
		while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			// 키 입력 메시지를 번역
			TranslateMessage(&msg);

			// 메시지를 적절한 윈도우 프로시저에 전달, 메시지가 위에서 등록한 WndProc 으로 전달됨
			DispatchMessage(&msg);

			if (msg.message == WM_QUIT)
			{
				bIsExit = true;
				break;
			}


			if (msg.message == WM_KEYDOWN && msg.wParam == VK_SPACE)
			{
				auto currentScene = sceneManager.GetCurrentScene();

				if (currentScene->GetSceneType() == SceneType::Game)
				{
					auto gameScene = static_cast<GameScene*>(currentScene);
					gameScene->SetTransparentBallMode(true);
				}
			}

			if (msg.message == WM_KEYUP && msg.wParam == VK_SPACE)
			{
				auto currentScene = sceneManager.GetCurrentScene();

				if (currentScene->GetSceneType() == SceneType::Game)
				{
					auto gameScene = static_cast<GameScene*>(currentScene);
					gameScene->SetTransparentBallMode(false);
				}
			}

			if (UIManager::Get().ShouldExit())
			{
				bIsExit = true;
			}
		}


		LARGE_INTEGER currentTime;
		QueryPerformanceCounter(&currentTime);

		double frameTime = static_cast<double>(currentTime.QuadPart - previousTime.QuadPart) / static_cast<double>(frequency.QuadPart);
		previousTime = currentTime;
		frameTime = (std::min)(frameTime, MaxFrameTime);
		physicsAccumulator += frameTime;

		while (physicsAccumulator >= FixedPhysicsStep)
		{
			sceneManager.Tick(static_cast<float>(FixedPhysicsStep));
			physicsAccumulator -= FixedPhysicsStep;
		}
		if (sceneManager.GetCurrentSceneIndex() == 1 && WaterGlobals::B_GAME_OVER)
		{
			WaterGlobals::B_GAME_OVER = false;
			sceneManager.NextScene();
		}
		soundManager.Update();

		//// -- Render 
		//sceneManager.Render(renderer);


		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		sceneManager.Render(renderer);
		UIManager::Get().Render(sceneManager.GetCurrentSceneIndex());

		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		renderer.SwapBuffer();
	}


	UIManager::Get().Shutdown();
	SoundManager::GetInstance().Shutdown();
	// 소멸하는 코드를 여기에 추가합니다.
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	// Renderer가 D3D 장치를 해제하기 전에 씬이 소유한 COM 객체를 먼저 정리합니다.
	sceneManager.Shutdown();

	renderer.ReleaseConstantBuffer();
	renderer.ReleaseShader();
	renderer.Release();

	return 0;
}

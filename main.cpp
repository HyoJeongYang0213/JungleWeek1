#include <windows.h>
// 여기에 아래 코드를 추가 합니다.

// D3D 사용에 필요한 라이브러리들을 링크합니다.
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

// D3D 사용에 필요한 헤더파일들을 포함합니다.
#include <d3d11.h>
#include <d3dcompiler.h>

#include "Map/PlatformManager.h"
#include "Map/TextureLoader.hpp"
#include "Map/MapGenerator.h"

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

#include "Resource/vertexSimple.hpp" 


#include "Resource/Sphere.h"
#include "Resource/Square.hpp"

#include "Player/Ball.h"
#include "Player/Input.h"
#include "PlayerGlobals.hpp"
#include "Map/Platform.h"

#include "Renderer/Renderer.h"
#include "Renderer/WindowGlobals.hpp"

// 삼각형을 하드 코딩
VertexSimple triangle_vertices[] =
{
	{  0.0f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f }, // Top vertex (red)
	{  1.0f, -1.0f, 0.0f,  0.0f, 1.0f, 0.0f, 1.0f }, // Bottom-right vertex (green)
	{ -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f }  // Bottom-left vertex (blue)
};


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

	// 윈도우 클래스 이름
	WCHAR WindowClass[] = L"JungleWindowClass";

	// 윈도우 타이틀바에 표시될 이름
	WCHAR Title[] = L"Game Tech Lab";

	// 각종 메시지를 처리할 함수인 WndProc의 함수 포인터를 WindowClass 구조체에 넣는다.
	WNDCLASSW wndclass = { 0, WndProc, 0, 0, 0, 0, 0, 0, 0, WindowClass };

	// 윈도우 클래스 등록
	RegisterClassW(&wndclass);

	// 1024 x 1024 크기에 윈도우 생성
	HWND hWnd = CreateWindowExW(0, WindowClass, Title, WS_POPUP | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, static_cast<int>(WindowGlobals::SCREENSIZE.Width), static_cast<int>(WindowGlobals::SCREENSIZE.Height),
		nullptr, nullptr, hInstance, nullptr);

	srand((UINT)GetTickCount64());

	bool bIsExit = false;

	// 각종 생성하는 코드를 여기에 추가합니다.
	Renderer renderer;
	renderer.Create(hWnd);
	renderer.CreateShader();
	renderer.CreateConstantBuffer();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init((void*)hWnd);
	ImGui_ImplDX11_Init(renderer.Device, renderer.DeviceContext);

	UINT numVerticesTriangle = sizeof(triangle_vertices) / sizeof(VertexSimple);
	UINT numVerticesSphere = sizeof(sphere_vertices) / sizeof(VertexSimple);
	UINT numVerticesCube = sizeof(cube_vertices) / sizeof(VertexSimple);

	ID3D11Buffer* vertexBufferTriangle = renderer.CreateVertexBuffer(triangle_vertices, sizeof(triangle_vertices));
	ID3D11Buffer* vertexBufferSphere = renderer.CreateVertexBuffer(sphere_vertices, sizeof(sphere_vertices));
	ID3D11Buffer* vertexBufferCube = renderer.CreateVertexBuffer(cube_vertices, sizeof(cube_vertices));

	ID3DBlob* VertexShaderBlob = nullptr, * psBlob = nullptr;
	ID3D11VertexShader* TextureVertexShader = nullptr;
	ID3D11PixelShader* TexturePixelShader = nullptr;
	ID3D11InputLayout* TextureLayout = nullptr;

	D3DCompileFromFile(L"TextureShader.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", 0, 0, &VertexShaderBlob, nullptr);
	renderer.Device->CreateVertexShader(VertexShaderBlob->GetBufferPointer(), VertexShaderBlob->GetBufferSize(), nullptr, &TextureVertexShader);

	D3DCompileFromFile(L"TextureShader.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", 0, 0, &psBlob, nullptr);
	renderer.Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &TexturePixelShader);

	D3D11_INPUT_ELEMENT_DESC TextureLayoutDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	renderer.Device->CreateInputLayout(TextureLayoutDesc, 2, VertexShaderBlob->GetBufferPointer(), VertexShaderBlob->GetBufferSize(), &TextureLayout);
	VertexShaderBlob->Release();
	psBlob->Release();

	enum ETypePrimitive
	{
		EPT_Triangle,
		EPT_Cube,
		EPT_Sphere,
		EPT_Max,
	};


	ETypePrimitive typePrimitive = EPT_Sphere;

	// 고성능 타이머 초기화
	LARGE_INTEGER frequency;
	QueryPerformanceFrequency(&frequency);

	LARGE_INTEGER startTime, endTime;
	double elapsedTime = 0.0;

	// Collision Mask 추가 부분

	CollisionMask collisionMask;


	if (!collisionMask.Load(
		"Asset/stage1_collision_mask_1536x3000.png"
	))
	{
		MessageBoxA(
			hWnd,
			"Failed to load CollisionMask.png",
			"Collision Mask Error",
			MB_OK | MB_ICONERROR
		);
	}


	// Collision Mask에서
	// Connected Component별 Platform Data 생성
	std::vector<PlatformCollisionData>
		platformData =
		collisionMask.BuildPlatformsNDC();

	renderer.CreatePrimitive<Ball>(vertexBufferSphere, numVerticesSphere);

	// 실제 Platform 생성
	/*for (const PlatformCollisionData& data :
		platformData)
	{
		renderer.CreatePrimitive<Platform>(
			vertexBufferCube,
			numVerticesCube,
			data.Center,
			data.HalfExtents
		);
	}*/


	Input input; 
	//renderer.CreatePrimitive<Platform>(vertexBufferCube, numVerticesCube);
	//renderer.CreatePrimitive<Platform>(vertexBufferCube, numVerticesCube);
	//renderer.CreatePrimitive<Platform>(vertexBufferCube, numVerticesCube);
	//renderer.CreatePrimitive<Platform>(vertexBufferCube, numVerticesCube);
	//renderer.CreatePrimitive<Platform>(vertexBufferCube, numVerticesCube);
	//renderer.CreatePrimitive<Platform>(vertexBufferCube, numVerticesCube);

	ID3D11ShaderResourceView* ShaderResourceViewGround = TextureLoader::CreateTextureFromFile(renderer.Device, L"Asset/Stage_Ground.png");
	ID3D11ShaderResourceView* ShaderResourceViewA = TextureLoader::CreateTextureFromFile(renderer.Device, L"Asset/Stage_A.png");
	ID3D11ShaderResourceView* ShaderResourceViewB = TextureLoader::CreateTextureFromFile(renderer.Device, L"Asset/Stage_B.png");
	ID3D11ShaderResourceView* ShaderResourceViewC = TextureLoader::CreateTextureFromFile(renderer.Device, L"Asset/Stage_C.png");

	ID3D11SamplerState* MapSampler = TextureLoader::CreateSamplerState(renderer.Device);

	InfiniteMap InfiniteMap;

	InfiniteMap.Init(renderer, ShaderResourceViewGround, { ShaderResourceViewA, ShaderResourceViewB, ShaderResourceViewC });

	float cameraCenterY = Globals::MAP_HEIGHT - (Globals::VIEW_HEIGHT_PX * 0.5f);

	ID3D11ShaderResourceView* ShaderResourceViewPlatform = TextureLoader::CreateTextureFromFile(renderer.Device, L"Asset/Platform.png");

	PlatformManager platformManager;
	platformManager.Init(renderer);

	// Main Loop (Quit Message가 들어오기 전까지 아래 Loop를 무한히 실행하게 됨)
	while (bIsExit == false)
	{
		QueryPerformanceCounter(&startTime);

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
		}


		input.Update();
		renderer.Tick(static_cast<float>(elapsedTime));

		Ball* player = dynamic_cast<Ball*>(renderer.PrimitiveList[0]);
		PlayerGlobals::PLAYERLOCATION = player->GetLocation();


		platformManager.Update(cameraCenterY);

		renderer.Prepare();

		renderer.DeviceContext->VSSetShader(TextureVertexShader, nullptr, 0);
		renderer.DeviceContext->PSSetShader(TexturePixelShader, nullptr, 0);
		renderer.DeviceContext->IASetInputLayout(TextureLayout);

		InfiniteMap.Render(renderer, MapSampler, cameraCenterY);

		platformManager.Render(renderer, ShaderResourceViewPlatform, cameraCenterY);

		renderer.PrepareShader(); // 단색 기본 셰이더로 복귀
		for (size_t i = 0; i < renderer.PrimitiveCount; ++i)
		{
			renderer.PrimitiveList[i]->Render(renderer);
		}

		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();


		ImGui::Render();
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		renderer.SwapBuffer();


		QueryPerformanceCounter(&endTime);
		elapsedTime = static_cast<double>(endTime.QuadPart - startTime.QuadPart) / static_cast<double>(frequency.QuadPart);

		////////////////////////////////////////////
	}

	// 소멸하는 코드를 여기에 추가합니다.
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	renderer.ReleaseVertexBuffer(vertexBufferTriangle);
	renderer.ReleaseVertexBuffer(vertexBufferSphere);
	renderer.ReleaseVertexBuffer(vertexBufferCube);

	renderer.ReleaseConstantBuffer();
	renderer.ReleaseShader();
	renderer.Release();

	if (ShaderResourceViewGround) ShaderResourceViewGround->Release();
	if (ShaderResourceViewA) ShaderResourceViewA->Release();
	if (ShaderResourceViewB) ShaderResourceViewB->Release();
	if (ShaderResourceViewC) ShaderResourceViewC->Release();
	if (ShaderResourceViewPlatform) ShaderResourceViewPlatform->Release();
	if (MapSampler) MapSampler->Release();

	if (TextureLayout) TextureLayout->Release();
	if (TextureVertexShader) TextureVertexShader->Release();
	if (TexturePixelShader) TexturePixelShader->Release();
	CoUninitialize();
	return 0;
}
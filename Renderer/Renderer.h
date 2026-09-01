#pragma once 

#include <d3d11.h>
#include <d3dcompiler.h>

#include "../Resource/Primitive.hpp"

#include "../Physics/Collider.h"
#include "../Physics/CollisionManifold.hpp"
#include "../Physics/CollisionDetector.h"
#include "../Physics/CollisionResolver.h"

#include "IRenderer.hpp"

#include "../Map/MapGlobals.hpp" 

#include "../Resource/vertexSimple.hpp"

#include "../Utils/Rnd.hpp" 

#include "../Player/Camera.h"

class Renderer : public IRenderer
{
public:
	Renderer();
	~Renderer();

	Renderer(const Renderer&) = delete;
	Renderer& operator=(const Renderer&) = delete;

	Renderer(Renderer&&) = default;
	Renderer& operator=(Renderer&&) = default;

public:
	// 렌더러 초기화 함수
	void Create(HWND hWindow); 

	// Direct3D 장치 및 스왑 체인을 생성하는 함수
	void CreateDeviceAndSwapChain(HWND hWindow); 


	// Direct3D 장치 및 스왑 체인을 해제하는 함수
	void ReleaseDeviceAndSwapChain();

	// 프레임 버퍼를 생성하는 함수
	void CreateFrameBuffer();


	// 프레임 버퍼를 해제하는 함수
	void ReleaseFrameBuffer();

	// 래스터라이저 상태를 생성하는 함수
	void CreateRasterizerState();

	// 래스터라이저 상태를 해제하는 함수
	void ReleaseRasterizerState();

	// 렌더러에 사용된 모든 리소스를 해제하는 함수
	void Release();

	// 스왑 체인의 백 버퍼와 프론트 버퍼를 교체하여 화면에 출력
	void SwapBuffer();

	void CreateShader();

	void ReleaseShader();


	void Prepare();

	void PrepareShader();

	virtual void RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices) override;

	ID3D11Buffer* CreateVertexBuffer(VertexSimple* vertices, UINT byteWidth);
	ID3D11Buffer* CreateDynamicVertexBuffer(UINT byteWidth);
	void UpdateDynamicVertexBuffer(ID3D11Buffer* buffer, const void* data, UINT byteWidth);

	void ReleaseVertexBuffer(ID3D11Buffer* vertexBuffer);

	void CreateConstantBuffer();

	void ReleaseConstantBuffer();

	virtual void UpdateConstant(Vector3 Offset, Vector3 scale, float rotation) override;


	template<typename T, typename... Args>
	void CreatePrimitive(Args&&... args)
	{
		const size_t newCapacity = PrimitiveCapacity == 0 ? 1 : PrimitiveCapacity * 2;
		if (PrimitiveCount >= PrimitiveCapacity)
		{
			Primitive** newList = new Primitive * [newCapacity];
			for (size_t i = 0; i < PrimitiveCount; ++i)
			{
				newList[i] = PrimitiveList[i];
			}
			delete[] PrimitiveList;

			PrimitiveList = newList;
			PrimitiveCapacity = newCapacity;
		}

		PrimitiveList[PrimitiveCount++] = new T(args...);
	}


	void ReleaseRandomPrimitive();

	void Tick(float dt);
	

	void Render();

public:
	// Direct3D 11 장치(Device)와 장치 컨텍스트(Device Context) 및 스왑 체인(Swap Chain)을 관리하기 위한 포인터들
	ID3D11Device* Device = nullptr; // GPU와 통신하기 위한 Direct3D 장치
	ID3D11DeviceContext* DeviceContext = nullptr; // GPU 명령 실행을 담당하는 컨텍스트
	IDXGISwapChain* SwapChain = nullptr; // 프레임 버퍼를 교체하는 데 사용되는 스왑 체인

	// 렌더링에 필요한 리소스 및 상태를 관리하기 위한 변수들
	ID3D11Texture2D* FrameBuffer = nullptr; // 화면 출력용 텍스처
	ID3D11RenderTargetView* FrameBufferRTV = nullptr; // 텍스처를 렌더 타겟으로 사용하는 뷰
	ID3D11RasterizerState* RasterizerState = nullptr; // 래스터라이저 상태(컬링, 채우기 모드 등 정의)
	ID3D11Buffer* ConstantBuffer = nullptr; // 쉐이더에 데이터를 전달하기 위한 상수 버퍼

	FLOAT ClearColor[4] = { 0.025f, 0.025f, 0.025f, 1.0f }; // 화면을 초기화(clear)할 때 사용할 색상 (RGBA)
	D3D11_VIEWPORT ViewportInfo; // 렌더링 영역을 정의하는 뷰포트 정보

	ID3D11VertexShader* SimpleVertexShader;
	ID3D11PixelShader* SimplePixelShader;
	ID3D11InputLayout* SimpleInputLayout;
	unsigned int Stride;

	struct Constants
	{
		Vector3 Offset;
		float padding1; 
		Vector3 scale;
		float rotation;
		Matrix3x3 Projection;
	};


	Primitive** PrimitiveList = nullptr;
	size_t PrimitiveCount = 0;
	size_t PrimitiveCapacity = 0;

	StaticCollider LeftWall;
	StaticCollider RightWall;
	StaticCollider TopWall;
	StaticCollider BottomWall;

	Camera mCamera; 
};
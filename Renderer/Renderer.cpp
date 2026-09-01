#include "Renderer.h"

#include <vector>


Renderer::Renderer() 
	: LeftWall(Globals::LEFT_BORDER_POINT, Globals::LEFT_BORDER_NORMAL),
RightWall(Globals::RIGHT_BORDER_POINT, Globals::RIGHT_BORDER_NORMAL),
TopWall(Globals::TOP_BORDER_POINT, Globals::TOP_BORDER_NORMAL),
BottomWall(Globals::BOTTOM_BORDER_POINT, Globals::BOTTOM_BORDER_NORMAL)
{
}

Renderer::~Renderer()
{

}

void Renderer::Create(HWND hWindow) 
{
	// Direct3D 장치 및 스왑 체인 생성
	CreateDeviceAndSwapChain(hWindow);

	// 프레임 버퍼 생성
	CreateFrameBuffer();

	// 래스터라이저 상태 생성
	CreateRasterizerState();

	// 깊이 스텐실 버퍼 및 블렌드 상태는 이 코드에서는 다루지 않음
}

void Renderer::CreateDeviceAndSwapChain(HWND hWindow)
{
	// 지원하는 Direct3D 기능 레벨을 정의
	D3D_FEATURE_LEVEL featurelevels[] = { D3D_FEATURE_LEVEL_11_0 };

	// 스왑 체인 설정 구조체 초기화
	DXGI_SWAP_CHAIN_DESC swapchaindesc = {};
	swapchaindesc.BufferDesc.Width = 0; // 창 크기에 맞게 자동으로 설정
	swapchaindesc.BufferDesc.Height = 0; // 창 크기에 맞게 자동으로 설정
	swapchaindesc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM; // 색상 포맷
	swapchaindesc.SampleDesc.Count = 1; // 멀티 샘플링 비활성화
	swapchaindesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 렌더 타겟으로 사용
	swapchaindesc.BufferCount = 2; // 더블 버퍼링
	swapchaindesc.OutputWindow = hWindow; // 렌더링할 창 핸들
	swapchaindesc.Windowed = TRUE; // 창 모드
	swapchaindesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // 스왑 방식

	// Direct3D 장치와 스왑 체인을 생성
	D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT | D3D11_CREATE_DEVICE_DEBUG,
		featurelevels, ARRAYSIZE(featurelevels), D3D11_SDK_VERSION,
		&swapchaindesc, &SwapChain, &Device, nullptr, &DeviceContext);

	// 생성된 스왑 체인의 정보 가져오기
	SwapChain->GetDesc(&swapchaindesc);

	// 뷰포트 정보 설정
	ViewportInfo = { 0.0f, 0.0f, (float)swapchaindesc.BufferDesc.Width, (float)swapchaindesc.BufferDesc.Height, 0.0f, 1.0f };
}

void Renderer::ReleaseDeviceAndSwapChain()
{
	if (DeviceContext)
	{
		DeviceContext->Flush(); // 남아있는 GPU 명령 실행
	}

	if (SwapChain)
	{
		SwapChain->Release();
		SwapChain = nullptr;
	}

	if (Device)
	{
		Device->Release();
		Device = nullptr;
	}

	if (DeviceContext)
	{
		DeviceContext->Release();
		DeviceContext = nullptr;
	}
}

void Renderer::CreateFrameBuffer() 
{
	// 스왑 체인으로부터 백 버퍼 텍스처 가져오기
	SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&FrameBuffer);

	// 렌더 타겟 뷰 생성
	D3D11_RENDER_TARGET_VIEW_DESC framebufferRTVdesc = {};
	framebufferRTVdesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB; // 색상 포맷
	framebufferRTVdesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D; // 2D 텍스처

	Device->CreateRenderTargetView(FrameBuffer, &framebufferRTVdesc, &FrameBufferRTV);
}

void Renderer::ReleaseFrameBuffer()
{
	if (FrameBuffer)
	{
		FrameBuffer->Release();
		FrameBuffer = nullptr;
	}

	if (FrameBufferRTV)
	{
		FrameBufferRTV->Release();
		FrameBufferRTV = nullptr;
	}
}

void Renderer::CreateRasterizerState()
{
	D3D11_RASTERIZER_DESC rasterizerdesc = {};
	rasterizerdesc.FillMode = D3D11_FILL_SOLID; // 채우기 모드
	rasterizerdesc.CullMode = D3D11_CULL_BACK; // 백 페이스 컬링

	Device->CreateRasterizerState(&rasterizerdesc, &RasterizerState);
}

void Renderer::ReleaseRasterizerState()
{
	if (RasterizerState)
	{
		RasterizerState->Release();
		RasterizerState = nullptr;
	}
}

void Renderer::Release()
{
	for (size_t i = 0; i < PrimitiveCount; ++i) {
		delete PrimitiveList[i];
	}
	delete[] PrimitiveList;
	PrimitiveList = nullptr;
	PrimitiveCount = 0;
	PrimitiveCapacity = 0;

	ReleaseRasterizerState();

	// 렌더 타겟을 초기화
	DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

	ReleaseFrameBuffer();
	ReleaseDeviceAndSwapChain();
}

void Renderer::SwapBuffer()
{
	SwapChain->Present(1, 0); // 1: VSync 활성화
}

void Renderer::CreateShader()
{
	ID3DBlob* vertexshaderCSO;
	ID3DBlob* pixelshaderCSO;

	D3DCompileFromFile(L"ShaderW0.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", 0, 0, &vertexshaderCSO, nullptr);

	Device->CreateVertexShader(vertexshaderCSO->GetBufferPointer(), vertexshaderCSO->GetBufferSize(), nullptr, &SimpleVertexShader);

	D3DCompileFromFile(L"ShaderW0.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", 0, 0, &pixelshaderCSO, nullptr);

	Device->CreatePixelShader(pixelshaderCSO->GetBufferPointer(), pixelshaderCSO->GetBufferSize(), nullptr, &SimplePixelShader);

	D3D11_INPUT_ELEMENT_DESC layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	Device->CreateInputLayout(layout, ARRAYSIZE(layout), vertexshaderCSO->GetBufferPointer(), vertexshaderCSO->GetBufferSize(), &SimpleInputLayout);

	Stride = sizeof(VertexSimple);

	vertexshaderCSO->Release();
	pixelshaderCSO->Release();
}

void Renderer::ReleaseShader()
{
	if (SimpleInputLayout)
	{
		SimpleInputLayout->Release();
		SimpleInputLayout = nullptr;
	}

	if (SimplePixelShader)
	{
		SimplePixelShader->Release();
		SimplePixelShader = nullptr;
	}

	if (SimpleVertexShader)
	{
		SimpleVertexShader->Release();
		SimpleVertexShader = nullptr;
	}
}

void Renderer::Prepare()
{
	DeviceContext->ClearRenderTargetView(FrameBufferRTV, ClearColor);

	DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	DeviceContext->RSSetViewports(1, &ViewportInfo);
	DeviceContext->RSSetState(RasterizerState);

	DeviceContext->OMSetRenderTargets(1, &FrameBufferRTV, nullptr);
	DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
}

void Renderer::PrepareShader()
{
	DeviceContext->VSSetShader(SimpleVertexShader, nullptr, 0);
	DeviceContext->PSSetShader(SimplePixelShader, nullptr, 0);
	DeviceContext->IASetInputLayout(SimpleInputLayout);

	if (ConstantBuffer)
	{
		DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
	}
}

void Renderer::RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices)
{
	UINT offset = 0;
	DeviceContext->IASetVertexBuffers(0, 1, &pBuffer, &Stride, &offset);

	DeviceContext->Draw(numVertices, 0);
}

ID3D11Buffer* Renderer::CreateVertexBuffer(VertexSimple* vertices, UINT byteWidth)
{
	// 2. Create a vertex buffer
	D3D11_BUFFER_DESC vertexbufferdesc = {};
	vertexbufferdesc.ByteWidth = byteWidth;
	vertexbufferdesc.Usage = D3D11_USAGE_IMMUTABLE; // will never be updated 
	vertexbufferdesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexbufferSRD = { vertices };

	ID3D11Buffer* vertexBuffer;

	Device->CreateBuffer(&vertexbufferdesc, &vertexbufferSRD, &vertexBuffer);

	return vertexBuffer;
}

ID3D11Buffer* Renderer::CreateDynamicVertexBuffer(UINT byteWidth)
{
	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = byteWidth;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;

	ID3D11Buffer* buffer = nullptr;
	Device->CreateBuffer(&desc, nullptr, &buffer);
	return buffer;
}

// 2. Map / Unmap을 이용해 정점 데이터 덮어쓰기
void Renderer::UpdateDynamicVertexBuffer(ID3D11Buffer* buffer, const void* data, UINT byteWidth)
{
	if (!buffer || !data) return;

	D3D11_MAPPED_SUBRESOURCE mappedResource = {};
	// D3D11_MAP_WRITE_DISCARD: 기존 버퍼 내용을 버리고 새로 작성 (가장 빠름)
	HRESULT hr = DeviceContext->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (SUCCEEDED(hr))
	{
		memcpy(mappedResource.pData, data, byteWidth);
		DeviceContext->Unmap(buffer, 0);
	}
}

void Renderer::ReleaseVertexBuffer(ID3D11Buffer* vertexBuffer)
{
	vertexBuffer->Release();
}

void Renderer::CreateConstantBuffer()
{
	D3D11_BUFFER_DESC constantbufferdesc = {};
	constantbufferdesc.ByteWidth = sizeof(Constants) + 0xf & 0xfffffff0; // ensure constant buffer size is multiple of 16 bytes
	constantbufferdesc.Usage = D3D11_USAGE_DYNAMIC; // will be updated from CPU every frame
	constantbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	constantbufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	Device->CreateBuffer(&constantbufferdesc, nullptr, &ConstantBuffer);
}

void Renderer::ReleaseConstantBuffer()
{
	if (ConstantBuffer)
	{
		ConstantBuffer->Release();
		ConstantBuffer = nullptr;
	}
}

void Renderer::UpdateConstant(Vector3 Offset, Vector3 scale, float rotation)
{
	if (ConstantBuffer)
	{
		D3D11_MAPPED_SUBRESOURCE constantbufferMSR;

		DeviceContext->Map(ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR); // update constant buffer every frame
		Constants* constants = (Constants*)constantbufferMSR.pData;
		{
			constants->Offset = Offset;
			constants->scale = scale;
			constants->rotation = rotation;
		}

		constants->Projection = Matrix3x3::Orthographic(Globals::LEFT_BORDER, Globals::RIGHT_BORDER, Globals::BOTTOM_BORDER, Globals::TOP_BORDER);
		constants->Projection.Transpose();

		DeviceContext->Unmap(ConstantBuffer, 0);
	}
}

void Renderer::ReleaseRandomPrimitive()
{
	size_t idx = (size_t)Rnd::GetRandom(0, static_cast<int>(PrimitiveCount) - 1);

	delete PrimitiveList[idx];
	for (size_t i = idx; i < PrimitiveCount - 1; ++i)
	{
		PrimitiveList[i] = PrimitiveList[i + 1];
	}
	--PrimitiveCount;
}

void Renderer::Tick(float dt)
{
	for (size_t i = 0; i < PrimitiveCount; ++i)
	{
		if (Globals::ENABLE_GRAVITY)
		{
			const float mass = PrimitiveList[i]->GetRigidBody().GetMass();

			const Vector3 gravityForce =
			{
				0.0f,
				-Globals::GRAVITY_CONSTANT * mass,
				0.0f
			};

			PrimitiveList[i]->GetRigidBody().AddForce(gravityForce);
		}

		PrimitiveList[i]->Tick(dt);
	}

	std::vector<CollisionManifold> Manifolds{};

	for (size_t i = 0; i < PrimitiveCount; ++i)
	{
		for (size_t j = i + 1; j < PrimitiveCount; ++j)
		{
			CollisionManifold manifold;
			// 1. 원 - 원 충돌 
			if(PrimitiveList[i]->GetCollider().GetColliderType() == ColliderType_Sphere && PrimitiveList[j]->GetCollider().GetColliderType() == ColliderType_Sphere)
			{
				if (CollisionDetector::FindCollision(static_cast<SphereCollider&>(PrimitiveList[i]->GetCollider()), static_cast<SphereCollider&>(PrimitiveList[j]->GetCollider()), manifold))
				{
					Manifolds.emplace_back(manifold);
				}
			}

			// 2. 원 - 사각형 충돌 
			if (PrimitiveList[i]->GetCollider().GetColliderType() == ColliderType_Sphere && PrimitiveList[j]->GetCollider().GetColliderType() == ColliderType_Box)
			{
				if (CollisionDetector::FindCollision(static_cast<SphereCollider&>(PrimitiveList[i]->GetCollider()), static_cast<BoxCollider&>(PrimitiveList[j]->GetCollider()), static_cast<BoxCollider&>(PrimitiveList[j]->GetCollider()).GetRigidBody().GetRotation(), manifold))
				{
					Manifolds.emplace_back(manifold);
				}
			}

		}
	}


	if (Globals::BOUND_BALL_TO_SCREEN)
	{
		StaticCollider* Walls[]
		{
			&LeftWall,
			&RightWall,
			&TopWall,
			&BottomWall
		};

		for (size_t PrimitiveIndex{ 0 }; PrimitiveIndex < PrimitiveCount; ++PrimitiveIndex)
		{
			ICollider& collider{ PrimitiveList[PrimitiveIndex]->GetCollider() };


			for (size_t WallIndex{ 0 }; WallIndex < 4; ++WallIndex)
			{
				CollisionManifold Manifold{};

				switch (collider.GetColliderType())
				{
				case ColliderType_Sphere:
				{
					SphereCollider& Circle{ static_cast<SphereCollider&>(collider) };
					if (CollisionDetector::FindCollision(Circle, *Walls[WallIndex], Manifold))
					{
						Manifolds.emplace_back(Manifold);
					}
				}
				break;
				case ColliderType_Box:
				{
					// BoxCollider& Box{ static_cast<BoxCollider&>(collider) };
				}
				default:
					break;
				}
			}
		}
	}


	for (CollisionManifold& manifold : Manifolds)
	{
		CollisionResolver::ResolvePosition(manifold);
		CollisionResolver::PrepareConstraints(manifold);
	}
	

	constexpr int MaxIterations{ 10 };

	for (int iteration = 0; iteration < MaxIterations; ++iteration)
	{
		for (CollisionManifold& manifold : Manifolds)
		{
			CollisionResolver::ResolveRestitution(manifold);
			CollisionResolver::ResolveFriction(manifold);
		}
	}



	


}

void Renderer::Render()
{
	Prepare();
	PrepareShader();
	for (size_t i = 0; i < PrimitiveCount; ++i)
	{
		PrimitiveList[i]->Render(*this);
	}
}
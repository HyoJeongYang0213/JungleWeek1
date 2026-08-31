#include <windows.h>
// 여기에 아래 코드를 추가 합니다.

// D3D 사용에 필요한 라이브러리들을 링크합니다.
#pragma comment(lib, "user32")
#pragma comment(lib, "d3d11")
#pragma comment(lib, "d3dcompiler")

// D3D 사용에 필요한 헤더파일들을 포함합니다.
#include <d3d11.h>
#include <d3dcompiler.h>

#include "ImGui/imgui.h"
#include "ImGui/imgui_internal.h"
#include "ImGui/imgui_impl_dx11.h"
#include "imGui/imgui_impl_win32.h"



// 1. Define the triangle vertices
struct FVertexSimple
{
	float x, y, z;    // Position
	float r, g, b, a; // Color
};

struct FVector3
{
	float x, y, z;
	FVector3(float _x = 0, float _y = 0, float _z = 0) : x(_x), y(_y), z(_z) {}

	float SquaredLength() const { return x * x + y * y + z * z; }

	float Length() const { return sqrtf(SquaredLength()); }

	float Dot(const FVector3& other) const { return x * other.x + y * other.y + z * other.z; }
	float Cross(const FVector3& other) const { return x * other.y - y * other.x; }

	FVector3 operator+(const FVector3& other) const { return FVector3(x + other.x, y + other.y, z + other.z); }
	FVector3& operator+=(const FVector3& other) { x += other.x; y += other.y; z += other.z; return *this; }

	FVector3 operator-(const FVector3& other) const { return FVector3(x - other.x, y - other.y, z - other.z); }
	FVector3& operator-=(const FVector3& other) { x -= other.x; y -= other.y; z -= other.z; return *this; }

	FVector3 operator*(float scalar) const { return FVector3(x * scalar, y * scalar, z * scalar); }
	FVector3 operator/(float scalar) const { return FVector3(x / scalar, y / scalar, z / scalar); }

	FVector3 Normalize() const
	{
		float len = Length();
		if (len > 0)
			return FVector3(x / len, y / len, z / len);
		return FVector3(0, 0, 0);
	}

};

class IURenderer {
public:
	virtual ~IURenderer() = default;

	virtual void UpdateConstant(FVector3 Offset, float scale, float rotation) = 0;
	virtual void RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices) = 0;
};

#include "Sphere.h"

// 삼각형을 하드 코딩
FVertexSimple triangle_vertices[] =
{
	{  0.0f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f }, // Top vertex (red)
	{  1.0f, -1.0f, 0.0f,  0.0f, 1.0f, 0.0f, 1.0f }, // Bottom-right vertex (green)
	{ -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f }  // Bottom-left vertex (blue)
};

namespace Globals {
	const float leftBorder = -1.0f;
	const float rightBorder = 1.0f;
	const float topBorder = -1.0f;
	const float bottomBorder = 1.0f;
	const float sphereRadius = 1.0f;

	bool bBoundBallToScreen = true;
	bool bPinballMovement = true;
	bool bEnableGravity = true;

	float fFrictionCoefficient = 0.5f;
	float fRestitutionCoefficient = 1.0f;
	float fGravityStrength = 9.8f;
	float fGravityAngle = 1.5707963f;

	bool bEnableRotation = true;

	int TargetPrimitiveCount = 0;
}

namespace Rnd {
	float GetRandomFloat(float min, float max)
	{
		float random = ((float)rand()) / (float)RAND_MAX; // 0.0 ~ 1.0
		float range = max - min;
		return (random * range) + min; // min ~ max
	}

	int GetRandomInt(int min, int max) {
		return rand() % (max - min + 1) + min; // min ~ max
	}
}


class UPrimitive
{
public:
	virtual ~UPrimitive() = default;

	virtual void Tick(float t) = 0;
	virtual void Render(class IURenderer& renderer) = 0;

	virtual class UCollider& GetCollider() = 0;
	virtual class URigidBody& GetRigidBody() = 0;
};

class URigidBody
{
private:
	FVector3& Position;
	FVector3& Velocity;
	float& Mass;

	FVector3 AccumulatedForce{};
	float AccumulatedTorque{};

	float AngularVelocity{};
	float Rotation{};
	float MomentOfInertia{};

public:
	URigidBody(FVector3& Position, FVector3& Velocity, float& Mass, float MomentOfInertia) :
		Position(Position), Velocity(Velocity), Mass(Mass), MomentOfInertia(MomentOfInertia) {
	}

	void SetMomentOfInertia(float moment)
	{
		MomentOfInertia = moment;
	}

	void AddForce(const FVector3& Force)
	{
		AccumulatedForce += Force;
	}

	void AddTorque(float Torque)
	{
		AccumulatedTorque += Torque;
	}

	void Integrate(float dt)
	{
		const float InverseMass{ GetInverseMass() };
		const float InverseMomentOfInertia{ GetInverseMomentOfInertia() };

		Velocity.x += AccumulatedForce.x * InverseMass * dt;
		Velocity.y += AccumulatedForce.y * InverseMass * dt;

		if (Globals::bEnableRotation) {
			AngularVelocity += AccumulatedTorque * InverseMomentOfInertia * dt;
			Rotation += AngularVelocity * dt;
		}

		Position.x += Velocity.x * dt;
		Position.y += Velocity.y * dt;

		AccumulatedForce = {};
		AccumulatedTorque = {};
	}

	void ApplyImpulse(const FVector3& Impulse, const FVector3& ContactPoint)
	{
		const FVector3 Offset{ ContactPoint.x - Position.x, ContactPoint.y - Position.y, ContactPoint.z - Position.z };
		const float InverseMass{ GetInverseMass() };
		const float InverseMomentOfInertia{ GetInverseMomentOfInertia() };

		Velocity.x += Impulse.x * InverseMass;
		Velocity.y += Impulse.y * InverseMass;

		AngularVelocity += Offset.Cross(Impulse) * InverseMomentOfInertia;
	}

	void ApplyPositionCorrection(const FVector3& Correction)
	{
		Position += Correction * GetInverseMass();
	}

	FVector3 GetVelocityAtPoint(const FVector3& ContactPoint) const
	{
		const FVector3 Offset{ ContactPoint - Position };
		const FVector3 RotationalVelocity{ -AngularVelocity * Offset.y, AngularVelocity * Offset.x, 0.0f };

		return Velocity + RotationalVelocity;
	}

	float GetAngularVelocity() const
	{
		return AngularVelocity;
	}

	float GetRotation() const
	{
		return Rotation;
	}

	float GetInverseMass() const
	{
		return Mass > 0.0f ? 1.0f / Mass : 0.0f;
	}

	float GetInverseMomentOfInertia() const
	{
		return MomentOfInertia > 0.0f ? 1.0f / MomentOfInertia : 0.0f;
	}

	const FVector3& GetPosition() const
	{
		return Position;
	}

	const FVector3& GetVelocity() const
	{
		return Velocity;
	}

	float GetMass() const
	{
		return Mass;
	}

};

class UCollider
{
public:
	URigidBody& RigidBody;
	float& Radius;

public:
	UCollider(URigidBody& r, float& rad) :
		RigidBody(r), Radius(rad) {
	}
	~UCollider() = default;

	URigidBody& GetRigidBody() { return RigidBody; }
	const URigidBody& GetRigidBody() const { return RigidBody; }

	const FVector3& GetCenter() const { return RigidBody.GetPosition(); }
	float GetRadius() const { return Radius; }
};

class UStaticCollider
{
public:
	FVector3 Position;
	FVector3 Velocity;
	float Mass;

	URigidBody RigidBody;
	FVector3 Normal;

public:
	UStaticCollider(const FVector3& pos, const FVector3& norm) :
		Position(pos), Velocity{}, Mass{}, RigidBody(Position, Velocity, Mass, 0.0f), Normal(norm.Normalize()) {
	}

	URigidBody& GetRigidBody() { return RigidBody; }
	const URigidBody& GetRigidBody() const { return RigidBody; }

	const FVector3& GetPosition() const { return Position; }
	const FVector3& GetNormal() const { return Normal; }
};

struct FCollisionManifold
{
	URigidBody* ColliderA{};
	URigidBody* ColliderB{};

	FVector3 Normal{};
	FVector3 ContactPoint{};

	float Penetration{};
};


class UCollisionDetector
{
public:
	static bool FindCollision(UCollider& Ca, UCollider& Cb, FCollisionManifold& OutManifold)
	{
		FVector3 delta = Cb.GetCenter() - Ca.GetCenter();
		float distanceSquared = delta.SquaredLength();
		float radiusSum = Ca.GetRadius() + Cb.GetRadius();

		if (distanceSquared < radiusSum * radiusSum)
		{
			float distance = sqrtf(distanceSquared);

			OutManifold.ColliderA = &Ca.GetRigidBody();
			OutManifold.ColliderB = &Cb.GetRigidBody();

			OutManifold.Normal = (distance > 0.0001f) ? delta / distance : FVector3(1.0f, 0.0f, 0.0f);
			OutManifold.ContactPoint = Ca.GetCenter() + OutManifold.Normal * Ca.GetRadius();
			OutManifold.Penetration = radiusSum - distance;

			return true;
		}

		return false;
	}

	static bool FindCollision(UCollider& c, UStaticCollider& s, FCollisionManifold& OutManifold)
	{
		const FVector3 CircleToPlanePoint{ c.GetCenter() - s.GetPosition() };
		const float SignedDistance{ CircleToPlanePoint.Dot(s.GetNormal()) };

		if (SignedDistance >= c.GetRadius())
		{
			return false;
		}

		OutManifold.ColliderA = &c.GetRigidBody();
		OutManifold.ColliderB = &s.GetRigidBody();
		OutManifold.Normal = s.GetNormal() * -1.0f;
		OutManifold.Penetration = c.GetRadius() - SignedDistance;
		OutManifold.ContactPoint = c.GetCenter() - s.GetNormal() * SignedDistance;

		return true;
	}
};

class UCollisionResolver
{
public:
	static void ResolveCollision(FCollisionManifold& Manifold)
	{
		URigidBody& A = *Manifold.ColliderA;
		URigidBody& B = *Manifold.ColliderB;

		const float InverseMassA = A.GetInverseMass();
		const float InverseMassB = B.GetInverseMass();
		const float InverseMassSum = InverseMassA + InverseMassB;

		if (InverseMassSum <= 0.0f)
		{
			return;
		}

		const float Slop{ 0.001f };
		const float CorrectionPercent{ 0.8f };
		const float CorrectionMagnitude = ((Manifold.Penetration > Slop) ? Manifold.Penetration - Slop : 0.0f) / InverseMassSum * CorrectionPercent;
		const FVector3 Correction = Manifold.Normal * CorrectionMagnitude;

		A.ApplyPositionCorrection(Correction * -1.f);
		B.ApplyPositionCorrection(Correction);

		const FVector3 VelocityA = A.GetVelocityAtPoint(Manifold.ContactPoint);
		const FVector3 VelocityB = B.GetVelocityAtPoint(Manifold.ContactPoint);
		const FVector3 RelativeVelocity = VelocityB - VelocityA;
		const float VelocityAlongNormal = RelativeVelocity.Dot(Manifold.Normal);

		if (VelocityAlongNormal >= 0.0f)
		{
			return;
		}

		const FVector3 OffsetA = Manifold.ContactPoint - A.GetPosition();
		const FVector3 OffsetB = Manifold.ContactPoint - B.GetPosition();

		const float AngularTermA = OffsetA.Cross(Manifold.Normal) * OffsetA.Cross(Manifold.Normal) * A.GetInverseMomentOfInertia();
		const float AngularTermB = OffsetB.Cross(Manifold.Normal) * OffsetB.Cross(Manifold.Normal) * B.GetInverseMomentOfInertia();
		const float Denominator = InverseMassSum + AngularTermA + AngularTermB;

		const float Restitution{ Globals::fRestitutionCoefficient };
		const float ImpulseMagnitude = -(1.0f + Restitution) * VelocityAlongNormal / Denominator;
		const FVector3 Impulse = Manifold.Normal * ImpulseMagnitude;

		A.ApplyImpulse(Impulse * -1.f, Manifold.ContactPoint);
		B.ApplyImpulse(Impulse, Manifold.ContactPoint);


		const float StaticFriction{ Globals::fFrictionCoefficient };
		const float DynamicFriction{ StaticFriction * 0.8f };

		const FVector3 FrictionVelocityA{ A.GetVelocityAtPoint(Manifold.ContactPoint) };
		const FVector3 FrictionVelocityB{ B.GetVelocityAtPoint(Manifold.ContactPoint) };
		const FVector3 FrictionRelativeVelocity{ FrictionVelocityB - FrictionVelocityA };

		const float TangentVelocityAlongNormal{ FrictionRelativeVelocity.Dot(Manifold.Normal) };
		const FVector3 TangentVelocity{ FrictionRelativeVelocity - Manifold.Normal * TangentVelocityAlongNormal };
		const float TangentVelocitySquaredLength{ TangentVelocity.SquaredLength() };

		if (TangentVelocitySquaredLength > 0.000001f)
		{
			const FVector3 Tangent{ TangentVelocity.Normalize() };

			const float OffsetACrossTangent{ OffsetA.Cross(Tangent) };
			const float OffsetBCrossTangent{ OffsetB.Cross(Tangent) };

			const float TangentDenominator{ InverseMassSum + OffsetACrossTangent * OffsetACrossTangent * A.GetInverseMomentOfInertia() + OffsetBCrossTangent * OffsetBCrossTangent * B.GetInverseMomentOfInertia() };
			const float UnclampedFrictionImpulseMagnitude{ -FrictionRelativeVelocity.Dot(Tangent) / TangentDenominator };
			const float StaticFrictionLimit{ ImpulseMagnitude * StaticFriction };
			const float FrictionImpulseMagnitude{ fabsf(UnclampedFrictionImpulseMagnitude) <= StaticFrictionLimit ? UnclampedFrictionImpulseMagnitude : (UnclampedFrictionImpulseMagnitude < 0.0f ? -1.0f : 1.0f) * ImpulseMagnitude * DynamicFriction };

			const FVector3 FrictionImpulse{ Tangent * FrictionImpulseMagnitude };

			A.ApplyImpulse(FrictionImpulse * -1.0f, Manifold.ContactPoint);
			B.ApplyImpulse(FrictionImpulse, Manifold.ContactPoint);
		}

	}
};


class UBall : public UPrimitive
{
public:
	// 클래스 이름과, 아래 다섯개의 변수 이름은 변경하지 않습니다.
	FVector3 Location;
	FVector3 Velocity;
	float Radius;
	float Mass;
	static int TotalNumBalls;

	URigidBody RigidBody;
	UCollider Collider;

	ID3D11Buffer* VertexBuffer;
	UINT NumVertices;
public:
	UBall(ID3D11Buffer* vb, UINT n)
		: Location{ Rnd::GetRandomFloat(-1.0f, 1.0f), Rnd::GetRandomFloat(-1.0f, 1.0f), 0.0f },
		Velocity{ Rnd::GetRandomFloat(-3.0f, 3.0f), Rnd::GetRandomFloat(-3.0f, 3.0f), 0.0f },
		Radius{ Rnd::GetRandomFloat(0.05f, 0.15f) },
		Mass{ 0.3f * Radius * Radius * 3.14159f },
		RigidBody{ Location, Velocity, Mass, 0.5f * Mass * Radius * Radius },
		Collider{ RigidBody, Radius },
		VertexBuffer{ vb }, NumVertices{ n } {
		TotalNumBalls++;
	}

	virtual ~UBall() override {
		TotalNumBalls--;
	}


	virtual void Tick(float t) override {
		if (Globals::bBoundBallToScreen)
		{
			float renderRadius = Radius;
			if (Location.x < Globals::leftBorder + renderRadius)
			{
				Location.x = Globals::leftBorder + renderRadius;
			}
			if (Location.x > Globals::rightBorder - renderRadius)
			{
				Location.x = Globals::rightBorder - renderRadius;
			}
			if (Location.y < Globals::topBorder + renderRadius)
			{
				Location.y = Globals::topBorder + renderRadius;
			}
			if (Location.y > Globals::bottomBorder - renderRadius)
			{
				Location.y = Globals::bottomBorder - renderRadius;
			}
		}




		// 핀볼 움직임이 켜져 있다면
		if (Globals::bPinballMovement)
		{
			RigidBody.Integrate(t);
		}

	}

	virtual UCollider& GetCollider() override
	{
		return Collider;
	}

	virtual URigidBody& GetRigidBody() override
	{
		return RigidBody;
	}

	virtual void Render(IURenderer& renderer) override
	{

		renderer.UpdateConstant(Location, Radius, RigidBody.GetRotation());

		renderer.RenderPrimitive(VertexBuffer, NumVertices);
	}
};
int UBall::TotalNumBalls = 0;




class URenderer : public IURenderer
{
#pragma region members  
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

	struct FConstants
	{
		FVector3 Offset;
		float scale;
		float rotation;
		FVector3 padding;
	};


	UPrimitive** PrimitiveList = nullptr;
	size_t PrimitiveCount = 0;
	size_t PrimitiveCapacity = 0;

	UStaticCollider LeftWall;
	UStaticCollider RightWall;
	UStaticCollider TopWall;
	UStaticCollider BottomWall;

#pragma endregion
public:
	URenderer()
		: LeftWall(FVector3(Globals::leftBorder, 0.0f, 0.0f), FVector3(1.0f, 0.0f, 0.0f)),
		RightWall(FVector3(Globals::rightBorder, 0.0f, 0.0f), FVector3(-1.0f, 0.0f, 0.0f)),
		TopWall(FVector3(0.0f, Globals::topBorder, 0.0f), FVector3(0.0f, 1.0f, 0.0f)),
		BottomWall(FVector3(0.0f, Globals::bottomBorder, 0.0f), FVector3(0.0f, -1.0f, 0.0f))
	{
	}

	// 렌더러 초기화 함수
	void Create(HWND hWindow)
	{
		// Direct3D 장치 및 스왑 체인 생성
		CreateDeviceAndSwapChain(hWindow);

		// 프레임 버퍼 생성
		CreateFrameBuffer();

		// 래스터라이저 상태 생성
		CreateRasterizerState();

		// 깊이 스텐실 버퍼 및 블렌드 상태는 이 코드에서는 다루지 않음
	}

	// Direct3D 장치 및 스왑 체인을 생성하는 함수
	void CreateDeviceAndSwapChain(HWND hWindow)
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

	// Direct3D 장치 및 스왑 체인을 해제하는 함수
	void ReleaseDeviceAndSwapChain()
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

	// 프레임 버퍼를 생성하는 함수
	void CreateFrameBuffer()
	{
		// 스왑 체인으로부터 백 버퍼 텍스처 가져오기
		SwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&FrameBuffer);

		// 렌더 타겟 뷰 생성
		D3D11_RENDER_TARGET_VIEW_DESC framebufferRTVdesc = {};
		framebufferRTVdesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM_SRGB; // 색상 포맷
		framebufferRTVdesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D; // 2D 텍스처

		Device->CreateRenderTargetView(FrameBuffer, &framebufferRTVdesc, &FrameBufferRTV);
	}

	// 프레임 버퍼를 해제하는 함수
	void ReleaseFrameBuffer()
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

	// 래스터라이저 상태를 생성하는 함수
	void CreateRasterizerState()
	{
		D3D11_RASTERIZER_DESC rasterizerdesc = {};
		rasterizerdesc.FillMode = D3D11_FILL_SOLID; // 채우기 모드
		rasterizerdesc.CullMode = D3D11_CULL_BACK; // 백 페이스 컬링

		Device->CreateRasterizerState(&rasterizerdesc, &RasterizerState);
	}

	// 래스터라이저 상태를 해제하는 함수
	void ReleaseRasterizerState()
	{
		if (RasterizerState)
		{
			RasterizerState->Release();
			RasterizerState = nullptr;
		}
	}

	// 렌더러에 사용된 모든 리소스를 해제하는 함수
	void Release()
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

	// 스왑 체인의 백 버퍼와 프론트 버퍼를 교체하여 화면에 출력
	void SwapBuffer()
	{
		SwapChain->Present(1, 0); // 1: VSync 활성화
	}

	void CreateShader()
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

		Stride = sizeof(FVertexSimple);

		vertexshaderCSO->Release();
		pixelshaderCSO->Release();
	}

	void ReleaseShader()
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

	void Prepare()
	{
		DeviceContext->ClearRenderTargetView(FrameBufferRTV, ClearColor);

		DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		DeviceContext->RSSetViewports(1, &ViewportInfo);
		DeviceContext->RSSetState(RasterizerState);

		DeviceContext->OMSetRenderTargets(1, &FrameBufferRTV, nullptr);
		DeviceContext->OMSetBlendState(nullptr, nullptr, 0xffffffff);
	}

	void PrepareShader()
	{
		DeviceContext->VSSetShader(SimpleVertexShader, nullptr, 0);
		DeviceContext->PSSetShader(SimplePixelShader, nullptr, 0);
		DeviceContext->IASetInputLayout(SimpleInputLayout);

		if (ConstantBuffer)
		{
			DeviceContext->VSSetConstantBuffers(0, 1, &ConstantBuffer);
		}
	}

	virtual void RenderPrimitive(ID3D11Buffer* pBuffer, UINT numVertices) override
	{
		UINT offset = 0;
		DeviceContext->IASetVertexBuffers(0, 1, &pBuffer, &Stride, &offset);

		DeviceContext->Draw(numVertices, 0);
	}

	ID3D11Buffer* CreateVertexBuffer(FVertexSimple* vertices, UINT byteWidth)
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

	void ReleaseVertexBuffer(ID3D11Buffer* vertexBuffer)
	{
		vertexBuffer->Release();
	}

	void CreateConstantBuffer()
	{
		D3D11_BUFFER_DESC constantbufferdesc = {};
		constantbufferdesc.ByteWidth = sizeof(FConstants) + 0xf & 0xfffffff0; // ensure constant buffer size is multiple of 16 bytes
		constantbufferdesc.Usage = D3D11_USAGE_DYNAMIC; // will be updated from CPU every frame
		constantbufferdesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		constantbufferdesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

		Device->CreateBuffer(&constantbufferdesc, nullptr, &ConstantBuffer);
	}

	void ReleaseConstantBuffer()
	{
		if (ConstantBuffer)
		{
			ConstantBuffer->Release();
			ConstantBuffer = nullptr;
		}
	}

	virtual void UpdateConstant(FVector3 Offset, float scale, float rotation) override
	{
		if (ConstantBuffer)
		{
			D3D11_MAPPED_SUBRESOURCE constantbufferMSR;

			DeviceContext->Map(ConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &constantbufferMSR); // update constant buffer every frame
			FConstants* constants = (FConstants*)constantbufferMSR.pData;
			{
				constants->Offset = Offset;
				constants->scale = scale;
				constants->rotation = rotation;
			}
			DeviceContext->Unmap(ConstantBuffer, 0);
		}
	}


	template<typename T, typename... Args>
	void CreatePrimitive(Args&&... args)
	{
		const size_t newCapacity = PrimitiveCapacity == 0 ? 1 : PrimitiveCapacity * 2;
		if (PrimitiveCount >= PrimitiveCapacity)
		{
			UPrimitive** newList = new UPrimitive * [newCapacity];
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

	void ReleaseRandomPrimitive() {
		size_t idx = (size_t)Rnd::GetRandomInt(0, static_cast<int>(PrimitiveCount) - 1);

		delete PrimitiveList[idx];
		for (size_t i = idx; i < PrimitiveCount - 1; ++i)
		{
			PrimitiveList[i] = PrimitiveList[i + 1];
		}
		--PrimitiveCount;
	}

	void Tick(float dt)
	{
		for (size_t i = 0; i < PrimitiveCount; ++i)
		{
			if (Globals::bEnableGravity)
			{
				const FVector3& gravityDirection = FVector3(cosf(Globals::fGravityAngle), sinf(Globals::fGravityAngle), 0.0f);
				const FVector3 gravityForce = gravityDirection * (-Globals::fGravityStrength * PrimitiveList[i]->GetRigidBody().GetMass());

				PrimitiveList[i]->GetRigidBody().AddForce(gravityForce);
			}
			PrimitiveList[i]->Tick(dt);
		}

		for (size_t i = 0; i < PrimitiveCount; ++i)
		{
			for (size_t j = i + 1; j < PrimitiveCount; ++j)
			{
				FCollisionManifold manifold;
				if (UCollisionDetector::FindCollision(PrimitiveList[i]->GetCollider(), PrimitiveList[j]->GetCollider(), manifold))
				{
					UCollisionResolver::ResolveCollision(manifold);
				}
			}
		}


		if (Globals::bBoundBallToScreen)
		{
			UStaticCollider* Walls[]
			{
				&LeftWall,
				&RightWall,
				&TopWall,
				&BottomWall
			};

			for (size_t PrimitiveIndex{ 0 }; PrimitiveIndex < PrimitiveCount; ++PrimitiveIndex)
			{
				UCollider& Circle{ PrimitiveList[PrimitiveIndex]->GetCollider() };

				for (size_t WallIndex{ 0 }; WallIndex < 4; ++WallIndex)
				{
					FCollisionManifold Manifold{};

					if (UCollisionDetector::FindCollision(Circle, *Walls[WallIndex], Manifold))
					{
						UCollisionResolver::ResolveCollision(Manifold);
					}
				}
			}
		}


	}

	void Render() {
		Prepare();
		PrepareShader();
		for (size_t i = 0; i < PrimitiveCount; ++i)
		{
			PrimitiveList[i]->Render(*this);
		}
	}

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
		CW_USEDEFAULT, CW_USEDEFAULT, 1024, 1024,
		nullptr, nullptr, hInstance, nullptr);

	srand((UINT)GetTickCount64());

	bool bIsExit = false;

	// 각종 생성하는 코드를 여기에 추가합니다.
	URenderer renderer;
	renderer.Create(hWnd);
	renderer.CreateShader();
	renderer.CreateConstantBuffer();

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	ImGui_ImplWin32_Init((void*)hWnd);
	ImGui_ImplDX11_Init(renderer.Device, renderer.DeviceContext);

	UINT numVerticesTriangle = sizeof(triangle_vertices) / sizeof(FVertexSimple);
	UINT numVerticesSphere = sizeof(sphere_vertices) / sizeof(FVertexSimple);

	ID3D11Buffer* vertexBufferTriangle = renderer.CreateVertexBuffer(triangle_vertices, sizeof(triangle_vertices));
	ID3D11Buffer* vertexBufferSphere = renderer.CreateVertexBuffer(sphere_vertices, sizeof(sphere_vertices));

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


		////////////////////////////////////////////
		// 매번 실행되는 코드를 여기에 추가합니다.
		if (Globals::TargetPrimitiveCount >= 0)
		{
			int deltaPrimitives = Globals::TargetPrimitiveCount - (int)UBall::TotalNumBalls;
			if (deltaPrimitives > 0)
			{
				for (int i = 0; i < deltaPrimitives; ++i)
				{
					renderer.CreatePrimitive<UBall>(vertexBufferSphere, numVerticesSphere);
				}
			}
			else if (deltaPrimitives < 0)
			{
				for (int i = 0; i < -deltaPrimitives; ++i)
				{
					renderer.ReleaseRandomPrimitive();
				}
			}
		}
		renderer.Tick(static_cast<float>(elapsedTime));
		renderer.Render();



		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// 이후 ImGui UI 컨트롤 추가는 ImGui::NewFrame()과 ImGui::Render() 사이인 여기에 위치합니다.
		ImGui::Begin("Jungle Property Window");

		ImGui::Checkbox("Bound Ball To Screen", &Globals::bBoundBallToScreen);
		ImGui::Checkbox("Pinball Movement", &Globals::bPinballMovement);
		ImGui::Checkbox("Gravity", &Globals::bEnableGravity);
		ImGui::Checkbox("Rotation", &Globals::bEnableRotation);
		ImGui::SliderFloat("Friction", &Globals::fFrictionCoefficient, 0.0f, 1.0f);
		ImGui::SliderFloat("Restitution", &Globals::fRestitutionCoefficient, 0.0f, 1.0f);
		ImGui::SliderFloat("Gravity Strength", &Globals::fGravityStrength, 0.0f, 20.0f);
		ImGui::SliderAngle("Gravity Angle", &Globals::fGravityAngle, -180.f, 180.0f);

		ImGui::InputInt("Total Balls", (int*)&Globals::TargetPrimitiveCount);

		ImGui::End();

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

	renderer.ReleaseConstantBuffer();
	renderer.ReleaseShader();
	renderer.Release();

	return 0;
}
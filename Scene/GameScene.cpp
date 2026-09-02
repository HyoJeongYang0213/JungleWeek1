#include "GameScene.h"

#include "../Physics/PhysicsGlobals.hpp"
#include "../Physics/RigidBody.h"
#include "../Physics/CollisionManifold.hpp"
#include "../Physics/CollisionDetector.h"
#include "../Physics/CollisionResolver.h"

#include "../Renderer/Renderer.h"

#include "../Resource/vertexSimple.hpp"
#include "../Resource/Primitive.hpp"
#include "../Resource/Square.hpp"
#include "../Resource/Sphere.h"
#include "../Resource/PolygonGeometry.hpp"

#include "../UI/GameButtonUI.hpp"

#include "../Map/TextureLoader.hpp"
#include "../Map/WaterGlobals.hpp"

#include "../Player/Ball.h"
#include "../Player/PlayerGlobals.hpp"

#include "../ImGui/imgui.h"
#include "../ImGui/imgui_internal.h"
#include "../ImGui/imgui_impl_dx11.h"
#include "../ImGui/imgui_impl_win32.h"
#include "../Renderer/WindowGlobals.hpp"
#include "../Map/Water.h"


GameScene::GameScene(IRenderer& renderer) 
{
	VertexSimple triangle_vertices[] =
	{
		{  0.0f,  1.0f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f }, // Top vertex (red)
		{  1.0f, -1.0f, 0.0f,  0.0f, 1.0f, 0.0f, 1.0f }, // Bottom-right vertex (green)
		{ -1.0f, -1.0f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f }  // Bottom-left vertex (blue)
	};

	Renderer& concreteRenderer = static_cast<Renderer&>(renderer);

	ID3D11Buffer* vertexBufferTriangle = concreteRenderer.CreateVertexBuffer(triangle_vertices, sizeof(triangle_vertices));
	ID3D11Buffer* vertexBufferSphere = concreteRenderer.CreateVertexBuffer(sphere_vertices, sizeof(sphere_vertices));
	ID3D11Buffer* vertexBufferCube = concreteRenderer.CreateVertexBuffer(cube_vertices, sizeof(cube_vertices));
	

	mWater = std::make_unique<Water>(concreteRenderer, 0.0f);


	std::vector<std::tuple<ID3D11Buffer*, UINT, std::vector<Vector3>>> polygonVertexBuffers{};

	{
		auto AddPolygonVertexBuffer = [&](auto& vertices, const auto& positions)
			{
				polygonVertexBuffers.emplace_back(
					concreteRenderer.CreateVertexBuffer(
						vertices,
						sizeof(vertices)
					),
					static_cast<UINT>(std::size(vertices)),
					positions
				);
			};

		// Convex
		AddPolygonVertexBuffer(convex_triangle_vertices, convex_triangle_positions);
		AddPolygonVertexBuffer(convex_quad_vertices, convex_quad_positions);
		AddPolygonVertexBuffer(convex_trapezoid_vertices, convex_trapezoid_positions);
		AddPolygonVertexBuffer(convex_pentagon_vertices, convex_pentagon_positions);
		AddPolygonVertexBuffer(convex_hexagon_vertices, convex_hexagon_positions);
		AddPolygonVertexBuffer(convex_octagon_vertices, convex_octagon_positions);
		AddPolygonVertexBuffer(convex_dodecagon_vertices, convex_dodecagon_positions);

		// Concave
		AddPolygonVertexBuffer(concave_arrow_vertices, concave_arrow_positions);
		AddPolygonVertexBuffer(concave_l_vertices, concave_l_positions);
		AddPolygonVertexBuffer(concave_u_vertices, concave_u_positions);
		AddPolygonVertexBuffer(concave_plus_vertices, concave_plus_positions);
		AddPolygonVertexBuffer(concave_c_vertices, concave_c_positions);
		AddPolygonVertexBuffer(concave_star_vertices, concave_star_positions);
		AddPolygonVertexBuffer(concave_lightning_vertices, concave_lightning_positions);
		AddPolygonVertexBuffer(concave_comb_vertices, concave_comb_positions);
		AddPolygonVertexBuffer(concave_spiral_vertices, concave_spiral_positions);
		AddPolygonVertexBuffer(concave_star16_vertices, concave_star16_positions);
	}


	ID3DBlob* VertexShaderBlob = nullptr, * psBlob = nullptr;

	D3DCompileFromFile(L"Resource/Shader/TextureShader.hlsl", nullptr, nullptr, "mainVS", "vs_5_0", 0, 0, &VertexShaderBlob, nullptr);
	concreteRenderer.Device->CreateVertexShader(VertexShaderBlob->GetBufferPointer(), VertexShaderBlob->GetBufferSize(), nullptr, &mTextureVertexShader);

	D3DCompileFromFile(L"Resource/Shader/TextureShader.hlsl", nullptr, nullptr, "mainPS", "ps_5_0", 0, 0, &psBlob, nullptr);
	concreteRenderer.Device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &mTexturePixelShader);

	D3D11_INPUT_ELEMENT_DESC TextureLayoutDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	concreteRenderer.Device->CreateInputLayout(TextureLayoutDesc, 2, VertexShaderBlob->GetBufferPointer(), VertexShaderBlob->GetBufferSize(), &mTextureLayout);
	VertexShaderBlob->Release();
	psBlob->Release();


	ID3D11ShaderResourceView* ShaderResourceViewGround = TextureLoader::CreateTextureFromFile(concreteRenderer.Device, L"Asset/Stage_Ground.png");
	ID3D11ShaderResourceView* ShaderResourceViewA = TextureLoader::CreateTextureFromFile(concreteRenderer.Device, L"Asset/Stage_A.png");
	ID3D11ShaderResourceView* ShaderResourceViewB = TextureLoader::CreateTextureFromFile(concreteRenderer.Device, L"Asset/Stage_B.png");
	ID3D11ShaderResourceView* ShaderResourceViewC = TextureLoader::CreateTextureFromFile(concreteRenderer.Device, L"Asset/Stage_C.png");

	mSamplerState = TextureLoader::CreateSamplerState(concreteRenderer.Device);

	mBackGround.Init(concreteRenderer, ShaderResourceViewGround, { ShaderResourceViewA, ShaderResourceViewB, ShaderResourceViewC });
	mPlatformManager.Init(concreteRenderer, "Asset/stage1_collision_mask_1536x3000.png", { "Asset/stage1_collision_mask_1536x3000.png","Asset/stage1_collision_mask_1536x3000_2.png" });
	mSRVPlatform = TextureLoader::CreateTextureFromFile(concreteRenderer.Device, L"Asset/Platform.png");

	mCamera.SetPosition(Vector3{ 0.0f, 0.0f, 0.0f });

	GameScene::CreatePrimitive<Ball>(vertexBufferSphere, static_cast<UINT>(sizeof(sphere_vertices) / sizeof(sphere_vertices[0])));

	PlayerGlobals::PLAYERLOCATION = static_cast<Ball*>(mPrimitives[0].get())->GetLocation();
	PlayerGlobals::PLAYERBALL = static_cast<Ball*>(mPrimitives[0].get());

	mInput.RegisterDragCallback([&](POINT targetPos)
		{
			Ball* player = static_cast<Ball*>(mPrimitives[0].get());
			Vector3 ReleasePoint{ Vector3(static_cast<float>(targetPos.x), static_cast<float>(targetPos.y), 0.0f) };
			player->GetRigidBody().ApplyImpulse((player->GetLocation() - ReleasePoint) * PlayerGlobals::PLAYER_DRAG_IMPULSE_MULTIPLIER, player->GetLocation());
		});


	mWater->Start(); 
}

GameScene::~GameScene()
{
	if (mSamplerState)
	{
		mSamplerState->Release();
		mSamplerState = nullptr;
	}

	if (mSRVPlatform)
	{
		mSRVPlatform->Release();
		mSRVPlatform = nullptr;
	}

	if (mTextureLayout)
	{
		mTextureLayout->Release();
		mTextureLayout = nullptr;
	}

	if (mTextureVertexShader)
	{
		mTextureVertexShader->Release();
		mTextureVertexShader = nullptr;
	}

	if (mTexturePixelShader)
	{
		mTexturePixelShader->Release();
		mTexturePixelShader = nullptr;
	}


}

void GameScene::Reset()
{
	mCamera.SetPosition(Vector3{ 0.0f, 0.0f, 0.0f });

	auto ball = static_cast<Ball*>(mPrimitives[0].get());
	ball->GetRigidBody().SetPosition(Vector3{ 0.0f, 5.0f, 0.0f });

	WaterGlobals::WATER_Y_SCALE = 0.f;
	WaterGlobals::B_GAME_OVER = false;
	mWater->Reset();
	mInput.Reset();
}

void GameScene::Tick(float dt)
{
	auto PrimitiveCount{ mPrimitives.size() };

	for (size_t i = 0; i < PrimitiveCount; ++i)
	{
		if (MapGlobals::ENABLE_GRAVITY)
		{
			const float mass = mPrimitives[i]->GetRigidBody().GetMass();

			const Vector3 gravityForce =
			{
				0.0f,
				-PhysicsGlobals::GRAVITY_CONSTANT * mass,
				0.0f
			};

			mPrimitives[i]->GetRigidBody().AddForce(gravityForce);
		}

		mPrimitives[i]->Tick(dt);
	}

	std::vector<CollisionManifold> Manifolds{};

	for (size_t i = 0; i < PrimitiveCount; ++i)
	{
		for (size_t j = i + 1; j < PrimitiveCount; ++j)
		{
			CollisionManifold manifold;
			// 1. 원 - 원 충돌 
			if (mPrimitives[i]->GetCollider().GetColliderType() == ColliderType_Sphere && mPrimitives[j]->GetCollider().GetColliderType() == ColliderType_Sphere)
			{
				if (CollisionDetector::FindCollision(static_cast<SphereCollider&>(mPrimitives[i]->GetCollider()), static_cast<SphereCollider&>(mPrimitives[j]->GetCollider()), manifold))
				{
					Manifolds.emplace_back(manifold);
				}
			}

			// 2. 원 - 사각형 충돌 
			if (mPrimitives[i]->GetCollider().GetColliderType() == ColliderType_Sphere && mPrimitives[j]->GetCollider().GetColliderType() == ColliderType_Box)
			{
				if (CollisionDetector::FindCollision(static_cast<SphereCollider&>(mPrimitives[i]->GetCollider()), static_cast<BoxCollider&>(mPrimitives[j]->GetCollider()), static_cast<BoxCollider&>(mPrimitives[j]->GetCollider()).GetRigidBody().GetRotation(), manifold))
				{
					Manifolds.emplace_back(manifold);
				}
			}

			// 3. 원 - 다각형 충돌 
			if (mPrimitives[i]->GetCollider().GetColliderType() == ColliderType_Sphere && mPrimitives[j]->GetCollider().GetColliderType() == ColliderType_Polygon)
			{
				if (CollisionDetector::FindCollision(static_cast<SphereCollider&>(mPrimitives[i]->GetCollider()), static_cast<PolygonCollider&>(mPrimitives[j]->GetCollider()), manifold))
				{
					Manifolds.emplace_back(manifold);
				}
			}

			// 4. 사각형 - 다각형 충돌
			if (mPrimitives[i]->GetCollider().GetColliderType() == ColliderType_Box && mPrimitives[j]->GetCollider().GetColliderType() == ColliderType_Polygon)
			{
				if (CollisionDetector::FindCollision(static_cast<BoxCollider&>(mPrimitives[i]->GetCollider()), static_cast<PolygonCollider&>(mPrimitives[j]->GetCollider()), manifold))
				{
					Manifolds.emplace_back(manifold);
				}
			}

			// 5. 다각형 - 다각형 충돌
			if (mPrimitives[i]->GetCollider().GetColliderType() == ColliderType_Polygon && mPrimitives[j]->GetCollider().GetColliderType() == ColliderType_Polygon)
			{
				if (CollisionDetector::FindCollision(static_cast<PolygonCollider&>(mPrimitives[i]->GetCollider()), static_cast<PolygonCollider&>(mPrimitives[j]->GetCollider()), manifold))
				{
					Manifolds.emplace_back(manifold);
				}
			}

		}
	}


	if (MapGlobals::BOUND_BALL_TO_SCREEN)
	{
		StaticCollider* Walls[]
		{
			&LeftWall,
			&RightWall,
			&BottomWall
		};

		for (size_t PrimitiveIndex{ 0 }; PrimitiveIndex < PrimitiveCount; ++PrimitiveIndex)
		{
			ICollider& collider{ mPrimitives[PrimitiveIndex]->GetCollider() };


			for (size_t WallIndex{ 0 }; WallIndex < 3; ++WallIndex)
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
				break;
				case  ColliderType_Polygon:
				{
					PolygonCollider& Polygon{ static_cast<PolygonCollider&>(collider) };
					if (CollisionDetector::FindCollision(Polygon, *Walls[WallIndex], Manifold))
					{
						Manifolds.emplace_back(Manifold);
					}
				}
				break;
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

	mWater->Tick(dt);

	mInput.Update();

	float MoveSpeed = 15.0f * dt;
	Vector3 CamPos = mCamera.GetPosition();
	Ball* player = static_cast<Ball*>(mPrimitives[0].get());
	CamPos.y = player->GetLocation().y - 8.f;
	if (CamPos.y < 0.0f) CamPos.y = 0.0f;

	mCamera.SetPosition(CamPos);

	const float CameraBottomY = CamPos.y;
	const float CameraCenterY = CameraBottomY + 15.0f;

	mPlatformManager.Update(CameraCenterY);
	PlayerGlobals::PLAYERLOCATION = player->GetLocation();
	PlayerGlobals::HIGH_SCORE = std::max(PlayerGlobals::HIGH_SCORE, player->GetLocation().y);

	mWater->Tick(dt);
}

void GameScene::Render(IRenderer& renderer)
{
	Renderer& concreteRenderer = static_cast<Renderer&>(renderer);

	concreteRenderer.Prepare();

	concreteRenderer.DeviceContext->VSSetShader(mTextureVertexShader, nullptr, 0);
	concreteRenderer.DeviceContext->PSSetShader(mTexturePixelShader, nullptr, 0);
	concreteRenderer.DeviceContext->IASetInputLayout(mTextureLayout);
	concreteRenderer.DeviceContext->PSSetSamplers(0, 1, &mSamplerState);
	concreteRenderer.DeviceContext->VSSetConstantBuffers(0, 1, &concreteRenderer.ConstantBuffer);

	mBackGround.Render(concreteRenderer, mSamplerState, mCamera.GetPosition().y);
	mPlatformManager.Render(concreteRenderer, mSRVPlatform);

	concreteRenderer.PrepareShader(); // 단색 기본 셰이더로 복귀

	for (auto& Primitive : mPrimitives)
	{
		Primitive->Render(renderer);
	}

	concreteRenderer.DeviceContext->VSSetShader(mTextureVertexShader, nullptr, 0);
	concreteRenderer.DeviceContext->PSSetShader(mTexturePixelShader, nullptr, 0);
	concreteRenderer.DeviceContext->IASetInputLayout(mTextureLayout);
	concreteRenderer.DeviceContext->PSSetSamplers(0, 1, &mSamplerState);
	concreteRenderer.DeviceContext->VSSetConstantBuffers(0, 1, &concreteRenderer.ConstantBuffer);

	mWater->Render(concreteRenderer);

	mInput.DragBall();
}

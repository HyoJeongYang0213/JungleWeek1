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

	VertexTexture ball_quad_vertices[] = {
		{ -1.0f,  1.0f, 0.0f,  0.0f, 0.0f },
		{  1.0f,  1.0f, 0.0f,  1.0f, 0.0f },
		{  1.0f, -1.0f, 0.0f,  1.0f, 1.0f },

		{ -1.0f,  1.0f, 0.0f,  0.0f, 0.0f },
		{  1.0f, -1.0f, 0.0f,  1.0f, 1.0f },
		{ -1.0f, -1.0f, 0.0f,  0.0f, 1.0f }
	};

	Renderer& concreteRenderer = static_cast<Renderer&>(renderer);

	for (VertexSimple& Vertex : sphere_vertices)
	{
		Vertex.r = 0.2f;
		Vertex.g = 0.2f;
		Vertex.b = 0.2f;
		Vertex.a = 1.0f;
	}

	//ID3D11Buffer* vertexBufferTriangle = concreteRenderer.CreateVertexBuffer(triangle_vertices, sizeof(triangle_vertices));
	mVertexBufferSphere = concreteRenderer.CreateVertexBuffer(sphere_vertices, sizeof(sphere_vertices));
	//ID3D11Buffer* vertexBufferCube = concreteRenderer.CreateVertexBuffer(cube_vertices, sizeof(cube_vertices));
	
	mMiniBallSystem.Init(mVertexBufferSphere, static_cast<UINT>(sizeof(sphere_vertices) / sizeof(sphere_vertices[0])));


	mBallVertexBuffer = concreteRenderer.CreateVertexBuffer(reinterpret_cast<VertexSimple*>(ball_quad_vertices),sizeof(ball_quad_vertices));


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


	/*ID3D11ShaderResourceView* ShaderResourceViewGround = TextureLoader::CreateTextureFromFile(concreteRenderer.Device, L"Asset/Stage_Ground.png");
	ID3D11ShaderResourceView* ShaderResourceViewA = TextureLoader::CreateTextureFromFile(concreteRenderer.Device, L"Asset/Stage_A.png");
	ID3D11ShaderResourceView* ShaderResourceViewB = TextureLoader::CreateTextureFromFile(concreteRenderer.Device, L"Asset/Stage_B.png");
	ID3D11ShaderResourceView* ShaderResourceViewC = TextureLoader::CreateTextureFromFile(concreteRenderer.Device, L"Asset/Stage_C.png");
	mSamplerState = TextureLoader::CreateSamplerState(concreteRenderer.Device);

	mBackGround.Init(concreteRenderer, ShaderResourceViewGround, { ShaderResourceViewA, ShaderResourceViewB, ShaderResourceViewC });*/

	mSRVBall = TextureLoader::CreateTextureFromFile(concreteRenderer.Device, L"Asset/Ball/Ball.png");

	// Sky BackGround Loading
	ID3D11ShaderResourceView* ShaderResourceViewGround = TextureLoader::CreateTextureFromFile(concreteRenderer.Device, L"Asset/Sky/Stage_Ground.png");
	ID3D11ShaderResourceView* ShaderResourceViewMorning = TextureLoader::CreateTextureFromFile(concreteRenderer.Device, L"Asset/Sky/Sky_Morning.png");
	ID3D11ShaderResourceView* ShaderResourceViewNoon = TextureLoader::CreateTextureFromFile(concreteRenderer.Device, L"Asset/Sky/Sky_Noon.png");
	ID3D11ShaderResourceView* ShaderResourceViewSunset = TextureLoader::CreateTextureFromFile(concreteRenderer.Device, L"Asset/Sky/Sky_Sunset.png");
	ID3D11ShaderResourceView* ShaderResourceViewEvening = TextureLoader::CreateTextureFromFile(concreteRenderer.Device, L"Asset/Sky/Sky_Evening.png");
	ID3D11ShaderResourceView* ShaderResourceViewNight = TextureLoader::CreateTextureFromFile(concreteRenderer.Device, L"Asset/Sky/Sky_Night.png");
	ID3D11ShaderResourceView* ShaderResourceViewSpace = TextureLoader::CreateTextureFromFile(concreteRenderer.Device, L"Asset/Sky/Sky_Space.png");
	mSamplerState = TextureLoader::CreateSamplerState(concreteRenderer.Device);

	// Transition BackGround Textures Loading
	ID3D11ShaderResourceView* srvTransMorningToNoon = TextureLoader::CreateTextureFromFile(concreteRenderer.Device, L"Asset/Sky/Sky_Trans/Sky_Trans_MorningToNoon.png");
	ID3D11ShaderResourceView* srvTransNoonToSunset = TextureLoader::CreateTextureFromFile(concreteRenderer.Device, L"Asset/Sky/Sky_Trans/Sky_Trans_NoonToSunset.png");
	ID3D11ShaderResourceView* srvTransSunsetToEvening = TextureLoader::CreateTextureFromFile(concreteRenderer.Device, L"Asset/Sky/Sky_Trans/Sky_Trans_SunsetToEvening.png");
	ID3D11ShaderResourceView* srvTransEveningToNight = TextureLoader::CreateTextureFromFile(concreteRenderer.Device, L"Asset/Sky/Sky_Trans/Sky_Trans_EveningToNight.png");
	ID3D11ShaderResourceView* srvTransNightToSpace = TextureLoader::CreateTextureFromFile(concreteRenderer.Device, L"Asset/Sky/Sky_Trans/Sky_Trans_NightToSpace.png");

	// Deco Textures Loading
	ID3D11ShaderResourceView* decoBird = TextureLoader::CreateTextureFromFile(concreteRenderer.Device, L"Asset/Sky/Sky_Deco/Deco_Bird.png");
	ID3D11ShaderResourceView* decoCloud1 = TextureLoader::CreateTextureFromFile(concreteRenderer.Device, L"Asset/Sky/Sky_Deco/Deco_Cloud1.png");
	ID3D11ShaderResourceView* decoCloud2 = TextureLoader::CreateTextureFromFile(concreteRenderer.Device, L"Asset/Sky/Sky_Deco/Deco_Cloud2.png");
	ID3D11ShaderResourceView* decoStar = TextureLoader::CreateTextureFromFile(concreteRenderer.Device, L"Asset/Sky/Sky_Deco/Deco_Str.png");
	ID3D11ShaderResourceView* decoMeteor = TextureLoader::CreateTextureFromFile(concreteRenderer.Device, L"Asset/Sky/Sky_Deco/Deco_Meteor.png");

	std::vector<std::vector<ID3D11ShaderResourceView*>> themeDecos = {
	{ decoBird, decoCloud1 },
	{ decoCloud1, decoCloud2 },
	{ decoCloud2 },
	{ decoCloud1, decoStar },
	{ decoStar },
	{ decoStar, decoMeteor }
	};

	mBackGround.Init(concreteRenderer, ShaderResourceViewGround, {
		ShaderResourceViewMorning,
		ShaderResourceViewNoon,
		ShaderResourceViewSunset,
		ShaderResourceViewEvening,
		ShaderResourceViewNight,
		ShaderResourceViewSpace}, {
		srvTransMorningToNoon,
		srvTransNoonToSunset,
		srvTransSunsetToEvening,
		srvTransEveningToNight,
		srvTransNightToSpace
		}, themeDecos
	);


	mPlatformTileManager.Init(concreteRenderer);

	mPlatformTileManager.SetBallSpawnCallback(
		[this](
			const Vector3& ContactPoint,
			const Vector3& CollisionNormal)
		{
			mMiniBallSystem.Spawn(
				ContactPoint,
				CollisionNormal
			);
		}
	);


	mCamera.SetPosition(Vector3{ 0.0f, 0.0f, 0.0f });

	GameScene::CreatePrimitive<Ball>(mVertexBufferSphere, static_cast<UINT>(sizeof(sphere_vertices) / sizeof(sphere_vertices[0])));

	PlayerGlobals::PLAYERLOCATION = static_cast<Ball*>(mPrimitives[0].get())->GetLocation();
	PlayerGlobals::PLAYERBALL = static_cast<Ball*>(mPrimitives[0].get());

	mInput.RegisterDragCallback([&](POINT targetPos)
		{
			Ball* player = static_cast<Ball*>(mPrimitives[0].get());
			Vector3 ReleasePoint{ Vector3(static_cast<float>(targetPos.x), static_cast<float>(targetPos.y), 0.0f) };
			player->GetRigidBody().ApplyImpulse((player->GetLocation() - ReleasePoint) * PlayerGlobals::PLAYER_DRAG_IMPULSE_MULTIPLIER, player->GetLocation());
		}
	);


	mWater->Start(); 
}

GameScene::~GameScene()
{
	mPrimitives.clear();

	if (mVertexBufferSphere)
	{
		mVertexBufferSphere->Release();
		mVertexBufferSphere = nullptr;
	}

	if (mSamplerState)
	{
		mSamplerState->Release();
		mSamplerState = nullptr;
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

	if (mBallVertexBuffer)
	{
		mBallVertexBuffer->Release();
		mBallVertexBuffer = nullptr;
	}

	if (mSRVBall)
	{
		mSRVBall->Release();
		mSRVBall = nullptr;
	}


}

void GameScene::Reset()
{
	mCamera.SetPosition(Vector3{ 0.0f, 0.0f, 0.0f });
	mPlatformTileManager.Reset();
	mPlatformTileManager.Update(MapGlobals::VIEW_HEIGHT * 0.5f);

	auto ball = static_cast<Ball*>(mPrimitives[0].get());
	ball->GetRigidBody().SetPosition(Vector3{ 0.0f, 5.0f, 0.0f });

	WaterGlobals::WATER_Y_SCALE = 0.f;
	WaterGlobals::B_GAME_OVER = false;

	mMiniBallSystem.Clear();
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

	//mPlatformTileManager.Update(mCamera.GetPosition().y + MapGlobals::VIEW_HEIGHT * 0.5f);

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

	mPlatformTileManager.FindCollisions(static_cast<SphereCollider&>(mPrimitives[0]->GetCollider()), Manifolds);


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



	for (CollisionManifold& manifold : Manifolds)
	{
		if (manifold.ColliderA.Type == ColliderType_Polygon and manifold.ColliderB.Type == ColliderType_Sphere)
		{
			manifold.ColliderB.RigidBody->ApplyRotateResistance(manifold.Normal, PhysicsGlobals::PLATFORM_ROTATION_RESISTANCE_COEFFICIENT, dt);
		}
		else if (manifold.ColliderA.Type == ColliderType_Sphere and manifold.ColliderB.Type == ColliderType_Polygon)
		{
			manifold.ColliderA.RigidBody->ApplyRotateResistance(manifold.Normal, PhysicsGlobals::PLATFORM_ROTATION_RESISTANCE_COEFFICIENT, dt);
		}
	}

	mInput.Update();

	float MoveSpeed = 15.0f * dt;
	Vector3 CamPos = mCamera.GetPosition();
	Ball* player = static_cast<Ball*>(mPrimitives[0].get());
	CamPos.y = player->GetLocation().y - 8.f;
	if (CamPos.y < 0.0f) CamPos.y = 0.0f;

	mCamera.SetPosition(CamPos);

	const float CameraBottomY = CamPos.y;
	const float CameraCenterY = CameraBottomY + 15.0f;

	mPlatformTileManager.Update(CameraCenterY);

	mMiniBallSystem.Tick(dt);

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

	concreteRenderer.SetAlphaBlendState(true);
	mPlatformTileManager.Render(renderer);
	concreteRenderer.SetAlphaBlendState(false);
	
	concreteRenderer.PrepareShader();

	concreteRenderer.DeviceContext->VSSetShader(mTextureVertexShader, nullptr, 0);
	concreteRenderer.DeviceContext->PSSetShader(mTexturePixelShader, nullptr, 0);
	concreteRenderer.DeviceContext->IASetInputLayout(mTextureLayout);
	concreteRenderer.DeviceContext->PSSetSamplers(0, 1, &mSamplerState);
	concreteRenderer.DeviceContext->VSSetConstantBuffers(0, 1, &concreteRenderer.ConstantBuffer);

	if (mSRVBall && mBallVertexBuffer)
	{
		Ball* player = static_cast<Ball*>(mPrimitives[0].get());
		Vector3 pos = player->GetLocation();
		float radius = player->GetRadius();
		float angle = player->GetRigidBody().GetRotation();

		concreteRenderer.UpdateConstant(
			Vector3{ pos.x, pos.y, 0.0f },
			Vector3{ radius, radius, 1.0f },
			angle
		);

		concreteRenderer.DeviceContext->PSSetShaderResources(0, 1, &mSRVBall);

		UINT stride = sizeof(VertexTexture);
		UINT offset = 0;
		concreteRenderer.DeviceContext->IASetVertexBuffers(0, 1, &mBallVertexBuffer, &stride, &offset);
		concreteRenderer.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// 6개 정점 직접 그리기
		concreteRenderer.DeviceContext->Draw(6, 0);
	}

	concreteRenderer.PrepareShader(); // 단색 기본 셰이더로 복귀

	for (size_t i = 1; i < mPrimitives.size(); ++i)
	{
		mPrimitives[i]->Render(renderer);
	}

	mMiniBallSystem.Render(renderer);

	concreteRenderer.DeviceContext->VSSetShader(mTextureVertexShader, nullptr, 0);
	concreteRenderer.DeviceContext->PSSetShader(mTexturePixelShader, nullptr, 0);
	concreteRenderer.DeviceContext->IASetInputLayout(mTextureLayout);
	concreteRenderer.DeviceContext->PSSetSamplers(0, 1, &mSamplerState);
	concreteRenderer.DeviceContext->VSSetConstantBuffers(0, 1, &concreteRenderer.ConstantBuffer);

	mWater->Render(concreteRenderer);

	mInput.DragBall();
}

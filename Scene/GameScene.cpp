#include "GameScene.h"

#include "../Physics/PhysicsGlobals.hpp"
#include "../Physics/RigidBody.h"
#include "../Physics/CollisionManifold.hpp"
#include "../Physics/CollisionDetector.h"
#include "../Physics/CollisionResolver.h"

void GameScene::Tick(float dt)
{
	auto PrimitiveCount{ mPrimitives.size() };

	for (auto& Primitive : mPrimitives)
	{
		if (Globals::ENABLE_GRAVITY)
		{
			const float mass = Primitive->GetRigidBody().GetMass();
			const Vector3 gravityForce =
			{
				0.0f,
				-Globals::GRAVITY_CONSTANT * mass,
				0.0f
			};
			Primitive->GetRigidBody().AddForce(gravityForce);
		}
		Primitive->Tick(dt);
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

		for (auto& Primitive : mPrimitives)
		{
			ICollider& collider{ Primitive->GetCollider() };

			for (auto& Wall : Walls)
			{
				CollisionManifold Manifold{};

				switch (collider.GetColliderType())
				{
				case ColliderType_Sphere:
				{
					SphereCollider& Circle{ static_cast<SphereCollider&>(collider) };
					if (CollisionDetector::FindCollision(Circle, *Wall, Manifold))
					{
						Manifolds.emplace_back(Manifold);
					}
					break;
				}
				case ColliderType_Box:
				{
					// BoxCollider& Box{ static_cast<BoxCollider&>(collider) };
					break;
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

void GameScene::Render(IRenderer& renderer)
{
	for (auto& Primitive : mPrimitives)
	{
		Primitive->Render(renderer);
	}
}

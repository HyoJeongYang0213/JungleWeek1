#pragma once 
#include "../Utils/Math.hpp"
#include "Collider.h"
#include "CollisionManifold.hpp"
#include "PhysicsGlobals.hpp"

class CollisionResolver
{
public:
	static void ResolvePosition(CollisionManifold& Manifold);
	static void ResolveCollision(CollisionManifold& Manifold);
	static float ResolveFrictionCoefficient(CollisionManifold& Manifold);
};

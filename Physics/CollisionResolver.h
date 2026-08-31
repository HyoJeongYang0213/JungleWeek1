#pragma once 
#include "Math.hpp"
#include "Collider.h"
#include "CollisionManifold.hpp"
#include "PhysicsGlobals.hpp"

class CollisionResolver
{
public:
	static void ResolveCollision(CollisionManifold& Manifold); 
};

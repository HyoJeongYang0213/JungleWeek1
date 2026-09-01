#pragma once 
#include "Collider.h"
#include "CollisionManifold.hpp"

class CollisionDetector
{
public:
	static bool FindCollision(SphereCollider& Ca, SphereCollider& Cb, CollisionManifold& OutManifold); 
	static bool FindCollision(SphereCollider& c, StaticCollider& s, CollisionManifold& OutManifold);
};
#pragma once 
#include "Collider.h"
#include "CollisionManifold.hpp"

class CollisionDetector
{
public:
	static bool FindCollision(Collider& Ca, Collider& Cb, CollisionManifold& OutManifold); 
	static bool FindCollision(Collider& c, StaticCollider& s, CollisionManifold& OutManifold);
};
#pragma once 

#include "RigidBody.h"
#include "../Utils/Math.hpp"

struct CollisionManifold
{
	RigidBody* ColliderA{};
	RigidBody* ColliderB{};

	Vector3 Normal{};
	Vector3 ContactPoint{};

	float Penetration{};
};

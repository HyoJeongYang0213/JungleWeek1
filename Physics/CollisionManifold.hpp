#pragma once 

#include "RigidBody.h"
#include "ColliderType.hpp"

#include "../Utils/Math.hpp"

struct ColliderInfo
{
	RigidBody* RigidBody{};
	ColliderType Type{};
};

struct CollisionManifold
{
	ColliderInfo ColliderA{};
	ColliderInfo ColliderB{};

	Vector3 Normal{};
	Vector3 ContactPoint{};

	float Penetration{};

	// Iteration variables
	float AccumulatedNormalImpulse{};
	float AccumulatedFrictionImpulse{};

	float RestitutionBias{};
};

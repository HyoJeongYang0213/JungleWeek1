#pragma once 

#include <array>

#include "RigidBody.h"
#include "ColliderType.hpp"

#include "../Utils/Math.hpp"

struct ColliderInfo
{
	RigidBody* RigidBody{};
	ColliderType Type{};
};

struct ContactConstraint
{
	Vector3 Point{};

	float AccumulatedNormalImpulse{};
	float AccumulatedFrictionImpulse{};
	float RestitutionBias{};
};

struct CollisionManifold
{
	ColliderInfo ColliderA{};
	ColliderInfo ColliderB{};

	Vector3 Normal{};
	float Penetration{};

	std::array<ContactConstraint, 2> Contacts{};
	size_t ContactCount{};
};
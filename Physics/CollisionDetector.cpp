#include "CollisionDetector.h"

bool CollisionDetector::FindCollision(Collider& Ca, Collider& Cb, CollisionManifold& OutManifold)
{
	Vector3 delta = Cb.GetCenter() - Ca.GetCenter();
	float distanceSquared = delta.SquaredLength();
	float radiusSum = Ca.GetRadius() + Cb.GetRadius();

	if (distanceSquared < radiusSum * radiusSum)
	{
		float distance = sqrtf(distanceSquared);

		OutManifold.ColliderA = &Ca.GetRigidBody();
		OutManifold.ColliderB = &Cb.GetRigidBody();

		OutManifold.Normal = (distance > 0.0001f) ? delta / distance : Vector3(1.0f, 0.0f, 0.0f);
		OutManifold.ContactPoint = Ca.GetCenter() + OutManifold.Normal * Ca.GetRadius();
		OutManifold.Penetration = radiusSum - distance;

		return true;
	}

	return false;
}

bool CollisionDetector::FindCollision(Collider& c, StaticCollider& s, CollisionManifold& OutManifold)
{
	const Vector3 CircleToPlanePoint{ c.GetCenter() - s.GetPosition() };
	const float SignedDistance{ CircleToPlanePoint.Dot(s.GetNormal()) };

	if (SignedDistance >= c.GetRadius())
	{
		return false;
	}

	OutManifold.ColliderA = &c.GetRigidBody();
	OutManifold.ColliderB = &s.GetRigidBody();
	OutManifold.Normal = s.GetNormal() * -1.0f;
	OutManifold.Penetration = c.GetRadius() - SignedDistance;
	OutManifold.ContactPoint = c.GetCenter() - s.GetNormal() * SignedDistance;

	return true;
}


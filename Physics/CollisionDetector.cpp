#include "CollisionDetector.h"
#include <algorithm>

bool CollisionDetector::FindCollision(SphereCollider& Ca, SphereCollider& Cb, CollisionManifold& OutManifold)
{
	Vector3 delta = Cb.GetCenter() - Ca.GetCenter();
	float distanceSquared = delta.SquaredLength();
	float radiusSum = Ca.GetRadius() + Cb.GetRadius();

	if (distanceSquared < radiusSum * radiusSum)
	{
		float distance = sqrtf(distanceSquared);

		OutManifold.ColliderA.RigidBody = &Ca.GetRigidBody();
		OutManifold.ColliderA.Type = ColliderType_Sphere;

		OutManifold.ColliderB.RigidBody = &Cb.GetRigidBody();
		OutManifold.ColliderB.Type = ColliderType_Sphere;

		OutManifold.Normal = (distance > 0.0001f) ? delta / distance : Vector3(1.0f, 0.0f, 0.0f);
		OutManifold.ContactPoint = Ca.GetCenter() + OutManifold.Normal * Ca.GetRadius();
		OutManifold.Penetration = radiusSum - distance;

		return true;
	}

	return false;
}

bool CollisionDetector::FindCollision(SphereCollider& c, StaticCollider& s, CollisionManifold& OutManifold)
{
	const Vector3 CircleToPlanePoint{ c.GetCenter() - s.GetPosition() };
	const float SignedDistance{ CircleToPlanePoint.Dot(s.GetNormal()) };

	if (SignedDistance >= c.GetRadius())
	{
		return false;
	}

	OutManifold.ColliderA.RigidBody = &c.GetRigidBody();
	OutManifold.ColliderA.Type = ColliderType_Sphere;

	OutManifold.ColliderB.RigidBody = &s.GetRigidBody();
	OutManifold.ColliderB.Type = ColliderType_Plane;

	OutManifold.Normal = s.GetNormal() * -1.0f;
	OutManifold.Penetration = c.GetRadius() - SignedDistance;
	OutManifold.ContactPoint = c.GetCenter() - s.GetNormal() * SignedDistance;

	return true;
}

bool CollisionDetector::FindCollision(SphereCollider& c, BoxCollider& aabb, CollisionManifold& OutManifold) 
{
	if (not aabb.IsPointInside(c.GetCenter()))
	{
		Vector3 closestPoint;
		closestPoint.x = std::clamp(c.GetCenter().x, aabb.GetRigidBody().GetPosition().x - aabb.GetHalfExtents().x, aabb.GetRigidBody().GetPosition().x + aabb.GetHalfExtents().x);
		closestPoint.y = std::clamp(c.GetCenter().y, aabb.GetRigidBody().GetPosition().y - aabb.GetHalfExtents().y, aabb.GetRigidBody().GetPosition().y + aabb.GetHalfExtents().y);
		closestPoint.z = 0.f;

		Vector3 delta = c.GetCenter() - closestPoint;

		if (delta.SquaredLength() < c.GetRadius() * c.GetRadius())
		{
			float distance = sqrtf(delta.SquaredLength());
			
			OutManifold.ColliderA.RigidBody = &aabb.GetRigidBody();
			OutManifold.ColliderA.Type = ColliderType_Box;

			OutManifold.ColliderB.RigidBody = &c.GetRigidBody();
			OutManifold.ColliderB.Type = ColliderType_Sphere;
			OutManifold.Normal = (distance > 0.0001f) ? delta / distance : Vector3(1.0f, 0.0f, 0.0f);
			OutManifold.ContactPoint = closestPoint;
			OutManifold.Penetration = c.GetRadius() - distance;
			
			return true;
		}
		else {
			return false;
		}
	}
	else {
		float penestrations[4] = {
			(c.GetCenter().x - (aabb.GetRigidBody().GetPosition().x - aabb.GetHalfExtents().x)) + c.GetRadius(),
			((aabb.GetRigidBody().GetPosition().x + aabb.GetHalfExtents().x) - c.GetCenter().x) + c.GetRadius(),
			(c.GetCenter().y - (aabb.GetRigidBody().GetPosition().y - aabb.GetHalfExtents().y)) + c.GetRadius(),
			((aabb.GetRigidBody().GetPosition().y + aabb.GetHalfExtents().y) - c.GetCenter().y) + c.GetRadius()
		};

		float minPenetration = *std::min_element(penestrations, penestrations + 4);
		
		int minIndex = static_cast<int>(std::distance(penestrations, std::min_element(penestrations, penestrations + 4)));

		OutManifold.ColliderA.RigidBody = &aabb.GetRigidBody();
		OutManifold.ColliderA.Type = ColliderType_Box;

		OutManifold.ColliderB.RigidBody = &c.GetRigidBody();
		OutManifold.ColliderB.Type = ColliderType_Sphere;
		OutManifold.Penetration = minPenetration;
		
		
		switch (minIndex)
		{
		case 0:
			OutManifold.ContactPoint = Vector3(aabb.GetRigidBody().GetPosition().x - aabb.GetHalfExtents().x, c.GetCenter().y, 0.0f);
			OutManifold.Normal = Vector3(-1.0f, 0.0f, 0.0f);
			break;
		case 1: 
			OutManifold.ContactPoint = Vector3(aabb.GetRigidBody().GetPosition().x + aabb.GetHalfExtents().x, c.GetCenter().y, 0.0f);
			OutManifold.Normal = Vector3(1.0f, 0.0f, 0.0f);
			break;
		case 2:
			OutManifold.ContactPoint = Vector3(c.GetCenter().x, aabb.GetRigidBody().GetPosition().y - aabb.GetHalfExtents().y, 0.0f);
			OutManifold.Normal = Vector3(0.0f, -1.0f, 0.0f);
			break;
		case 3:
			OutManifold.ContactPoint = Vector3(c.GetCenter().x, aabb.GetRigidBody().GetPosition().y + aabb.GetHalfExtents().y, 0.0f);
			OutManifold.Normal = Vector3(0.0f, 1.0f, 0.0f);
			break;
		default:
			break;
		}

		return true;
	}

	return false;
}

bool CollisionDetector::FindCollision(SphereCollider& c, BoxCollider& box, float theta, CollisionManifold& OutManifold) 
{
	Vector3 SphereCenter = c.GetCenter();
	Vector3 SphereVelocity = c.GetRigidBody().GetVelocity();
	float mass = c.GetRigidBody().GetMass();
	RigidBody SphereRigidBodyClone{ SphereCenter, SphereVelocity, mass, c.GetRigidBody().GetInverseMomentOfInertia() };
	float SphereRadius = c.GetRadius();
	SphereCollider sphereClone{ SphereRigidBodyClone, SphereRadius };

	Vector3 DirectionToSphere = sphereClone.GetCenter() - box.GetRigidBody().GetPosition();
	SphereRigidBodyClone.SetPosition(Vector3(DirectionToSphere.x * cosf(theta) + DirectionToSphere.y * sinf(theta), DirectionToSphere.y * cosf(theta) - DirectionToSphere.x * sinf(theta), 0.f ) + box.GetRigidBody().GetPosition());

	if (not FindCollision(sphereClone, box, OutManifold))
	{
		return false;
	}

	const Vector3 localNormal = OutManifold.Normal;

	OutManifold.Normal = Vector3(localNormal.x * cosf(-theta) + localNormal.y * sinf(-theta), localNormal.y * cosf(-theta) - localNormal.x * sinf(-theta), 0.f);

	Vector3 localContactPoint = OutManifold.ContactPoint - box.GetRigidBody().GetPosition();

	OutManifold.ContactPoint = Vector3(localContactPoint.x * cosf(-theta) + localContactPoint.y * sinf(-theta), localContactPoint.y * cosf(-theta) - localContactPoint.x * sinf(-theta), 0.f) + box.GetRigidBody().GetPosition();

	OutManifold.ColliderA.RigidBody = &box.GetRigidBody();
	OutManifold.ColliderA.Type = ColliderType_Box;

	OutManifold.ColliderB.RigidBody = &c.GetRigidBody();
	OutManifold.ColliderB.Type = ColliderType_Sphere;

	return true; 
}


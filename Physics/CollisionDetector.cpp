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

		OutManifold.ContactCount = 1;
		OutManifold.Contacts[0].Point = Ca.GetCenter() + OutManifold.Normal * Ca.GetRadius();
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

	OutManifold.ContactCount = 1;
	OutManifold.Contacts[0].Point = c.GetCenter() - s.GetNormal() * SignedDistance;

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

			OutManifold.ContactCount = 1;
			OutManifold.Contacts[0].Point = closestPoint;
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
			OutManifold.ContactCount = 1;
			OutManifold.Contacts[0].Point = Vector3(aabb.GetRigidBody().GetPosition().x - aabb.GetHalfExtents().x, c.GetCenter().y, 0.0f);
			OutManifold.Normal = Vector3(-1.0f, 0.0f, 0.0f);
			break;
		case 1: 
			OutManifold.ContactCount = 1;
			OutManifold.Contacts[0].Point = Vector3(aabb.GetRigidBody().GetPosition().x + aabb.GetHalfExtents().x, c.GetCenter().y, 0.0f);
			OutManifold.Normal = Vector3(1.0f, 0.0f, 0.0f);
			break;
		case 2:
			OutManifold.ContactCount = 1;
			OutManifold.Contacts[0].Point = Vector3(c.GetCenter().x, aabb.GetRigidBody().GetPosition().y - aabb.GetHalfExtents().y, 0.0f);
			OutManifold.Normal = Vector3(0.0f, -1.0f, 0.0f);
			break;
		case 3:
			OutManifold.ContactCount = 1;
			OutManifold.Contacts[0].Point = Vector3(c.GetCenter().x, aabb.GetRigidBody().GetPosition().y + aabb.GetHalfExtents().y, 0.0f);
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

	const Vector3 BoxPosition = box.GetRigidBody().GetPosition();
	const float BoxRotation = box.GetRigidBody().GetRotation();

	const Vector3 LocalSphereCenter = CollisionDetector::ToLocalSpace(c.GetCenter(), BoxPosition, BoxRotation);
	SphereRigidBodyClone.SetPosition(LocalSphereCenter + BoxPosition);

	if (not FindCollision(sphereClone, box, OutManifold))
	{
		return false;
	}

	OutManifold.Normal = CollisionDetector::ToWorldSpaceVector(OutManifold.Normal, BoxRotation);

	OutManifold.ContactCount = 1;
	const Vector3 LocalContactPoint = OutManifold.Contacts[0].Point - BoxPosition;
	OutManifold.Contacts[0].Point = CollisionDetector::ToWorldSpacePoint(LocalContactPoint, BoxPosition, BoxRotation);

	OutManifold.ColliderA.RigidBody = &box.GetRigidBody();
	OutManifold.ColliderA.Type = ColliderType_Box;

	OutManifold.ColliderB.RigidBody = &c.GetRigidBody();
	OutManifold.ColliderB.Type = ColliderType_Sphere;

	return true;
}

bool CollisionDetector::FindCollision(SphereCollider& c, PolygonCollider& poly, CollisionManifold& OutManifold)
{
	Vector3 SphereCenter = c.GetCenter();

	SphereCenter = CollisionDetector::ToLocalSpace(SphereCenter, poly.GetRigidBody().GetPosition(), poly.GetRigidBody().GetRotation());

	const std::vector<Vector3>& HullPoints = poly.GetHullPoints();

	Vector3 ClosestPoint{};
	Vector3 ClosestNormal{};

	float MinDistanceSquared = std::numeric_limits<float>::max();

	for (size_t i = 0; i < HullPoints.size(); ++i)
	{
		const Vector3& A = HullPoints[i];
		const Vector3& B = HullPoints[(i + 1) % HullPoints.size()];

		const Vector3 Edge = B - A;
		const float EdgeLengthSquared = Edge.SquaredLength();

		if (EdgeLengthSquared < 0.0001f)
		{
			continue;
		}

		const float T = std::clamp((SphereCenter - A).Dot(Edge) / EdgeLengthSquared, 0.0f, 1.0f);
		const Vector3 Point = A + Edge * T;
		const Vector3 Delta = SphereCenter - Point;
		const float DistanceSquared = Delta.SquaredLength();

		if (DistanceSquared < MinDistanceSquared)
		{
			MinDistanceSquared = DistanceSquared;
			ClosestPoint = Point;
			ClosestNormal = Vector3(Edge.y, -Edge.x, 0.0f).Normalize();
		}
	}

	const float Distance = sqrtf(MinDistanceSquared);
	const bool IsInside = poly.IsPointInside(SphereCenter);

	if (!IsInside)
	{
		if (Distance >= c.GetRadius())
		{
			return false;
		}

		if (Distance > 0.0001f)
		{
			OutManifold.Normal = (SphereCenter - ClosestPoint).Normalize();
		}
		else
		{
			OutManifold.Normal = ClosestNormal;
		}

		OutManifold.Penetration = c.GetRadius() - Distance;
	}
	else
	{
		if (Distance > 0.0001f)
		{
			OutManifold.Normal = (ClosestPoint - SphereCenter).Normalize();
		}
		else
		{
			OutManifold.Normal = ClosestNormal;
		}

		OutManifold.Penetration = c.GetRadius() + Distance;
	}

	OutManifold.Contacts[0].Point = ClosestPoint;
	OutManifold.ContactCount = 1;

	OutManifold.Normal = CollisionDetector::ToWorldSpaceVector(OutManifold.Normal, poly.GetRigidBody().GetRotation());
	OutManifold.Contacts[0].Point = CollisionDetector::ToWorldSpacePoint(OutManifold.Contacts[0].Point, poly.GetRigidBody().GetPosition(), poly.GetRigidBody().GetRotation());

	OutManifold.ColliderA.RigidBody = &poly.GetRigidBody();
	OutManifold.ColliderA.Type = ColliderType_Polygon;

	OutManifold.ColliderB.RigidBody = &c.GetRigidBody();
	OutManifold.ColliderB.Type = ColliderType_Sphere;

	return true;
}

bool CollisionDetector::FindCollision(BoxCollider& box, PolygonCollider& poly, CollisionManifold& OutManifold)
{
	const ConvexCollisionData BoxData = MakeCollisionData(box);
	const ConvexCollisionData PolyData = MakeCollisionData(poly);

	SATResult Result{};

	if (not CollisionDetector::FindCollisionSAT(BoxData, PolyData, Result))
	{
		return false;
	}


	if (not CollisionDetector::GenerateDetector(BoxData.Points, PolyData.Points, Result.Normal, OutManifold))
	{
		return false;
	}

	OutManifold.ColliderA.RigidBody = &box.GetRigidBody();
	OutManifold.ColliderA.Type = ColliderType_Box;

	OutManifold.ColliderB.RigidBody = &poly.GetRigidBody();
	OutManifold.ColliderB.Type = ColliderType_Polygon;

	OutManifold.Normal = Result.Normal;
	OutManifold.Penetration = Result.Penetration;

	return true;
}

bool CollisionDetector::FindCollision(PolygonCollider& poly, PolygonCollider& poly2, CollisionManifold& OutManifold)
{
	const ConvexCollisionData PolyData1 = MakeCollisionData(poly);
	const ConvexCollisionData PolyData2 = MakeCollisionData(poly2);

	SATResult Result{};

	if (not CollisionDetector::FindCollisionSAT(PolyData1, PolyData2, Result))
	{
		return false;
	}


	if (not CollisionDetector::GenerateDetector(PolyData1.Points, PolyData2.Points, Result.Normal, OutManifold))
	{
		return false;
	}

	OutManifold.ColliderA.RigidBody = &poly.GetRigidBody();
	OutManifold.ColliderA.Type = ColliderType_Polygon;

	OutManifold.ColliderB.RigidBody = &poly2.GetRigidBody();
	OutManifold.ColliderB.Type = ColliderType_Polygon;

	OutManifold.Normal = Result.Normal;
	OutManifold.Penetration = Result.Penetration;

	return true;
}

bool CollisionDetector::FindCollision(PolygonCollider& poly, StaticCollider& s, CollisionManifold& OutManifold)
{
	const Vector3& PlanePosition{ s.GetPosition() };
	const Vector3& PlaneNormal{ s.GetNormal() };

	const ConvexCollisionData PolyData = MakeCollisionData(poly);

	if (PolyData.Points.empty())
	{
		return false;
	}

	float MinDistance = std::numeric_limits<float>::max();

	for (const Vector3& Point : PolyData.Points)
	{
		const float Distance = (Point - PlanePosition).Dot(PlaneNormal);
		MinDistance = std::min(MinDistance, Distance);
	}

	constexpr float ContactEpsilon{ 0.001f };
	if (MinDistance > ContactEpsilon)
	{
		return false;
	}

	OutManifold.ColliderA.RigidBody = &s.GetRigidBody();
	OutManifold.ColliderA.Type = ColliderType_Plane;

	OutManifold.ColliderB.RigidBody = &poly.GetRigidBody();
	OutManifold.ColliderB.Type = ColliderType_Polygon;

	OutManifold.Normal = PlaneNormal;
	OutManifold.Penetration = std::max(0.0f, -MinDistance);


	OutManifold.ContactCount = 0;

	for (const Vector3& Point : PolyData.Points)
	{
		const float Distance = (Point - PlanePosition).Dot(PlaneNormal);
		if (Distance - MinDistance <= ContactEpsilon)
		{
			// 평면 위로 투영 
			const Vector3 ContractPoint = Point - PlaneNormal * Distance;

			if (OutManifold.ContactCount < 2) 
			{
				OutManifold.Contacts[OutManifold.ContactCount].Point = ContractPoint;
				++OutManifold.ContactCount;
			}
		}
	}

	return OutManifold.ContactCount > 0;
}

Vector3 CollisionDetector::ToLocalSpace(const Vector3& point, const Vector3& position, float rotation)
{
	const Vector3 Delta = point - position;

	const float SinTheta = sinf(rotation);
	const float CosTheta = cosf(rotation);

	return Vector3(Delta.x * CosTheta + Delta.y * SinTheta, -Delta.x * SinTheta + Delta.y * CosTheta, 0.0f);

}

Vector3 CollisionDetector::ToWorldSpacePoint(const Vector3& point, const Vector3& position, float rotation)
{
	const float SinTheta = sinf(rotation);
	const float CosTheta = cosf(rotation);

	return Vector3(point.x * CosTheta - point.y * SinTheta, point.x * SinTheta + point.y * CosTheta, 0.0f) + position;
}

Vector3 CollisionDetector::ToWorldSpaceVector(const Vector3& vector, float rotation)
{
	const float SinTheta = sinf(rotation);
	const float CosTheta = cosf(rotation);

	return Vector3(vector.x * CosTheta - vector.y * SinTheta, vector.x * SinTheta + vector.y * CosTheta, 0.0f);
}

CollisionDetector::ConvexCollisionData CollisionDetector::MakeCollisionData(const BoxCollider& box)
{
	const Vector3& BoxPosition = box.GetRigidBody().GetPosition();
	const float BoxRotation = box.GetRigidBody().GetRotation();
	const Vector3& HalfExtents = box.GetHalfExtents();

	ConvexCollisionData Data{};

	Data.Points.emplace_back(CollisionDetector::ToWorldSpacePoint(Vector3(-HalfExtents.x, -HalfExtents.y, 0.0f), BoxPosition, BoxRotation) );
	Data.Points.emplace_back(CollisionDetector::ToWorldSpacePoint(Vector3(HalfExtents.x, -HalfExtents.y, 0.0f), BoxPosition, BoxRotation) );
	Data.Points.emplace_back(CollisionDetector::ToWorldSpacePoint(Vector3(HalfExtents.x, HalfExtents.y, 0.0f), BoxPosition, BoxRotation) );
	Data.Points.emplace_back(CollisionDetector::ToWorldSpacePoint(Vector3(-HalfExtents.x, HalfExtents.y, 0.0f), BoxPosition, BoxRotation) );

	Data.Axes.emplace_back(CollisionDetector::ToWorldSpaceVector(Vector3(1.0f, 0.0f, 0.0f), BoxRotation).Normalize());
	Data.Axes.emplace_back(CollisionDetector::ToWorldSpaceVector(Vector3(0.0f, 1.0f, 0.0f), BoxRotation).Normalize());


	return Data;
}

CollisionDetector::ConvexCollisionData CollisionDetector::MakeCollisionData(const PolygonCollider& poly)
{
	const Vector3& Position = poly.GetRigidBody().GetPosition();
	const float Rotation = poly.GetRigidBody().GetRotation();

	ConvexCollisionData Data{};

	const auto& HullPoints = poly.GetHullPoints();
	
	for(const Vector3& LocalPoint : HullPoints)
	{
		Data.Points.emplace_back( CollisionDetector::ToWorldSpacePoint(LocalPoint, Position, Rotation) );
	}

	for (size_t i = 0; i < Data.Points.size(); ++i)
	{
		const Vector3& A = Data.Points[i];
		const Vector3& B = Data.Points[(i + 1) % Data.Points.size()];

		const Vector3 Edge = B - A;
		if (Edge.SquaredLength() < 0.0001f)
		{
			continue;
		}
		
		const Vector3 EdgeNormal(Edge.y, -Edge.x, 0.0f);
		Data.Axes.emplace_back(EdgeNormal.Normalize());
	}


	return Data;
}

void CollisionDetector::ProjectionPoints(const std::vector<Vector3>& Points, const Vector3& Axis, float& OutMin, float& OutMax)
{
	OutMin = Points.front().Dot(Axis);
	OutMax = OutMin;

	for (const Vector3& Point : Points)
	{
		const float Projection = Point.Dot(Axis);

		OutMin = std::min(OutMin, Projection);
		OutMax = std::max(OutMax, Projection);
	}
}

bool CollisionDetector::SATTest(const std::vector<Vector3>& PointsA, const std::vector<Vector3>& PointsB, const Vector3& Axis, SATResult& OutResult)
{
	if (Axis.SquaredLength() <= 0.000001f)
	{
		return true;
	}

	const Vector3 axis = Axis.Normalize();

	float minA{};
	float maxA{};
	float minB{};
	float maxB{};

	ProjectionPoints(PointsA, axis, minA, maxA);
	ProjectionPoints(PointsB, axis, minB, maxB);

	if (maxA < minB || maxB < minA)
	{
		return false;
	}
	
	// B 를 +Axis 방향으로 밀어내는데 필요한 거리
	const float MoveBPositive = maxA - minB;

	// B 를 -Axis 방향으로 밀어내는데 필요한 거리
	const float MoveBNegative = maxB - minA;


	if (MoveBPositive < MoveBNegative)
	{
		if (MoveBPositive < OutResult.Penetration)
		{
			OutResult.Penetration = MoveBPositive;
			OutResult.Normal = axis;
		}
	}
	else
	{
		if (MoveBNegative < OutResult.Penetration)
		{
			OutResult.Penetration = MoveBNegative;
			OutResult.Normal = axis * -1.0f;
		}
	}


	return true;
}

bool CollisionDetector::FindCollisionSAT(const ConvexCollisionData& DataA, const ConvexCollisionData& DataB, SATResult& OutResult)
{
	if (DataA.Points.size() < 3 || DataB.Points.size() < 3)
	{
		return false;
	}


	OutResult.Penetration = std::numeric_limits<float>::max();
	OutResult.Normal = Vector3(0.0f, 0.0f, 0.0f);

	for (const Vector3& Axis : DataA.Axes)
	{
		if (not SATTest(DataA.Points, DataB.Points, Axis, OutResult))
		{
			return false;
		}
	}

	for (const Vector3& Axis : DataB.Axes)
	{
		if (not SATTest(DataA.Points, DataB.Points, Axis, OutResult))
		{
			return false;
		}
	}


	return true;
}

CollisionDetector::SupportFeature CollisionDetector::FindSupportFeatures(const std::vector<Vector3>& Points, const Vector3& Direction, bool FindMax)
{
	
	constexpr float SupportEpsilon{ 0.0001f };
	SupportFeature Result{};

	float SupportDistance = FindMax ? -std::numeric_limits<float>::max() : std::numeric_limits<float>::max();

	for (const Vector3& Point : Points)
	{
		const float Projection = Point.Dot(Direction);
		if (FindMax)
		{
			SupportDistance = std::max(SupportDistance, Projection);
		}
		else
		{
			SupportDistance = std::min(SupportDistance, Projection);
		}
	}

	Result.Distance = SupportDistance;

	for (const Vector3& Point : Points)
	{
		const float Projection = Point.Dot(Direction);
		
		const float diff = FindMax ? SupportDistance - Projection : Projection - SupportDistance;

		if (diff <= SupportEpsilon)
		{
			Result.Points.emplace_back(Point);
		}
	}
		
	return Result;
}

void CollisionDetector::ProjectFeature(const std::vector<Vector3>& Points, const Vector3& Axis, float& OutMin, float& OutMax)
{
	OutMin = Points.front().Dot(Axis);
	OutMax = OutMin;

	for (const Vector3& Point : Points)
	{
		const float Projection = Point.Dot(Axis);
		OutMin = std::min(OutMin, Projection);
		OutMax = std::max(OutMax, Projection);
	}
}

bool CollisionDetector::GenerateDetector(const std::vector<Vector3>& PointsA, const std::vector<Vector3>& PointsB, const Vector3& Normal, CollisionManifold& OutManifold)
{
	constexpr float SupportEpsilon{ 0.0001f };

	const Vector3 Tangent{ -Normal.y, Normal.x, 0.0f };

	const SupportFeature FeatureA{ CollisionDetector::FindSupportFeatures(PointsA, Normal, true) };
	const SupportFeature FeatureB{ CollisionDetector::FindSupportFeatures(PointsB, Normal, false) };

	if (FeatureA.Points.empty() || FeatureB.Points.empty())
	{
		return false;
	}


	float ATangentMin{};
	float ATangentMax{};

	float BTangentMin{};
	float BTangentMax{};

	CollisionDetector::ProjectFeature(FeatureA.Points, Tangent, ATangentMin, ATangentMax);
	CollisionDetector::ProjectFeature(FeatureB.Points, Tangent, BTangentMin, BTangentMax);

	const float ContactMin{ std::max(ATangentMin, BTangentMin) };
	const float ContactMax{ std::min(ATangentMax, BTangentMax) };

	if (ContactMin > ContactMax + SupportEpsilon)
	{
		// 접촉 영역이 없음
		return false;
	}

	const float ContactNormalDistance{ (FeatureA.Distance + FeatureB.Distance) * 0.5f };

	auto  CreateContactPoint = [&](float TangentDistance)
		{
			return Normal * ContactNormalDistance + Tangent * TangentDistance;
		};

	if (ContactMax - ContactMin <= SupportEpsilon)
	{
		OutManifold.ContactCount = 1;
		OutManifold.Contacts[0].Point = CreateContactPoint((ContactMin + ContactMax) * 0.5f);
	}
	else
	{
		OutManifold.ContactCount = 2;
		OutManifold.Contacts[0].Point = CreateContactPoint(ContactMin);
		OutManifold.Contacts[1].Point = CreateContactPoint(ContactMax);
	}

	return true;
}

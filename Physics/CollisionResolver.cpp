#include "CollisionResolver.h"
#include <algorithm>

void CollisionResolver::ResolvePosition(CollisionManifold& Manifold)
{
	RigidBody& A = *Manifold.ColliderA.RigidBody;
	RigidBody& B = *Manifold.ColliderB.RigidBody;

	const float InverseMassA = A.GetInverseMass();
	const float InverseMassB = B.GetInverseMass();
	const float InverseMassSum = InverseMassA + InverseMassB;

	if (InverseMassSum <= 0.0f)
	{
		return;
	}

	const float Slop{ 0.001f };
	const float CorrectionPercent{ 0.8f };
	const float CorrectionMagnitude = ((Manifold.Penetration > Slop) ? Manifold.Penetration - Slop : 0.0f) / InverseMassSum * CorrectionPercent;
	const Vector3 Correction = Manifold.Normal * CorrectionMagnitude;

	A.ApplyPositionCorrection(Correction * -1.f);
	B.ApplyPositionCorrection(Correction );
}

void CollisionResolver::PrepareConstraints(CollisionManifold& Manifold)
{
	RigidBody& A = *Manifold.ColliderA.RigidBody;
	RigidBody& B = *Manifold.ColliderB.RigidBody;

	const Vector3 VelocityA = A.GetVelocityAtPoint(Manifold.ContactPoint);
	const Vector3 VelocityB = B.GetVelocityAtPoint(Manifold.ContactPoint);

	const Vector3 RelativeVelocity = VelocityB - VelocityA;

	const float VelocityAlongNormal = RelativeVelocity.Dot(Manifold.Normal);

	const float Restitution{ Globals::RESTITUTION_COEFFICIENT };

	constexpr float RestitutionThreshold{ 0.1f };

	if(VelocityAlongNormal < -RestitutionThreshold)
	{
		Manifold.RestitutionBias = -Restitution * VelocityAlongNormal;
	}
	else
	{
		Manifold.RestitutionBias = 0.0f;
	}

	Manifold.AccumulatedNormalImpulse = 0.0f;
	Manifold.AccumulatedFrictionImpulse = 0.0f;
}

void CollisionResolver::ResolveRestitution(CollisionManifold& Manifold)
{
	RigidBody& A = *Manifold.ColliderA.RigidBody;
	RigidBody& B = *Manifold.ColliderB.RigidBody;

	const float InverseMassA = A.GetInverseMass();
	const float InverseMassB = B.GetInverseMass();
	const float InverseMassSum = InverseMassA + InverseMassB;

	if (InverseMassSum <= 0.0f)
	{
		return;
	}

	const Vector3 VelocityA = A.GetVelocityAtPoint(Manifold.ContactPoint);
	const Vector3 VelocityB = B.GetVelocityAtPoint(Manifold.ContactPoint);
	const Vector3 RelativeVelocity = VelocityB - VelocityA;
	const float VelocityAlongNormal = RelativeVelocity.Dot(Manifold.Normal);

	const Vector3 OffsetA = Manifold.ContactPoint - A.GetPosition();
	const Vector3 OffsetB = Manifold.ContactPoint - B.GetPosition();

	const float AngularTermA = OffsetA.Cross(Manifold.Normal) * OffsetA.Cross(Manifold.Normal) * A.GetInverseMomentOfInertia();
	const float AngularTermB = OffsetB.Cross(Manifold.Normal) * OffsetB.Cross(Manifold.Normal) * B.GetInverseMomentOfInertia();
	const float Denominator = InverseMassSum + AngularTermA + AngularTermB;

	if (Denominator <= 0.00001f)
	{
		return;
	}

	float Lambda = -(VelocityAlongNormal - Manifold.RestitutionBias) / Denominator;
	const float OldImpulse = Manifold.AccumulatedNormalImpulse; 

	Manifold.AccumulatedNormalImpulse = std::max(OldImpulse + Lambda, 0.0f);
	
	Lambda = Manifold.AccumulatedNormalImpulse - OldImpulse;
	const Vector3 Impulse = Manifold.Normal * Lambda;

	A.ApplyImpulse(Impulse * -1.f, Manifold.ContactPoint);
	B.ApplyImpulse(Impulse, Manifold.ContactPoint);
}

void CollisionResolver::ResolveFriction(CollisionManifold& Manifold)
{
	RigidBody& A = *Manifold.ColliderA.RigidBody;
	RigidBody& B = *Manifold.ColliderB.RigidBody;

	const Vector3 OffsetA = Manifold.ContactPoint - A.GetPosition();
	const Vector3 OffsetB = Manifold.ContactPoint - B.GetPosition();

	const float InverseMassA = A.GetInverseMass();
	const float InverseMassB = B.GetInverseMass();
	const float InverseMassSum = InverseMassA + InverseMassB;

	if (InverseMassSum <= 0.0f)
	{
		return;
	}

	const float StaticFriction{ CollisionResolver::ResolveFrictionCoefficient(Manifold) };
	const float DynamicFriction{ StaticFriction * 0.8f };

	const Vector3 FrictionVelocityA{ A.GetVelocityAtPoint(Manifold.ContactPoint) };
	const Vector3 FrictionVelocityB{ B.GetVelocityAtPoint(Manifold.ContactPoint) };
	const Vector3 FrictionRelativeVelocity{ FrictionVelocityB - FrictionVelocityA };



	const Vector3 Tangent{ -Manifold.Normal.y, Manifold.Normal.x, 0.f };

	const float OffsetACrossTangent{ OffsetA.Cross(Tangent) };
	const float OffsetBCrossTangent{ OffsetB.Cross(Tangent) };

	const float TangentDenominator{ InverseMassSum + OffsetACrossTangent * OffsetACrossTangent * A.GetInverseMomentOfInertia() + OffsetBCrossTangent * OffsetBCrossTangent * B.GetInverseMomentOfInertia() };

	const float TangentVelocity{ FrictionRelativeVelocity.Dot(Tangent) };
	float Lambda = -TangentVelocity / TangentDenominator;
	const float OldFrictionImpulse = Manifold.AccumulatedFrictionImpulse;
	const float NewFrictionImpulse = OldFrictionImpulse + Lambda;

	const float MaxFrictionImpulse = Manifold.AccumulatedNormalImpulse * StaticFriction;

	if (std::abs(NewFrictionImpulse) <= MaxFrictionImpulse)
	{
		Manifold.AccumulatedFrictionImpulse = NewFrictionImpulse;
	}
	else {
		const float MaxDynamicFrictionImpulse = Manifold.AccumulatedNormalImpulse * DynamicFriction;
		Manifold.AccumulatedFrictionImpulse = std::clamp(NewFrictionImpulse, -MaxDynamicFrictionImpulse, MaxDynamicFrictionImpulse);
	}

	Lambda = Manifold.AccumulatedFrictionImpulse - OldFrictionImpulse;

	const Vector3 FrictionImpulse{ Tangent * Lambda };

	A.ApplyImpulse(FrictionImpulse * -1.0f, Manifold.ContactPoint);
	B.ApplyImpulse(FrictionImpulse, Manifold.ContactPoint);
	
}

float CollisionResolver::ResolveFrictionCoefficient(CollisionManifold& Manifold)
{
	ColliderType TypeA = Manifold.ColliderA.Type;
	ColliderType TypeB = Manifold.ColliderB.Type;

	// 1. Sphere - Sphere
	if (TypeA == ColliderType_Sphere && TypeB == ColliderType_Sphere)
	{
		return Globals::GENERAL_FRICTION_COEFFICIENT;
	}
	// 2. Sphere - Box
	else if ((TypeA == ColliderType_Sphere && TypeB == ColliderType_Box) || (TypeA == ColliderType_Box && TypeB == ColliderType_Sphere))
	{
		return Globals::GROUND_FRICTION_COEFFICIENT;
	}
	// 3. Sphere - Plane
	else if ((TypeA == ColliderType_Sphere && TypeB == ColliderType_Plane) || (TypeA == ColliderType_Plane && TypeB == ColliderType_Sphere))
	{
		return Globals::GROUND_FRICTION_COEFFICIENT;
	}
	else // Undefined Behavior for other combinations, return default friction coefficient
	{
		return Globals::GENERAL_FRICTION_COEFFICIENT;
	}


}

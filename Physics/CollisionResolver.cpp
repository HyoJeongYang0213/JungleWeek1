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
	B.ApplyPositionCorrection(Correction);
}

void CollisionResolver::ResolveCollision(CollisionManifold& Manifold)
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

	if (VelocityAlongNormal >= 0.0f)
	{
		return;
	}

	const Vector3 OffsetA = Manifold.ContactPoint - A.GetPosition();
	const Vector3 OffsetB = Manifold.ContactPoint - B.GetPosition();

	const float AngularTermA = OffsetA.Cross(Manifold.Normal) * OffsetA.Cross(Manifold.Normal) * A.GetInverseMomentOfInertia();
	const float AngularTermB = OffsetB.Cross(Manifold.Normal) * OffsetB.Cross(Manifold.Normal) * B.GetInverseMomentOfInertia();
	const float Denominator = InverseMassSum + AngularTermA + AngularTermB;

	const float RestitutionThreshold{ 0.1f };
	const float Restitution{  std::abs(VelocityAlongNormal) < RestitutionThreshold ? 0.0f : Globals::RESTITUTION_COEFFICIENT };
	const float ImpulseMagnitude = -(1.0f + Restitution) * VelocityAlongNormal / Denominator;
	const Vector3 Impulse = Manifold.Normal * ImpulseMagnitude;

	A.ApplyImpulse(Impulse * -1.f, Manifold.ContactPoint);
	B.ApplyImpulse(Impulse, Manifold.ContactPoint);


	const float StaticFriction{ CollisionResolver::ResolveFrictionCoefficient(Manifold) };
	const float DynamicFriction{ StaticFriction * 0.1f };

	const Vector3 FrictionVelocityA{ A.GetVelocityAtPoint(Manifold.ContactPoint) };
	const Vector3 FrictionVelocityB{ B.GetVelocityAtPoint(Manifold.ContactPoint) };
	const Vector3 FrictionRelativeVelocity{ FrictionVelocityB - FrictionVelocityA };

	const float TangentVelocityAlongNormal{ FrictionRelativeVelocity.Dot(Manifold.Normal) };
	const Vector3 TangentVelocity{ FrictionRelativeVelocity - Manifold.Normal * TangentVelocityAlongNormal };
	const float TangentVelocitySquaredLength{ TangentVelocity.SquaredLength() };

	if (TangentVelocitySquaredLength > 0.0001f)
	{
		const Vector3 Tangent{ TangentVelocity.Normalize() };

		const float OffsetACrossTangent{ OffsetA.Cross(Tangent) };
		const float OffsetBCrossTangent{ OffsetB.Cross(Tangent) };

		const float TangentDenominator{ InverseMassSum + OffsetACrossTangent * OffsetACrossTangent * A.GetInverseMomentOfInertia() + OffsetBCrossTangent * OffsetBCrossTangent * B.GetInverseMomentOfInertia() };
		const float UnclampedFrictionImpulseMagnitude{ -FrictionRelativeVelocity.Dot(Tangent) / TangentDenominator };
		const float StaticFrictionLimit{ ImpulseMagnitude * StaticFriction };
		const float FrictionImpulseMagnitude{ fabsf(UnclampedFrictionImpulseMagnitude) <= StaticFrictionLimit ? UnclampedFrictionImpulseMagnitude : (UnclampedFrictionImpulseMagnitude < 0.0f ? -1.0f : 1.0f) * ImpulseMagnitude * DynamicFriction };

		const Vector3 FrictionImpulse{ Tangent * FrictionImpulseMagnitude };

		A.ApplyImpulse(FrictionImpulse * -1.0f, Manifold.ContactPoint);
		B.ApplyImpulse(FrictionImpulse, Manifold.ContactPoint);
	}

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

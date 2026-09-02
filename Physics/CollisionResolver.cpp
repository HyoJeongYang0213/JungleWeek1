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

	const float Slop{ 0.005f };
	const float CorrectionPercent{ 0.2f };
	const float CorrectionMagnitude = ((Manifold.Penetration > Slop) ? Manifold.Penetration - Slop : 0.0f) / InverseMassSum * CorrectionPercent;
	const Vector3 Correction = Manifold.Normal * CorrectionMagnitude;

	A.ApplyPositionCorrection(Correction * -1.f);
	B.ApplyPositionCorrection(Correction );
}

void CollisionResolver::PrepareConstraints(CollisionManifold& Manifold)
{
	RigidBody& A = *Manifold.ColliderA.RigidBody;
	RigidBody& B = *Manifold.ColliderB.RigidBody;

	const float Restitution { PhysicsGlobals::RESTITUTION_COEFFICIENT };

	constexpr float RestitutionThreshold { PhysicsGlobals::RESTITUTION_VELOCITY_THRESHOLD };

	for (size_t ContactIndex = 0; ContactIndex < Manifold.ContactCount; ++ContactIndex)
	{
		ContactConstraint& Contact{ Manifold.Contacts[ContactIndex] };

		const Vector3 VelocityA { A.GetVelocityAtPoint(Contact.Point) };
		const Vector3 VelocityB { B.GetVelocityAtPoint(Contact.Point) };

		const Vector3 RelativeVelocity { VelocityB - VelocityA };

		const float VelocityAlongNormal { RelativeVelocity.Dot(Manifold.Normal) };

		if (VelocityAlongNormal < -RestitutionThreshold)
		{
			Contact.RestitutionBias = -Restitution * VelocityAlongNormal;
		}
		else
		{
			Contact.RestitutionBias = 0.0f;
		}

		Contact.AccumulatedNormalImpulse = 0.0f;
		Contact.AccumulatedFrictionImpulse = 0.0f;
	}
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

	for (size_t ContactIndex = 0; ContactIndex < Manifold.ContactCount; ++ContactIndex)
	{
		ContactConstraint& Contact{ Manifold.Contacts[ContactIndex] };

		const Vector3 VelocityA { A.GetVelocityAtPoint(Contact.Point) };
		const Vector3 VelocityB { B.GetVelocityAtPoint(Contact.Point) };

		const Vector3 RelativeVelocity { VelocityB - VelocityA };
		const float VelocityAlongNormal { RelativeVelocity.Dot(Manifold.Normal) };

		const Vector3 OffsetA { Contact.Point - A.GetPosition() };
		const Vector3 OffsetB { Contact.Point - B.GetPosition() };

		const float OffsetACrossNormal{ OffsetA.Cross(Manifold.Normal) };
		const float OffsetBCrossNormal{ OffsetB.Cross(Manifold.Normal) };

		const float AngularTermA{ OffsetACrossNormal * OffsetACrossNormal * A.GetInverseMomentOfInertia() };
		const float AngularTermB { OffsetBCrossNormal * OffsetBCrossNormal * B.GetInverseMomentOfInertia() };

		const float Denominator { InverseMassSum + AngularTermA + AngularTermB };

		if (Denominator <= 0.00001f)
		{
			continue;
		}

		float Lambda = -(VelocityAlongNormal - Contact.RestitutionBias) / Denominator;

		const float OldImpulse { Contact.AccumulatedNormalImpulse };

		Contact.AccumulatedNormalImpulse = std::max(OldImpulse + Lambda, 0.0f);

		Lambda = Contact.AccumulatedNormalImpulse - OldImpulse;

		const Vector3 Impulse { Manifold.Normal * Lambda };

		A.ApplyImpulse(Impulse * -1.0f, Contact.Point);
		B.ApplyImpulse(Impulse, Contact.Point);
	}
}

void CollisionResolver::ResolveFriction(CollisionManifold& Manifold)
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

	const float StaticFriction{ CollisionResolver::ResolveFrictionCoefficient(Manifold) };
	const float DynamicFriction { StaticFriction * 0.8f };

	const Vector3 Tangent { -Manifold.Normal.y, Manifold.Normal.x, 0.0f };

	for (size_t ContactIndex = 0; ContactIndex < Manifold.ContactCount; ++ContactIndex)
	{
		ContactConstraint& Contact { Manifold.Contacts[ContactIndex] };

		const Vector3 OffsetA { Contact.Point - A.GetPosition() };
		const Vector3 OffsetB { Contact.Point - B.GetPosition() };

		const Vector3 FrictionVelocityA { A.GetVelocityAtPoint(Contact.Point) };
		const Vector3 FrictionVelocityB { B.GetVelocityAtPoint(Contact.Point) };
		const Vector3 FrictionRelativeVelocity { FrictionVelocityB - FrictionVelocityA };

		const float OffsetACrossTangent { OffsetA.Cross(Tangent) };
		const float OffsetBCrossTangent { OffsetB.Cross(Tangent) };

		const float TangentDenominator { InverseMassSum + OffsetACrossTangent * OffsetACrossTangent * A.GetInverseMomentOfInertia() + OffsetBCrossTangent * OffsetBCrossTangent * B.GetInverseMomentOfInertia() };

		if (TangentDenominator <= 0.00001f)
		{
			continue;
		}

		const float TangentVelocity { FrictionRelativeVelocity.Dot(Tangent) };

		float Lambda = -TangentVelocity / TangentDenominator;

		const float OldFrictionImpulse { Contact.AccumulatedFrictionImpulse };
		const float NewFrictionImpulse { OldFrictionImpulse + Lambda };

		const float MaxFrictionImpulse { Contact.AccumulatedNormalImpulse * StaticFriction };

		if (std::abs(NewFrictionImpulse) <= MaxFrictionImpulse)
		{
			Contact.AccumulatedFrictionImpulse = NewFrictionImpulse;
		}
		else
		{
			const float MaxDynamicFrictionImpulse { Contact.AccumulatedNormalImpulse * DynamicFriction };

			Contact.AccumulatedFrictionImpulse = std::clamp(NewFrictionImpulse, -MaxDynamicFrictionImpulse, MaxDynamicFrictionImpulse);
		}

		Lambda = Contact.AccumulatedFrictionImpulse - OldFrictionImpulse;

		const Vector3 FrictionImpulse { Tangent * Lambda };

		A.ApplyImpulse(FrictionImpulse * -1.0f, Contact.Point);
		B.ApplyImpulse(FrictionImpulse, Contact.Point);
	}
}

float CollisionResolver::ResolveFrictionCoefficient(CollisionManifold& Manifold)
{
	ColliderType TypeA = Manifold.ColliderA.Type;
	ColliderType TypeB = Manifold.ColliderB.Type;

	// 1. Sphere - Sphere
	if (TypeA == ColliderType_Sphere && TypeB == ColliderType_Sphere)
	{
		return PhysicsGlobals::GENERAL_FRICTION_COEFFICIENT;
	}
	// 2. Sphere - Box
	else if ((TypeA == ColliderType_Sphere && TypeB == ColliderType_Box) || (TypeA == ColliderType_Box && TypeB == ColliderType_Sphere))
	{
		return PhysicsGlobals::GROUND_FRICTION_COEFFICIENT;
	}
	// 3. Sphere - Plane
	else if ((TypeA == ColliderType_Sphere && TypeB == ColliderType_Plane) || (TypeA == ColliderType_Plane && TypeB == ColliderType_Sphere))
	{
		return PhysicsGlobals::GROUND_FRICTION_COEFFICIENT;
	}
	else // Undefined Behavior for other combinations, return default friction coefficient
	{
		return PhysicsGlobals::GENERAL_FRICTION_COEFFICIENT;
	}
}

#include "RigidBody.h"

RigidBody::RigidBody(Vector3& Position, Vector3& Velocity, float& Mass, float MomentOfInertia) :
	Position(Position), Velocity(Velocity), Mass(Mass), MomentOfInertia(MomentOfInertia) {
}

void RigidBody::SetMomentOfInertia(float moment)
{
	MomentOfInertia = moment;
}

void RigidBody::SetRotation(float rotation) 
{
	Rotation = rotation;
}

void RigidBody::AddForce(const Vector3& Force)
{
	AccumulatedForce += Force;
}

void RigidBody::AddTorque(float Torque)
{
	AccumulatedTorque += Torque;
}

void RigidBody::Integrate(float dt) 
{
	const float InverseMass{ GetInverseMass() };
	const float InverseMomentOfInertia{ GetInverseMomentOfInertia() };

	Velocity.x += AccumulatedForce.x * InverseMass * dt;
	Velocity.y += AccumulatedForce.y * InverseMass * dt;


	AngularVelocity += AccumulatedTorque * InverseMomentOfInertia * dt;
	Rotation += AngularVelocity * dt;
	

	Position.x += Velocity.x * dt;
	Position.y += Velocity.y * dt;

	AccumulatedForce = {};
	AccumulatedTorque = {};
}

void RigidBody::ApplyImpulse(const Vector3& Impulse, const Vector3& ContactPoint)
{
	const Vector3 Offset{ ContactPoint.x - Position.x, ContactPoint.y - Position.y, 0.f };
	const float InverseMass{ GetInverseMass() };
	const float InverseMomentOfInertia{ GetInverseMomentOfInertia() };

	Velocity.x += Impulse.x * InverseMass;
	Velocity.y += Impulse.y * InverseMass;

	AngularVelocity += Offset.Cross(Impulse) * InverseMomentOfInertia;
}

void RigidBody::ApplyPositionCorrection(const Vector3& Correction)
{
	Position += Correction * GetInverseMass();
}

Vector3 RigidBody::GetVelocityAtPoint(const Vector3& ContactPoint) const
{
	const Vector3 Offset{ ContactPoint - Position };
	const Vector3 RotationalVelocity{ -AngularVelocity * Offset.y, AngularVelocity * Offset.x, 0.0f };

	return Velocity + RotationalVelocity;
}

float RigidBody::GetAngularVelocity() const
{
	return AngularVelocity;
}

float RigidBody::GetRotation() const
{
	return Rotation;
}

float RigidBody::GetInverseMass() const
{
	return Mass > 0.0f ? 1.0f / Mass : 0.0f;
}

float RigidBody::GetInverseMomentOfInertia() const
{
	return MomentOfInertia > 0.0f ? 1.0f / MomentOfInertia : 0.0f;
}

const Vector3& RigidBody::GetPosition() const
{
	return Position;
}

const Vector3& RigidBody::GetVelocity() const
{
	return Velocity;
}

float RigidBody::GetMass() const
{
	return Mass;
}

void RigidBody::ApplyRotateResistance(const Vector3& SurfaceNormal, float resistanceCoefficient, float dt)
{
	const Vector3 tangent{ -SurfaceNormal.y, SurfaceNormal.x, 0.0f };

	const float damping = std::clamp(resistanceCoefficient * dt, 0.0f, 1.0f);
	const float tangentSpeed = Velocity.Dot(tangent);

	Velocity -= tangent * tangentSpeed * damping;
	AngularVelocity -= AngularVelocity * damping;

	if (std::abs(tangentSpeed) < 0.05f)
	{
		Velocity -= tangent * tangentSpeed;
		AngularVelocity = 0.0f;
	}
}

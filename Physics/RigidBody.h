#pragma once 
#include "../Utils/Math.hpp"


class RigidBody
{
public:
	RigidBody(Vector3& Position, Vector3& Velocity, float& Mass, float MomentOfInertia); 
	~RigidBody() = default;

	RigidBody(const RigidBody&) = delete;
	RigidBody& operator=(const RigidBody&) = delete;

	RigidBody(RigidBody&&) = default;
	RigidBody& operator=(RigidBody&&) = default;

public:
	void Integrate(float dt); 

	void AddForce(const Vector3& Force);
	void AddTorque(float Torque);
	void ApplyImpulse(const Vector3& Impulse, const Vector3& ContactPoint);
	void ApplyPositionCorrection(const Vector3& Correction);

	void SetMomentOfInertia(float moment); 
	void SetRotation(float rotation);

	float GetAngularVelocity() const; 
	float GetRotation() const;
	Vector3 GetVelocityAtPoint(const Vector3& ContactPoint) const;
	float GetInverseMass() const;
	float GetInverseMomentOfInertia() const;

	const Vector3& GetPosition() const;
	const Vector3& GetVelocity() const;
	float GetMass() const;


	// 매우 위험. 복사된 RigidBody 인 경우에 한해 사용 할 것. 원본 침해의 가능성 있음. 
	void SetPosition(const Vector3& position) { Position = position; }

private:
	Vector3& Position;
	Vector3& Velocity;
	float& Mass;

	Vector3 AccumulatedForce{};
	float AccumulatedTorque{};

	float AngularVelocity{};
	float Rotation{};
	float MomentOfInertia{};

};
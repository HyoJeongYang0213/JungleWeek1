#pragma once 
#include "RigidBody.h"

class Collider
{
public:
	Collider(RigidBody& r, float& rad); 
	~Collider() = default;

	Collider(const Collider&) = delete;
	Collider& operator=(const Collider&) = delete;

	Collider(Collider&&) = default;
	Collider& operator=(Collider&&) = default;

public: 
	RigidBody& GetRigidBody();
	const RigidBody& GetRigidBody() const;

	const Vector3& GetCenter() const;
	float GetRadius() const;

private:
	RigidBody& mRigidBody;
	float& mRadius;
};

class StaticCollider
{
public:
	StaticCollider(const Vector3& pos, const Vector3& norm); 
	~StaticCollider() = default;

	StaticCollider(const StaticCollider&) = delete;
	StaticCollider& operator=(const StaticCollider&) = delete;

	StaticCollider(StaticCollider&&) = default;
	StaticCollider& operator=(StaticCollider&&) = default;

public:
	RigidBody& GetRigidBody();
	const RigidBody& GetRigidBody() const;

	const Vector3& GetPosition() const;
	const Vector3& GetNormal() const;

public:
	Vector3 mPosition;
	Vector3 mVelocity;
	float mMass;

	RigidBody mRigidBody;
	Vector3 mNormal;

};
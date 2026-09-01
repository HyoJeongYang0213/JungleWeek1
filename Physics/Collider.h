#pragma once 
#include "RigidBody.h"
#include "ColliderType.hpp"

class ICollider {
public:
	virtual ~ICollider() = default;

public:
	virtual ColliderType GetColliderType() const = 0;
};

class SphereCollider : public ICollider
{
public:
	SphereCollider(RigidBody& r, float& rad); 
	~SphereCollider() = default;

	SphereCollider(const SphereCollider&) = delete;
	SphereCollider& operator=(const SphereCollider&) = delete;

	SphereCollider(SphereCollider&&) = default;
	SphereCollider& operator=(SphereCollider&&) = default;

public: 
	virtual ColliderType GetColliderType() const override { return ColliderType_Sphere; }
	
	RigidBody& GetRigidBody();
	const RigidBody& GetRigidBody() const;

	const Vector3& GetCenter() const;
	float GetRadius() const;

	SphereCollider Clone(); 

private:
	RigidBody& mRigidBody;
	float& mRadius;
};

class BoxCollider : public ICollider
{
public:
	BoxCollider(RigidBody& r, Vector3& halfExtents);
	~BoxCollider() = default;
	
	BoxCollider(const BoxCollider&) = delete;
	BoxCollider& operator=(const BoxCollider&) = delete;
	
	BoxCollider(BoxCollider&&) = default;
	BoxCollider& operator=(BoxCollider&&) = default;

public:
	virtual ColliderType GetColliderType() const override { return ColliderType_Box; }

	RigidBody& GetRigidBody();
	const RigidBody& GetRigidBody() const;

	const Vector3& GetHalfExtents() const;


	bool IsPointInside(const Vector3& point) const;
private:
	RigidBody& mRigidBody;
	Vector3& mHalfExtents;
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
	float mMass{ 0.f };

	RigidBody mRigidBody;
	Vector3 mNormal;

};
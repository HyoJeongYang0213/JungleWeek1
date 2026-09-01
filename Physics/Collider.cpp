#include "Collider.h"

SphereCollider::SphereCollider(RigidBody& r, float& rad) : mRigidBody(r), mRadius(rad) 
{
}

RigidBody& SphereCollider::GetRigidBody()
{
	return mRigidBody;
}

const RigidBody& SphereCollider::GetRigidBody() const
{
	return mRigidBody;
}

const Vector3& SphereCollider::GetCenter() const
{
	return mRigidBody.GetPosition();
}

float SphereCollider::GetRadius() const
{
	return mRadius;
}



BoxCollider::BoxCollider(RigidBody& r, Vector3& halfExtents) : mRigidBody(r), mHalfExtents(halfExtents)
{
}

RigidBody& BoxCollider::GetRigidBody()
{
	return  mRigidBody;
}

const RigidBody& BoxCollider::GetRigidBody() const
{
	return mRigidBody;
}

const Vector3& BoxCollider::GetHalfExtents() const
{
	return mHalfExtents;
}




StaticCollider::StaticCollider(const Vector3& pos, const Vector3& norm) :
	mPosition(pos), mVelocity{}, mMass{}, mRigidBody(mPosition, mVelocity, mMass, 0.0f), mNormal(norm.Normalize()) 
{
}

RigidBody& StaticCollider::GetRigidBody()
{
	return mRigidBody;
}

const RigidBody& StaticCollider::GetRigidBody() const
{
	return mRigidBody;
}

const Vector3& StaticCollider::GetPosition() const
{
	return mPosition;
}

const Vector3& StaticCollider::GetNormal() const
{
	return mNormal; 
}





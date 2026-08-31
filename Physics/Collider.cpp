#include "Collider.h"

Collider::Collider(RigidBody& r, float& rad) : mRigidBody(r), mRadius(rad) 
{
}

RigidBody& Collider::GetRigidBody()
{
	return mRigidBody;
}

const RigidBody& Collider::GetRigidBody() const
{
	return mRigidBody;
}

const Vector3& Collider::GetCenter() const
{
	return mRigidBody.GetPosition();
}

float Collider::GetRadius() const
{
	return mRadius;
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



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

SphereCollider SphereCollider::Clone() 
{
	return SphereCollider(mRigidBody, mRadius);
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

bool BoxCollider::IsPointInside(const Vector3& point) const 
{
	return (point.x >= mRigidBody.GetPosition().x - mHalfExtents.x and point.x <= mRigidBody.GetPosition().x + mHalfExtents.x) &&
		(point.y >= mRigidBody.GetPosition().y - mHalfExtents.y and point.y <= mRigidBody.GetPosition().y + mHalfExtents.y);
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





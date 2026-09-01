#include "Platform.h"

Platform::Platform(ID3D11Buffer* vb, UINT numVertices)
	: mVertexBuffer{vb}, mNumVertices{numVertices},
	mRigidBody{ mLocation, mVelocity, mMass, 1.0f }, mCollider{ mRigidBody, mHalfExtents }
{
	mRigidBody.SetRotation(Rnd::GetRandom(0.f, 360.f));
}

Platform::~Platform()
{
}

void Platform::Tick(float t)
{
}

ICollider& Platform::GetCollider()
{
	return mCollider;
}

RigidBody& Platform::GetRigidBody()
{
	return mRigidBody;
}

void Platform::Render(IRenderer& renderer)
{
	renderer.UpdateConstant(mLocation, mHalfExtents, mRigidBody.GetRotation());
	renderer.RenderPrimitive(mVertexBuffer, mNumVertices);
}

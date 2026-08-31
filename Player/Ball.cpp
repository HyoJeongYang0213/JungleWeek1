#include "Ball.h"

int Ball::TotalNumBalls = 0;


Ball::Ball(ID3D11Buffer* vb, UINT numVertices)
	: mLocation{ Rnd::GetRandomFloat(-1.0f, 1.0f), Rnd::GetRandomFloat(-1.0f, 1.0f), 0.0f },
	mVelocity{ Rnd::GetRandomFloat(-3.0f, 3.0f), Rnd::GetRandomFloat(-3.0f, 3.0f), 0.0f },
	mRadius{ Rnd::GetRandomFloat(0.05f, 0.15f) },
	mMass{ 0.1f * mRadius * mRadius * 3.14159f },
	mRigidBody{ mLocation, mVelocity, mMass, 0.5f * mMass * mRadius * mRadius },
	mCollider{ mRigidBody, mRadius },
	mVertexBuffer{ vb }, mNumVertices{ numVertices }
{
	TotalNumBalls++;
}
Ball::~Ball()
{
	TotalNumBalls--;
}

void Ball::Tick(float t)
{
	mRigidBody.Integrate(t);
}

ICollider& Ball::GetCollider() 
{
	return mCollider;
}

RigidBody& Ball::GetRigidBody()
{
	return mRigidBody;
}

void Ball::Render(IRenderer& renderer)
{
	renderer.UpdateConstant(mLocation, Vector3{ mRadius, mRadius, 0.f }, mRigidBody.GetRotation());
	renderer.RenderPrimitive(mVertexBuffer, mNumVertices);
}

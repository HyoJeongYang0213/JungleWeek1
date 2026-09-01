#include "Ball.h"
#include "PlayerGlobals.hpp"

int Ball::TotalNumBalls = 0;


Ball::Ball(ID3D11Buffer* vb, UINT numVertices)
	: mLocation{ Rnd::GetRandom(0.0f, 40.0f), Rnd::GetRandom(0.0f, 15.0f), 0.0f },
	mVelocity{ Rnd::GetRandom(-3.0f, 3.0f), Rnd::GetRandom(-3.0f, 3.0f), 0.0f },
	mRadius{ 0.5f },
	mMass{ 1.f * mRadius * mRadius * 3.14159f },
	mRigidBody{ mLocation, mVelocity, mMass, 0.5f * mMass * mRadius * mRadius },
	mCollider{ mRigidBody, mRadius },
	mVertexBuffer{ vb }, mNumVertices{ numVertices }
{
	TotalNumBalls++;
	PlayerGlobals::PLAYERBALL = this;
}
Ball::~Ball()
{
	TotalNumBalls--;
	PlayerGlobals::PLAYERBALL = nullptr;
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

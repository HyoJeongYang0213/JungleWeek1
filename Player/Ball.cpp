#include "Ball.h"
#include "PlayerGlobals.hpp"

int Ball::TotalNumBalls = 0;


Ball::Ball(ID3D11Buffer* vb, UINT numVertices)
	: mLocation{ 7.5f,20.0f, 0.0f },
	mVelocity{ 1.f,0.f, 0.f},
	mRadius{ 0.8f },
	mMass{ 0.4f * mRadius * mRadius * 3.14159f },
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

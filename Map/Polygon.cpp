#include "Polygon.h"

PPolygon::PPolygon(ID3D11Buffer* vb, UINT numVertices, const Vector3& position, const std::vector<Vector3>& vertices)
	: mLocation{ position },
	mVelocity{ 0.f, 0.f, 0.f },
	mMass{ 5.f },
	mRigidBody{ mLocation, mVelocity, mMass, 1.f },
	mCollider{ mRigidBody, vertices },
	mVertexBuffer{ vb }, mNumVertices{ numVertices }
{
}

void PPolygon::Tick(float t)
{
	mRigidBody.Integrate(t);
}

ICollider& PPolygon::GetCollider()
{
	return mCollider;
}

RigidBody& PPolygon::GetRigidBody()
{
	return mRigidBody;
}

void PPolygon::Render(IRenderer& renderer)
{
	renderer.UpdateConstant(mLocation, Vector3{ 1.f, 1.f, 0.f }, mRigidBody.GetRotation());
	renderer.RenderPrimitive(mVertexBuffer, mNumVertices);
}

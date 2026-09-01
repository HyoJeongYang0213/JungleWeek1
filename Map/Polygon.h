#pragma once 

#include <vector>

#include "../Resource/Primitive.hpp"
#include "../Utils/Math.hpp"

#include "../Physics/RigidBody.h"
#include "../Physics/Collider.h"

#include "../Renderer/IRenderer.hpp"


class PPolygon : public Primitive {
public:
	PPolygon(ID3D11Buffer* vb, UINT numVertices, const Vector3& position, const std::vector<Vector3>& vertices);
	~PPolygon() override = default;

	PPolygon(const PPolygon&) = delete;
	PPolygon& operator=(const PPolygon&) = delete;

	PPolygon(PPolygon&&) = default;
	PPolygon& operator=(PPolygon&&) = default;

public:
	virtual void Tick(float t) override;
	virtual ICollider& GetCollider() override;
	virtual RigidBody& GetRigidBody() override;
	virtual void Render(IRenderer& renderer) override;

private:

	Vector3 mLocation{};
	Vector3 mVelocity{};

	float mMass{ 0.f };

	RigidBody mRigidBody;
	PolygonCollider mCollider;

	ID3D11Buffer* mVertexBuffer;
	UINT mNumVertices;

};
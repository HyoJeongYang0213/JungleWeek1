#pragma once 

#include <string>
#include <vector>

#include "../Resource/Primitive.hpp"
#include "../Resource/vertexSimple.hpp"
#include "../Utils/Math.hpp"

#include "../Physics/RigidBody.h"
#include "../Physics/Collider.h"

#include "../Renderer/IRenderer.hpp"

struct PolygonImageData
{
	std::vector<VertexTexture> Vertices{};
	UINT NumVertices{};
	std::vector<Vector3> Positions{};
};

class PPolygon : public Primitive {
public:
	PPolygon(ID3D11Buffer* vb, UINT numVertices, const Vector3& position, const std::vector<Vector3>& vertices, bool isStatic, UINT vertexStride = sizeof(VertexSimple));
	PPolygon(ID3D11Buffer* vb, UINT numVertices, const Vector3& center, const Vector3& halfExtents, const std::vector<Vector3>& vertices, bool isStatic, UINT vertexStride = sizeof(VertexSimple));
	~PPolygon() override = default;

	PPolygon(const PPolygon&) = delete;
	PPolygon& operator=(const PPolygon&) = delete;

	PPolygon(PPolygon&&) = default;
	PPolygon& operator=(PPolygon&&) = default;

public:
	static PolygonImageData CreateGeometryFromImage(const std::string& imagePath, unsigned char alphaThreshold = 128);

	virtual void Tick(float t) override;
	
	virtual ICollider& GetCollider() override;
	virtual RigidBody& GetRigidBody() override;

	virtual void Render(IRenderer& renderer) override;

	void SetPosition(const Vector3& position) { mLocation = position; }	
private:

	Vector3 mLocation{};
	Vector3 mHalfExtents{ 1.f, 1.f, 0.f };
	Vector3 mVelocity{};

	float mMass{ 0.f };

	RigidBody mRigidBody;
	PolygonCollider mCollider;

	ID3D11Buffer* mVertexBuffer;
	UINT mNumVertices;
	UINT mVertexStride;

};

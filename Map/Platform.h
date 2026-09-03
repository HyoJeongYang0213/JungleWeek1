#pragma once 

#include <d3d11.h>


#include "../Resource/Primitive.hpp"
#include "../Utils/Math.hpp"

#include "../Physics/RigidBody.h"
#include "../Physics/Collider.h"

#include "../Renderer/IRenderer.hpp"

#include "../Utils/Rnd.hpp"

class Platform : public Primitive
{
public:
	Platform(ID3D11Buffer* vb, UINT numVertices);
	Platform(ID3D11Buffer* vb, UINT numVertices, const Vector3& location, const Vector3& halfExtents); 

	virtual ~Platform() override;

	Platform(const Platform&) = delete;
	Platform& operator=(const Platform&) = delete;

	Platform(Platform&&) = default;
	Platform& operator=(Platform&&) = default;

public:
	virtual void Tick(float t) override;

	virtual ICollider& GetCollider() override;
	virtual RigidBody& GetRigidBody() override;

	virtual void Render(IRenderer& renderer) override;
private:
	//Vector3 mLocation{ Rnd::GetRandomFloat(-0.8f, 0.8f), Rnd::GetRandomFloat(-0.8f, 0.8f), 0.0f };
	Vector3 mLocation{ 0.f, 0.f, 0.f };
	Vector3 mVelocity{ 0.f, 0.f, 0.f };
	//Vector3 mHalfExtents{ Rnd::GetRandomFloat(0.1f, 0.2f), Rnd::GetRandomFloat(0.1f, 0.2f), 0.f };
	Vector3 mHalfExtents{ 0.1f, 0.1f, 0.f };
	float mMass{ 0.f };
	
	RigidBody mRigidBody;
	BoxCollider mCollider;

	ID3D11Buffer* mVertexBuffer;
	UINT mNumVertices;
};

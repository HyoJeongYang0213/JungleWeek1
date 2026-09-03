#pragma once 

#include <d3d11.h>

#include "../Resource/Primitive.hpp"
#include "../Utils/Math.hpp"
#include "../Physics/RigidBody.h"
#include "../Physics/Collider.h"
#include "../Utils/Rnd.hpp"

#include "../Renderer/IRenderer.hpp"

class Ball : public Primitive
{
public:
	Ball(ID3D11Buffer* vb, UINT numVertices);
	virtual ~Ball() override; 

	Ball(const Ball&) = delete;
	Ball& operator=(const Ball&) = delete;

	Ball(Ball&&) = default;
	Ball& operator=(Ball&&) = default;

public:
	virtual void Tick(float t) override; 

	virtual ICollider& GetCollider() override;
	virtual RigidBody& GetRigidBody() override;


	virtual void Render(IRenderer& renderer) override;

	Vector3 GetLocation() const { return mLocation; }
	Vector3 GetVelocity() const { return mVelocity; }
	float GetRadius() const { return mRadius; }
	float GetMass() const { return mMass; }

private: 
	// 클래스 이름과, 아래 다섯개의 변수 이름은 변경하지 않습니다.
	Vector3 mLocation;
	Vector3 mVelocity;
	float mRadius;
	float mMass;
	static int TotalNumBalls;

	RigidBody mRigidBody;
	SphereCollider mCollider;

	ID3D11Buffer* mVertexBuffer;
	UINT mNumVertices;

};

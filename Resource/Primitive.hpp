#pragma once 

class Primitive
{
public:
	virtual ~Primitive() = default;

	virtual void Tick(float t) = 0;
	virtual void Render(class IRenderer& renderer) = 0;

	virtual class ICollider& GetCollider() = 0;
	virtual class RigidBody& GetRigidBody() = 0;
};



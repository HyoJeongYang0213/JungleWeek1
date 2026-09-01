#pragma once 

#include <vector>
#include <iterator>
#include <stdexcept>

#include "RigidBody.h"
#include "ColliderType.hpp"

class ICollider {
public:
	virtual ~ICollider() = default;

public:
	virtual ColliderType GetColliderType() const = 0;
};

class SphereCollider : public ICollider
{
public:
	SphereCollider(RigidBody& r, float& rad); 
	~SphereCollider() = default;

	SphereCollider(const SphereCollider&) = delete;
	SphereCollider& operator=(const SphereCollider&) = delete;

	SphereCollider(SphereCollider&&) = default;
	SphereCollider& operator=(SphereCollider&&) = default;

public: 
	virtual ColliderType GetColliderType() const override { return ColliderType_Sphere; }
	
	RigidBody& GetRigidBody();
	const RigidBody& GetRigidBody() const;

	const Vector3& GetCenter() const;
	float GetRadius() const;

	SphereCollider Clone(); 

private:
	RigidBody& mRigidBody;
	float& mRadius;
};

class BoxCollider : public ICollider
{
public:
	BoxCollider(RigidBody& r, Vector3& halfExtents);
	~BoxCollider() = default;
	
	BoxCollider(const BoxCollider&) = delete;
	BoxCollider& operator=(const BoxCollider&) = delete;
	
	BoxCollider(BoxCollider&&) = default;
	BoxCollider& operator=(BoxCollider&&) = default;

public:
	virtual ColliderType GetColliderType() const override { return ColliderType_Box; }

	RigidBody& GetRigidBody();
	const RigidBody& GetRigidBody() const;

	const Vector3& GetHalfExtents() const;


	bool IsPointInside(const Vector3& point) const;
private:
	RigidBody& mRigidBody;
	Vector3& mHalfExtents;
};

class PolygonCollider : public ICollider {
	template <typename Iterator>
	class Subrange
	{
	public:
		Subrange(Iterator begin, Iterator end)
			: Begin(begin)
			, End(end)
		{
		}

		Iterator begin() const
		{
			return Begin;
		}

		Iterator end() const
		{
			return End;
		}

		bool empty() const
		{
			return Begin == End;
		}

	private:
		Iterator Begin;
		Iterator End;
	};

public:
	PolygonCollider(RigidBody& r, const std::vector<Vector3>& points);
	~PolygonCollider() = default;

	PolygonCollider(const PolygonCollider&) = delete;
	PolygonCollider& operator=(const PolygonCollider&) = delete;

	PolygonCollider(PolygonCollider&&) = default;
	PolygonCollider& operator=(PolygonCollider&&) = default;

public:
	virtual  ColliderType GetColliderType() const override { return ColliderType_Polygon; }

	RigidBody& GetRigidBody();
	const  RigidBody& GetRigidBody() const;

	const std::vector<Vector3>& GetPoints() const;

private:
	void CreateConvexHull(const std::vector<Vector3>& points);
	
	template<typename T> 
	inline void MakeConvexHull(T begin, T end, std::vector<Vector3>& hull, size_t endCount) {
		for (const auto& p : Subrange(begin, end)) {

			while (hull.size() >= endCount) {
				Vector3 a = hull[hull.size() - 2];
				Vector3 b = hull[hull.size() - 1];

				Vector3 ab = b - a;
				Vector3 ap = p - a;

				if (ab.Cross(ap) <= 0.f) {
					hull.pop_back();
				}
				else {
					break;
				}
			}

			hull.push_back(p);
		}
	}

private:
	RigidBody& mRigidBody;
	std::vector<Vector3> mPoints{};
};


class StaticCollider
{
public:
	StaticCollider(const Vector3& pos, const Vector3& norm); 
	~StaticCollider() = default;

	StaticCollider(const StaticCollider&) = delete;
	StaticCollider& operator=(const StaticCollider&) = delete;

	StaticCollider(StaticCollider&&) = default;
	StaticCollider& operator=(StaticCollider&&) = default;

public:
	RigidBody& GetRigidBody();
	const RigidBody& GetRigidBody() const;

	const Vector3& GetPosition() const;
	const Vector3& GetNormal() const;

public:
	Vector3 mPosition;
	Vector3 mVelocity;
	float mMass{ 0.f };

	RigidBody mRigidBody;
	Vector3 mNormal;

};

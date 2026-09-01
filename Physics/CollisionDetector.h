#pragma once 
#include "Collider.h"
#include "CollisionManifold.hpp"

/*
*  AABB 와 원 사이의 충돌 Manifold 검출 과정
AABB + Circle 입력
       │
       ▼
Circle Center가 AABB 내부인지 검사
       │
       ├─────────────── 아니오 ──────────────┐
       │                                    │
       │                                    ▼
       │                              Closest Point 계산
       │                                    │
       │                                    ▼
       │                            Circle Center와 거리 계산
       │                                    │
       │                              Radius보다 큰가?
       │                               │           │
       │                              Yes          No
       │                               │           │
       │                           충돌 없음        ▼
       │                                      Normal 계산
       │                                      Penetration 계산
       │                                      ContactPoint 결정
       │
       │
       └─────────────── 예 ──────────────────┐
                                             │
                                             ▼
                                    가장 가까운 면 탐색
                                             │
                                             ▼
                                    탈출 Normal 결정
                                             │
                                             ▼
                               Penetration = 면까지 거리 + Radius
                                             │
                                             ▼
                                   면 위 ContactPoint 결정
                                             │
                         ┌───────────────────┘
                         ▼
               CollisionManifold 완성



 => OBB 와 원 사이의 충돌 Manifold 검출은 AABB와 원 사이의 충돌 Manifold 검출 이전 과정에, 원을 OBB 의 Local Space 로 변환하여 AABB와 원 사이의 충돌 Manifold 검출을 수행하면 된다.

*/

#ifdef max 
#undef max
#endif 


class CollisionDetector
{
public:
    struct ConvexCollisionData {
		std::vector<Vector3> Points;
        std::vector<Vector3> Axes; 
    };

    struct SupportFeature {
		std::vector<Vector3> Points;
		float Distance{ 0.0f };
    };


    struct SATResult {
        Vector3 Normal;
        float Penetration{ std::numeric_limits<float>::max() };
    };

public:
	static bool FindCollision(SphereCollider& Ca, SphereCollider& Cb, CollisionManifold& OutManifold); 
	static bool FindCollision(SphereCollider& c, StaticCollider& s, CollisionManifold& OutManifold);
	static bool FindCollision(SphereCollider& c, BoxCollider& aabb, CollisionManifold& OutManifold);
	static bool FindCollision(SphereCollider& c, BoxCollider& box, float theta, CollisionManifold& OutManifold);
	static bool FindCollision(SphereCollider& c, PolygonCollider& poly, CollisionManifold& OutManifold);
	static bool FindCollision(BoxCollider& box, PolygonCollider& poly, CollisionManifold& OutManifold);
	static bool FindCollision(PolygonCollider& poly, PolygonCollider& poly2, CollisionManifold& OutManifold);
	static bool FindCollision(PolygonCollider& poly, StaticCollider& s, CollisionManifold& OutManifold);

private:
	static Vector3 ToLocalSpace(const Vector3& point, const Vector3& position, float rotation);
	static Vector3 ToWorldSpacePoint(const Vector3& point, const Vector3& position, float rotation);
	static Vector3 ToWorldSpaceVector(const Vector3& vector, float rotation);

    static ConvexCollisionData MakeCollisionData(const BoxCollider& box);
	static ConvexCollisionData MakeCollisionData(const PolygonCollider& poly);
    static void ProjectionPoints(const std::vector<Vector3>& Points, const Vector3& Axis, float& OutMin, float& OutMax);

    static bool SATTest(const std::vector<Vector3>& PointsA, const std::vector<Vector3>& PointsB, const Vector3& Axis, SATResult& OutResult);
	static bool FindCollisionSAT(const ConvexCollisionData& DataA, const ConvexCollisionData& DataB, SATResult& OutResult);

    static SupportFeature FindSupportFeatures(const std::vector<Vector3>& Points, const Vector3& Direction, bool FindMax);

    static void ProjectFeature(const std::vector<Vector3>& Points, const Vector3& Axis, float& OutMin, float& OutMax);

	static bool GenerateDetector(const std::vector<Vector3>& PointsA, const std::vector<Vector3>& PointsB, const Vector3& Normal, CollisionManifold& OutManifold);
};

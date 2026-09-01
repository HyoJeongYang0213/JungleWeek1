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


class CollisionDetector
{
public:
	static bool FindCollision(SphereCollider& Ca, SphereCollider& Cb, CollisionManifold& OutManifold); 
	static bool FindCollision(SphereCollider& c, StaticCollider& s, CollisionManifold& OutManifold);
	static bool FindCollision(SphereCollider& c, BoxCollider& aabb, CollisionManifold& OutManifold);
	static bool FindCollision(SphereCollider& c, BoxCollider& box, float theta, CollisionManifold& OutManifold);
};
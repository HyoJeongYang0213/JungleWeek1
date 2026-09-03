#include "MiniBallSystem.h"

#include <algorithm>

#include "../Renderer/IRenderer.hpp"
#include "../Utils/Rnd.hpp"

void MiniBallSystem::Init(
    ID3D11Buffer* SphereVertexBuffer,
    UINT SphereVertexCount)
{
    mSphereVertexBuffer = SphereVertexBuffer;
    mSphereVertexCount = SphereVertexCount;

    // 충돌할 때마다 재할당되는 횟수를 조금 줄임
    mBalls.reserve(64);
}

void MiniBallSystem::Spawn(
    const Vector3& ContactPoint,
    const Vector3& CollisionNormal)
{
    constexpr int SpawnCount = 5;

    const Vector3 Normal =
        CollisionNormal.Normalize();

    // 충돌 법선과 직각인 옆 방향
    const Vector3 Tangent{
        -Normal.y,
        Normal.x,
        0.0f
    };

    for (int i = 0; i < SpawnCount; ++i)
    {
        MiniBall NewBall{};

        // 플랫폼 안쪽에서 생성되는 것을 막기 위해
        // 접촉점에서 법선 방향으로 살짝 띄움
        NewBall.Position =
            ContactPoint + Normal * 0.05f;

        const float ForwardSpeed =
            Rnd::GetRandom(2.0f, 4.0f);

        const float SideSpeed =
            Rnd::GetRandom(-1.5f, 1.5f);

        NewBall.Velocity =
            Normal * ForwardSpeed +
            Tangent * SideSpeed;

        NewBall.Radius =
            Rnd::GetRandom(0.04f, 0.09f);

        NewBall.Life =
            Rnd::GetRandom(0.3f, 0.6f);

        mBalls.push_back(NewBall);
    }
}

void MiniBallSystem::EmitTrail(
    const Vector3& BallPosition,
    const Vector3& BallVelocity,
    float BallRadius,
    float DeltaTime)
{
    constexpr float MinimumSpeed = 7.0f;
    constexpr float SpawnInterval = 0.05f;

    const float Speed = BallVelocity.Length();

    if (Speed < MinimumSpeed)
    {
        mTrailSpawnTimer = 0.0f;
        return;
    }

    mTrailSpawnTimer += DeltaTime;

    if (mTrailSpawnTimer < SpawnInterval)
    {
        return;
    }

    mTrailSpawnTimer = 0.0f;

    const Vector3 MoveDirection =
        BallVelocity / Speed;

    // 이동 방향에 수직인 방향
    const Vector3 Tangent{
        -MoveDirection.y,
        MoveDirection.x,
        0.0f
    };

    // trail 시작점 설정
    constexpr float SideOffsets[4] =
    {
        -0.6f,
        -0.2f,
         0.2f,
         0.6f
    };

	constexpr float BackOffsets[4] =
	{
		-0.1f,
		-0.4f,
		-0.4f,
		-0.1f
	};

    for (int i = 0; i < 4; ++i)
    {
        MiniBall NewBall{};

        const float SideOffset =
            BallRadius * SideOffsets[i];

        const float BackOffset =
            BallRadius * BackOffsets[i];

        NewBall.Position =
            BallPosition -
            MoveDirection * (BallRadius * 0.85f) 
			+ MoveDirection * BackOffset +
            Tangent * SideOffset;

        // 날아가는 공을 조금만 따라가게 설정
        NewBall.Velocity =
            BallVelocity * 0.08f;

        NewBall.Radius = 0.07f;
        NewBall.Life = 0.2f;

        mBalls.push_back(NewBall);
    }
}

void MiniBallSystem::Tick(float DeltaTime)
{
    constexpr float Gravity = 18.0f;

    for (MiniBall& Ball : mBalls)
    {
        if (!Ball.UseGravity)
        {
            // 간단한 중력
            Ball.Velocity.y -= Gravity * DeltaTime;
        }
        
        // 위치 이동
        Ball.Position += Ball.Velocity * DeltaTime;

        // 생존 시간 감소
        Ball.Life -= DeltaTime;
    }

    // 수명이 끝난 작은 공 제거
    const auto RemoveBegin = std::remove_if(
        mBalls.begin(),
        mBalls.end(),
        [](const MiniBall& Ball)
        {
            return Ball.Life <= 0.0f;
        }
    );

    mBalls.erase(RemoveBegin, mBalls.end());
}

void MiniBallSystem::Render(IRenderer& Renderer) const
{
    if (!mSphereVertexBuffer ||
        mSphereVertexCount == 0)
    {
        return;
    }

    for (const MiniBall& Ball : mBalls)
    {
        Renderer.UpdateConstant(
            Ball.Position,
            Vector3{
                Ball.Radius,
                Ball.Radius,
                0.0f
            },
            0.0f
        );

        Renderer.RenderPrimitive(
            mSphereVertexBuffer,
            mSphereVertexCount
        );
    }
}

void MiniBallSystem::Clear()
{
    mBalls.clear();
    mTrailSpawnTimer = 0.0f;
}
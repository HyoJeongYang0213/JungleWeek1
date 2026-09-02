#pragma once

#include <d3d11.h>
#include <vector>

#include "../Utils/Math.hpp"

class IRenderer;

class MiniBallSystem
{
private:
    struct MiniBall
    {
        Vector3 Position{};
        Vector3 Velocity{};

        float Radius = 0.55f;
        float Life = 0.0f;
    };

public:
    MiniBallSystem() = default;
    ~MiniBallSystem() = default;

    MiniBallSystem(const MiniBallSystem&) = delete;
    MiniBallSystem& operator=(const MiniBallSystem&) = delete;

public:
    void Init(ID3D11Buffer* SphereVertexBuffer,UINT SphereVertexCount);

    void Spawn(
        const Vector3& ContactPoint,
        const Vector3& CollisionNormal
    );

    void Tick(float DeltaTime);
    void Render(IRenderer& Renderer) const;
    void Clear();

private:
    std::vector<MiniBall> mBalls{};

    // GameScene이 소유한 구 버퍼를 빌려서 사용
    ID3D11Buffer* mSphereVertexBuffer = nullptr;
    UINT mSphereVertexCount = 0;
};

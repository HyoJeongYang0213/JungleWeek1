#pragma once

#include <vector>
#include <cstdint>

#include "../Utils/Math.hpp"


struct PlatformCollisionData
{
    Vector3 Center{ 0.0f, 0.0f, 0.0f };
    Vector3 HalfExtents{ 0.0f, 0.0f, 0.0f };

    int PixelCount = 0;
};


class CollisionMask
{
public:
    bool Load(const char* filePath);

    bool IsSolid(int x, int y) const;

    std::vector<PlatformCollisionData>
        BuildPlatformsNDC() const;

    int GetWidth() const
    {
        return mWidth;
    }

    int GetHeight() const
    {
        return mHeight;
    }


private:
    PlatformCollisionData BuildComponent(
        int startX,
        int startY,
        std::vector<uint8_t>& visited
    ) const;


private:
    int mWidth = 0;
    int mHeight = 0;

    std::vector<uint8_t> mPixels;

    static constexpr uint8_t CollisionThreshold = 128;
    static constexpr int MinimumPixelCount = 4;
};
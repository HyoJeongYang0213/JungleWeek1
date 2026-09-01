#include "CollisionMask.h"

#include "../ImGui/stb_image.h"

#include <algorithm>
#include <queue>
#include <utility>


bool CollisionMask::Load(
    const char* filePath
)
{
    int channels = 0;

    unsigned char* data =
        stbi_load(
            filePath,
            &mWidth,
            &mHeight,
            &channels,
            1
        );

    if (data == nullptr)
    {
        mWidth = 0;
        mHeight = 0;
        mPixels.clear();

        return false;
    }

    const size_t pixelCount =
        static_cast<size_t>(mWidth) *
        static_cast<size_t>(mHeight);

    mPixels.assign(
        data,
        data + pixelCount
    );

    stbi_image_free(data);

    return true;
}


bool CollisionMask::IsSolid(
    int x,
    int y
) const
{
    if (x < 0 ||
        x >= mWidth ||
        y < 0 ||
        y >= mHeight)
    {
        return false;
    }

    const size_t index =
        static_cast<size_t>(y) *
        static_cast<size_t>(mWidth)
        +
        static_cast<size_t>(x);

    return
        mPixels[index] >=
        CollisionThreshold;
}


PlatformCollisionData
CollisionMask::BuildComponent(
    int startX,
    int startY,
    std::vector<uint8_t>& visited
) const
{
    PlatformCollisionData result;

    int minX = startX;
    int maxX = startX;

    int minY = startY;
    int maxY = startY;

    int pixelCount = 0;

    std::queue<std::pair<int, int>> queue;

    queue.push({ startX, startY });

    visited[
        startY * mWidth + startX
    ] = 1;


    // 4-connected
    constexpr int dx[4] =
    {
        1, -1, 0, 0
    };

    constexpr int dy[4] =
    {
        0, 0, 1, -1
    };


    while (!queue.empty())
    {
        const auto current =
            queue.front();

        queue.pop();

        const int x = current.first;
        const int y = current.second;

        ++pixelCount;

        minX = std::min(minX, x);
        maxX = std::max(maxX, x);

        minY = std::min(minY, y);
        maxY = std::max(maxY, y);


        for (int i = 0; i < 4; ++i)
        {
            const int nextX =
                x + dx[i];

            const int nextY =
                y + dy[i];


            if (nextX < 0 ||
                nextX >= mWidth ||
                nextY < 0 ||
                nextY >= mHeight)
            {
                continue;
            }


            const size_t index =
                static_cast<size_t>(nextY) *
                static_cast<size_t>(mWidth)
                +
                static_cast<size_t>(nextX);


            if (visited[index])
            {
                continue;
            }

            if (!IsSolid(
                nextX,
                nextY))
            {
                continue;
            }

            visited[index] = 1;

            queue.push({
                nextX,
                nextY
                });
        }
    }



    // Pixel Boundary


    const float leftPixel =
        static_cast<float>(minX);

    const float rightPixel =
        static_cast<float>(maxX + 1);

    const float topPixel =
        static_cast<float>(minY);

    const float bottomPixel =
        static_cast<float>(maxY + 1);


    const float width =
        static_cast<float>(mWidth);

    const float height =
        static_cast<float>(mHeight);


    // Pixel → NDC

    const float leftNDC =
        (leftPixel / width)
        * 2.0f - 1.0f;

    const float rightNDC =
        (rightPixel / width)
        * 2.0f - 1.0f;


    // Image Y 방향과
    // NDC Y 방향이 반대
    const float topNDC =
        1.0f -
        (topPixel / height)
        * 2.0f;

    const float bottomNDC =
        1.0f -
        (bottomPixel / height)
        * 2.0f;



    // Center


    result.Center =
    {
        (leftNDC + rightNDC) * 0.5f,
        (topNDC + bottomNDC) * 0.5f,
        0.0f
    };



    // HalfExtents


    result.HalfExtents =
    {
        (rightNDC - leftNDC) * 0.5f,
        (topNDC - bottomNDC) * 0.5f,
        0.0f
    };


    result.PixelCount =
        pixelCount;


    return result;
}


std::vector<PlatformCollisionData>
CollisionMask::BuildPlatformsNDC() const
{
    std::vector<PlatformCollisionData> result;

    if (mPixels.empty() ||
        mWidth <= 0 ||
        mHeight <= 0)
    {
        return result;
    }


    std::vector<uint8_t> visited(
        static_cast<size_t>(mWidth) *
        static_cast<size_t>(mHeight),
        0
    );


    for (int y = 0; y < mHeight; ++y)
    {
        for (int x = 0; x < mWidth; ++x)
        {
            const size_t index =
                static_cast<size_t>(y) *
                static_cast<size_t>(mWidth)
                +
                static_cast<size_t>(x);


            if (visited[index])
            {
                continue;
            }


            if (!IsSolid(x, y))
            {
                continue;
            }


            PlatformCollisionData data =
                BuildComponent(
                    x,
                    y,
                    visited
                );


            if (data.PixelCount <
                MinimumPixelCount)
            {
                continue;
            }


            result.push_back(data);
        }
    }


    return result;
}
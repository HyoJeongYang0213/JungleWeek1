#pragma once
#include <vector>
#include "MapGlobals.hpp" 

struct VertexTex
{
    float x, y, z;
    float u, v;
};

namespace MapView
{

    inline float Clamp(float val, float minVal, float maxVal)
    {
        if (val < minVal) return minVal;
        if (val > maxVal) return maxVal;
        return val;
    }

    inline std::vector<VertexTex> CreateMapQuad(
        float viewHeightPx,
        float windowWidth ,
        float windowHeight,
        float cameraBottomPx = Globals::MAP_HEIGHT)
    {
        float bottomPx = Clamp(cameraBottomPx, viewHeightPx, Globals::MAP_HEIGHT);
        float topPx = bottomPx - viewHeightPx;

        float topV = topPx / Globals::MAP_HEIGHT;
        float bottomV = bottomPx / Globals::MAP_HEIGHT;

        float aspectWindow = windowWidth / windowHeight;
        float aspectView = Globals::MAP_WIDTH / viewHeightPx;
        float halfX = 1.0f;

        return std::vector<VertexTex>{
            { -halfX, 1.0f, 0.0f, 0.0f, topV },
            { halfX,  1.0f, 0.0f,  1.0f, topV },
            { halfX, -1.0f, 0.0f,  1.0f, bottomV },

            { -halfX,  1.0f, 0.0f,  0.0f, topV },
            { halfX, -1.0f, 0.0f,  1.0f, bottomV },
            { -halfX, -1.0f, 0.0f,  0.0f, bottomV }
        };
    }
}
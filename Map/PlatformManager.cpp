#include "PlatformManager.h"
#include "../Renderer/Renderer.h"
#include "../Map/MapGlobals.hpp"
#include "../Utils/Rnd.hpp"
#include <algorithm>
#include <cmath>

PlatformManager::~PlatformManager()
{
    if (mVertexBuffer)
    {
        mVertexBuffer->Release();
        mVertexBuffer = nullptr;
    }
}

void PlatformManager::Init(Renderer& renderer)
{
    mVertexBuffer = renderer.CreateDynamicVertexBuffer(sizeof(VertexTexture) * 6);
}

void PlatformManager::RegisterGroundPlatforms(const std::vector<PlatformData>& groundPlatforms)
{
    mGroundTemplates = groundPlatforms;
}

void PlatformManager::RegisterPatternPlatforms(const std::vector<std::vector<PlatformData>>& patternPlatforms)
{
    mPatternTemplates = patternPlatforms;
}

void PlatformManager::LoadFloor(int floorIndex)
{
    if (mLoadedFloors.find(floorIndex) != mLoadedFloors.end())
        return;

    const std::vector<PlatformData>* TargetTemplates = nullptr;

    if (floorIndex == 0)
    {
        TargetTemplates = &mGroundTemplates;
    }
    else if (!mPatternTemplates.empty())
    {
        auto it = mFloorPatternMap.find(floorIndex);
        int PatternIndex = 0;
        if (it != mFloorPatternMap.end())
        {
            PatternIndex = it->second;
        }
        else
        {
            PatternIndex = static_cast<int>(Rnd::GetRandom(0.0f, static_cast<float>(mPatternTemplates.size() - 1)));
            mFloorPatternMap[floorIndex] = PatternIndex;
        }
        TargetTemplates = &mPatternTemplates[PatternIndex];
    }

    if (!TargetTemplates) return;

    float FloorOffsetTopY = -static_cast<float>(floorIndex) * Globals::MAP_HEIGHT;

    for (const auto& tmpl : *TargetTemplates)
    {
        PlatformData Platform = tmpl;
        Platform.mWorldPosition.y = FloorOffsetTopY + tmpl.mWorldPosition.y;
        Platform.mFloorIndex = floorIndex;
        Platform.mIsVisible = false;

        mActivePlatforms.push_back(Platform);
    }

    mLoadedFloors.insert(floorIndex);
}

void PlatformManager::Update(float cameraCenterY)
{
    float ActiveMinY = cameraCenterY - BUFFER_DISTANCE;
    float ActiveMaxY = cameraCenterY + BUFFER_DISTANCE;

    auto RemoveIt = std::remove_if(mActivePlatforms.begin(), mActivePlatforms.end(),
        [ActiveMinY, ActiveMaxY](const PlatformData& plat) {
            return (plat.mWorldPosition.y < ActiveMinY || plat.mWorldPosition.y > ActiveMaxY);
        });

    mActivePlatforms.erase(RemoveIt, mActivePlatforms.end());

    mLoadedFloors.clear();
    for (const auto& plat : mActivePlatforms)
    {
        mLoadedFloors.insert(plat.mFloorIndex);
    }

    int MinFloor = static_cast<int>(std::floor((Globals::MAP_HEIGHT - ActiveMaxY) / Globals::MAP_HEIGHT));
    int MaxFloor = static_cast<int>(std::floor((Globals::MAP_HEIGHT - ActiveMinY) / Globals::MAP_HEIGHT));
    if (MinFloor < 0) MinFloor = 0;

    for (int f = MinFloor; f <= MaxFloor; ++f)
    {
        LoadFloor(f);
    }

    float HalfMapW = Globals::MAP_WIDTH * 0.5f;
    float HalfViewH = Globals::VIEW_HEIGHT_PX * 0.5f;

    for (auto& plat : mActivePlatforms)
    {
        float HalfW = plat.mWidth * 0.5f;
        float HalfH = plat.mHeight * 0.5f;

        float WorldLeft = plat.mWorldPosition.x - HalfW;
        float WorldRight = plat.mWorldPosition.x + HalfW;
        float WorldTop = plat.mWorldPosition.y - HalfH;
        float WorldBottom = plat.mWorldPosition.y + HalfH;

        float NDCLeft = WorldLeft / HalfMapW;
        float NDCRight = WorldRight / HalfMapW;
        float NDCTop = (cameraCenterY - WorldTop) / HalfViewH;
        float NDCBottom = (cameraCenterY - WorldBottom) / HalfViewH;

        bool InViewY = (NDCBottom <= 1.0f && NDCTop >= -1.0f);
        bool InViewX = (NDCLeft <= 1.0f && NDCRight >= -1.0f);

        plat.mIsVisible = (InViewX && InViewY);
    }
}

void PlatformManager::Render(Renderer& renderer, ID3D11ShaderResourceView* platformTexture, float cameraCenterY)
{
    if (!platformTexture || !mVertexBuffer) return;

    float HalfMapW = Globals::MAP_WIDTH * 0.5f;
    float HalfViewH = Globals::VIEW_HEIGHT_PX * 0.5f;

    UINT Stride = sizeof(VertexTexture);
    UINT Offset = 0;

    renderer.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    renderer.DeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &Stride, &Offset);
    renderer.DeviceContext->PSSetShaderResources(0, 1, &platformTexture);

    for (const auto& plat : mActivePlatforms)
    {
        if (!plat.mIsVisible) continue;

        float HalfW = plat.mWidth * 0.5f;
        float HalfH = plat.mHeight * 0.5f;

        float WorldLeft = plat.mWorldPosition.x - HalfW;
        float WorldRight = plat.mWorldPosition.x + HalfW;
        float WorldTop = plat.mWorldPosition.y - HalfH;
        float WorldBottom = plat.mWorldPosition.y + HalfH;

        float NDCLeft = WorldLeft / HalfMapW;
        float NDCRight = WorldRight / HalfMapW;
        float NDCTop = (cameraCenterY - WorldTop) / HalfViewH;
        float NDCBottom = (cameraCenterY - WorldBottom) / HalfViewH;

        VertexTexture Quad[6] = {
            { NDCLeft,  NDCTop,    0.0f,  0.0f, 0.0f }, // 좌상단
            { NDCRight, NDCTop,    0.0f,  1.0f, 0.0f }, // 우상단
            { NDCRight, NDCBottom, 0.0f,  1.0f, 1.0f }, // 우하단

            { NDCLeft,  NDCTop,    0.0f,  0.0f, 0.0f }, // 좌상단
            { NDCRight, NDCBottom, 0.0f,  1.0f, 1.0f }, // 우하단
            { NDCLeft,  NDCBottom, 0.0f,  0.0f, 1.0f }  // 좌하단
        };

        renderer.UpdateDynamicVertexBuffer(mVertexBuffer, Quad, sizeof(Quad));
        renderer.DeviceContext->Draw(6, 0);
    }
}
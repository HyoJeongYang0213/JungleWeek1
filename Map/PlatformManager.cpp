#include "PlatformManager.h"
#include "../Renderer/Renderer.h"
#include "../Map/MapGlobals.hpp"
#include "../Physics/CollisionMask.h"
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

std::vector<PlatformData> PlatformManager::LoadMaskTemplate(const std::string& FilePath)
{
    std::vector<PlatformData> Templates;
    CollisionMask Mask;
    if (!Mask.Load(FilePath.c_str()))
    {
        return Templates;
    }

    std::vector<PlatformCollisionData> RawDataList = Mask.BuildPlatformsNDC();
    float MapW = Globals::MAP_WIDTH;
    float MapH = Globals::MAP_HEIGHT;

    for (const auto& Raw : RawDataList)
    {
        PlatformData Plat;

        float RatioX = Raw.Center.x / 15.0f;
        float RatioY = (30.0f - Raw.Center.y) / 30.0f;

        Plat.mWorldPosition.x = (RatioX - 0.5f) * MapW;
        Plat.mWorldPosition.y = RatioY * MapH;
        Plat.mWorldPosition.z = 0.0f;

        Plat.mWidth = std::abs(Raw.HalfExtents.x / 15.0f) * MapW * 2.0f;
        Plat.mHeight = std::abs(Raw.HalfExtents.y / 30.0f) * MapH * 2.0f;
        Plat.mFloorIndex = 0;
        Plat.mIsVisible = false;

        Templates.push_back(Plat);
    }

    return Templates;
}

void PlatformManager::Init(Renderer& Renderer,
    const std::string& GroundMaskPath,
    const std::vector<std::string>& PatternMaskPaths)
{
    mVertexBuffer = Renderer.CreateDynamicVertexBuffer(sizeof(VertexTexture) * 6);

    mGroundTemplates = LoadMaskTemplate(GroundMaskPath);

    mPatternTemplates.clear();
    for (const auto& Path : PatternMaskPaths)
    {
        auto Tmpl = LoadMaskTemplate(Path);
        if (!Tmpl.empty())
        {
            mPatternTemplates.push_back(Tmpl);
        }
    }
}

void PlatformManager::LoadFloor(int FloorIndex)
{
    if (mLoadedFloors.find(FloorIndex) != mLoadedFloors.end())
        return;

    const std::vector<PlatformData>* TargetTemplates = nullptr;

    if (FloorIndex == 0)
    {
        TargetTemplates = &mGroundTemplates;
    }
    else if (!mPatternTemplates.empty())
    {
        auto It = mFloorPatternMap.find(FloorIndex);
        int PatternIndex = 0;
        if (It != mFloorPatternMap.end())
        {
            PatternIndex = It->second;
        }
        else
        {
            PatternIndex = static_cast<int>(Rnd::GetRandom(0.0f, static_cast<float>(mPatternTemplates.size() - 1)));
            mFloorPatternMap[FloorIndex] = PatternIndex;
        }
        TargetTemplates = &mPatternTemplates[PatternIndex];
    }

    if (!TargetTemplates) return;

    float FloorOffsetTopY = -static_cast<float>(FloorIndex) * Globals::MAP_HEIGHT;

    for (const auto& Tmpl : *TargetTemplates)
    {
        PlatformData PlatformItem = Tmpl;
        PlatformItem.mWorldPosition.y = FloorOffsetTopY + Tmpl.mWorldPosition.y;
        PlatformItem.mFloorIndex = FloorIndex;
        PlatformItem.mIsVisible = false;

        mActivePlatforms.push_back(PlatformItem);
    }

    mLoadedFloors.insert(FloorIndex);
}

void PlatformManager::Update(float CameraCenterY)
{
    float ActiveMinY = CameraCenterY - BUFFER_DISTANCE;
    float ActiveMaxY = CameraCenterY + BUFFER_DISTANCE;

    auto RemoveIt = std::remove_if(mActivePlatforms.begin(), mActivePlatforms.end(),
        [ActiveMinY, ActiveMaxY](const PlatformData& Plat) {
            return (Plat.mWorldPosition.y < ActiveMinY || Plat.mWorldPosition.y > ActiveMaxY);
        });

    mActivePlatforms.erase(RemoveIt, mActivePlatforms.end());

    mLoadedFloors.clear();
    for (const auto& Plat : mActivePlatforms)
    {
        mLoadedFloors.insert(Plat.mFloorIndex);
    }

    int MinFloor = static_cast<int>(std::floor((Globals::MAP_HEIGHT - ActiveMaxY) / Globals::MAP_HEIGHT));
    int MaxFloor = static_cast<int>(std::floor((Globals::MAP_HEIGHT - ActiveMinY) / Globals::MAP_HEIGHT));
    if (MinFloor < 0) MinFloor = 0;

    for (int F = MinFloor; F <= MaxFloor; ++F)
    {
        LoadFloor(F);
    }

    float HalfMapW = Globals::MAP_WIDTH * 0.5f;
    float HalfViewH = Globals::VIEW_HEIGHT_PX * 0.5f;

    for (auto& Plat : mActivePlatforms)
    {
        float HalfW = Plat.mWidth * 0.5f;
        float HalfH = Plat.mHeight * 0.5f;

        float WorldLeft = Plat.mWorldPosition.x - HalfW;
        float WorldRight = Plat.mWorldPosition.x + HalfW;
        float WorldTop = Plat.mWorldPosition.y - HalfH;
        float WorldBottom = Plat.mWorldPosition.y + HalfH;

        float ScreenTop = (CameraCenterY - WorldTop) / HalfViewH;
        float ScreenBottom = (CameraCenterY - WorldBottom) / HalfViewH;
        float ScreenLeft = WorldLeft / HalfMapW;
        float ScreenRight = WorldRight / HalfMapW;

        float NDCTop = (std::max)(ScreenTop, ScreenBottom);
        float NDCBottom = (std::min)(ScreenTop, ScreenBottom);
        float NDCLeft = (std::min)(ScreenLeft, ScreenRight);
        float NDCRight = (std::max)(ScreenLeft, ScreenRight);

        bool InViewY = (NDCBottom <= 1.0f && NDCTop >= -1.0f);
        bool InViewX = (NDCLeft <= 1.0f && NDCRight >= -1.0f);

        Plat.mIsVisible = (InViewX && InViewY);
    }
}

void PlatformManager::Render(Renderer& Renderer, ID3D11ShaderResourceView* PlatformTexture, float CameraCenterY)
{
    if (!PlatformTexture || !mVertexBuffer) return;

    float HalfMapW = Globals::MAP_WIDTH * 0.5f;
    float HalfViewH = Globals::VIEW_HEIGHT_PX * 0.5f;

    UINT Stride = sizeof(VertexTexture);
    UINT Offset = 0;

    Renderer.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Renderer.DeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &Stride, &Offset);
    Renderer.DeviceContext->PSSetShaderResources(0, 1, &PlatformTexture);

    for (const auto& Plat : mActivePlatforms)
    {
        if (!Plat.mIsVisible) continue;

        float HalfW = Plat.mWidth * 0.5f;
        float HalfH = Plat.mHeight * 0.5f;

        float NDCLeft = (Plat.mWorldPosition.x - HalfW) / HalfMapW;
        float NDCRight = (Plat.mWorldPosition.x + HalfW) / HalfMapW;
        float NDCTop = (CameraCenterY - (Plat.mWorldPosition.y - HalfH)) / HalfViewH;
        float NDCBottom = (CameraCenterY - (Plat.mWorldPosition.y + HalfH)) / HalfViewH;

        VertexTexture Quad[6] = {
            { NDCLeft,  NDCTop,    0.0f,  0.0f, 0.0f },
            { NDCRight, NDCTop,    0.0f,  1.0f, 0.0f },
            { NDCRight, NDCBottom, 0.0f,  1.0f, 1.0f },

            { NDCLeft,  NDCTop,    0.0f,  0.0f, 0.0f },
            { NDCRight, NDCBottom, 0.0f,  1.0f, 1.0f },
            { NDCLeft,  NDCBottom, 0.0f,  0.0f, 1.0f }
        };

        Renderer.UpdateDynamicVertexBuffer(mVertexBuffer, Quad, sizeof(Quad));
        Renderer.DeviceContext->Draw(6, 0);
    }
}
#include "PlatformManager.h"
#include "../Renderer/Renderer.h"
#include "../Map/MapGlobals.hpp"
#include "../Utils/Rnd.hpp"
#include <algorithm>

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
    // 사각형(Quad) 1개 크기의 Dynamic Vertex Buffer 생성
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

    const std::vector<PlatformData>* targetTemplates = nullptr;

    if (floorIndex == 0)
    {
        targetTemplates = &mGroundTemplates;
    }
    else if (!mPatternTemplates.empty())
    {
        auto it = mFloorPatternMap.find(floorIndex);
        int patternIdx = 0;
        if (it != mFloorPatternMap.end())
        {
            patternIdx = it->second;
        }
        else
        {
            patternIdx = Rnd::GetRandom(0, static_cast<int>(mPatternTemplates.size()) - 1);
            mFloorPatternMap[floorIndex] = patternIdx;
        }
        targetTemplates = &mPatternTemplates[patternIdx];
    }

    if (!targetTemplates) return;

    float floorOffsetTopY = -static_cast<float>(floorIndex) * Globals::MAP_HEIGHT;

    for (const auto& tmpl : *targetTemplates)
    {
        PlatformData plat = tmpl;
        plat.mWorldPosition.y = floorOffsetTopY + tmpl.mWorldPosition.y;
        plat.mFloorIndex = floorIndex;
        plat.mIsVisible = false;

        mActivePlatforms.push_back(plat);
    }

    mLoadedFloors.insert(floorIndex);
}

void PlatformManager::Update(float cameraCenterY)
{
    float activeMinY = cameraCenterY - BUFFER_DISTANCE;
    float activeMaxY = cameraCenterY + BUFFER_DISTANCE;

    auto removeIt = std::remove_if(mActivePlatforms.begin(), mActivePlatforms.end(),
        [activeMinY, activeMaxY, this](const PlatformData& plat) {
            bool outOfRange = (plat.mWorldPosition.y < activeMinY || plat.mWorldPosition.y > activeMaxY);
            if (outOfRange)
            {
                this->mLoadedFloors.erase(plat.mFloorIndex);
            }
            return outOfRange;
        });
    mActivePlatforms.erase(removeIt, mActivePlatforms.end());

    // 2. 버퍼 범위 내 층 로드
    int minFloor = static_cast<int>(std::floor((Globals::MAP_HEIGHT - activeMaxY) / Globals::MAP_HEIGHT));
    int maxFloor = static_cast<int>(std::floor((Globals::MAP_HEIGHT - activeMinY) / Globals::MAP_HEIGHT));
    if (minFloor < 0) minFloor = 0;

    for (int f = minFloor; f <= maxFloor; ++f)
    {
        LoadFloor(f);
    }

    float halfViewH = Globals::VIEW_HEIGHT_PX * 0.5f;

    for (auto& plat : mActivePlatforms)
    {
        float halfH = plat.mHeight * 0.5f;
        float worldTop = plat.mWorldPosition.y - halfH;
        float worldBottom = plat.mWorldPosition.y + halfH;

        float screenTop = 1.0f + ((cameraCenterY - worldTop) / halfViewH);
        float screenBottom = 1.0f + ((cameraCenterY - worldBottom) / halfViewH);

        if (screenTop >= 0.0f && screenBottom <= 2.0f)
        {
            plat.mIsVisible = true;
        }
        else
        {
            plat.mIsVisible = false;
        }
    }
}

void PlatformManager::Render(Renderer& renderer, ID3D11ShaderResourceView* platformTexture, float cameraCenterY)
{
    if (!platformTexture || !mVertexBuffer) return;

    float halfMapW = Globals::MAP_WIDTH * 0.5f;
    float halfViewH = Globals::VIEW_HEIGHT_PX * 0.5f;

    UINT stride = sizeof(VertexTexture);
    UINT offset = 0;

    renderer.DeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &stride, &offset);
    renderer.DeviceContext->PSSetShaderResources(0, 1, &platformTexture);

    for (const auto& plat : mActivePlatforms)
    {
        if (!plat.mIsVisible) continue;

        float halfW = plat.mWidth * 0.5f;
        float halfH = plat.mHeight * 0.5f;

        float worldLeft = plat.mWorldPosition.x - halfW;
        float worldRight = plat.mWorldPosition.x + halfW;
        float worldTop = plat.mWorldPosition.y - halfH;
        float worldBottom = plat.mWorldPosition.y + halfH;

        float screenLeft = 1.0f + (worldLeft / halfMapW);
        float screenRight = 1.0f + (worldRight / halfMapW);
        float screenTop = 1.0f + ((cameraCenterY - worldTop) / halfViewH);
        float screenBottom = 1.0f + ((cameraCenterY - worldBottom) / halfViewH);

        VertexTexture quad[6] = {
            { screenLeft,  screenTop,    0.0f,  0.0f, 0.0f },
            { screenRight, screenTop,    0.0f,  1.0f, 0.0f },
            { screenRight, screenBottom, 0.0f,  1.0f, 1.0f },

            { screenLeft,  screenTop,    0.0f,  0.0f, 0.0f },
            { screenRight, screenBottom, 0.0f,  1.0f, 1.0f },
            { screenLeft,  screenBottom, 0.0f,  0.0f, 1.0f }
        };

        renderer.UpdateDynamicVertexBuffer(mVertexBuffer, quad, sizeof(quad));
        renderer.DeviceContext->Draw(6, 0);
    }
}
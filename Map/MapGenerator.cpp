#include "../Map/MapGenerator.h"
#include "../Renderer/Renderer.h"
#include "../Utils/Rnd.hpp"
#include "../Resource/vertexSimple.hpp"
#include <cmath>

void InfiniteMap::Init(Renderer& renderer, ID3D11ShaderResourceView* groundmShaderResourceView, const std::vector<ID3D11ShaderResourceView*>& pattermShaderResourceViews)
{
    mShaderResourceViewGround = groundmShaderResourceView;
    mShaderResourceViewPatterns = pattermShaderResourceViews;

    mFloorTextures[0] = mShaderResourceViewGround;

    mVertexBufferChunk = renderer.CreateDynamicVertexBuffer(sizeof(VertexTexture) * 6);

    VertexTexture quad[6] = {
        { -1.0f,  1.0f, 0.0f,  0.0f, 0.0f },
        {  1.0f,  1.0f, 0.0f,  1.0f, 0.0f },
        {  1.0f, -1.0f, 0.0f,  1.0f, 1.0f },

        { -1.0f,  1.0f, 0.0f,  0.0f, 0.0f },
        {  1.0f, -1.0f, 0.0f,  1.0f, 1.0f },
        { -1.0f, -1.0f, 0.0f,  0.0f, 1.0f }
    };
    renderer.UpdateDynamicVertexBuffer(mVertexBufferChunk, quad, sizeof(quad));
}

InfiniteMap::~InfiniteMap()
{
    if (mVertexBufferChunk)
    {
        mVertexBufferChunk->Release();
        mVertexBufferChunk = nullptr;
    }

    if (mShaderResourceViewGround)
    {
        mShaderResourceViewGround->Release();
        mShaderResourceViewGround = nullptr;
    }

    for (auto& ShaderResourceView : mShaderResourceViewPatterns)
    {
        if (ShaderResourceView)
        {
            ShaderResourceView->Release();
            ShaderResourceView = nullptr;
        }
    }
}

/*ID3D11ShaderResourceView* InfiniteMap::GetOrCreateFloorTexture(int floorIndex)
{
    auto it = mFloorTextures.find(floorIndex);
    if (it != mFloorTextures.end())
    {
        return it->second;
    }

    if (!mShaderResourceViewPatterns.empty())
    {
        int MaxIndex = static_cast<int>(mShaderResourceViewPatterns.size()) - 1;
        int RandIndex = Rnd::GetRandom(0, MaxIndex);

        mFloorTextures[floorIndex] = mShaderResourceViewPatterns[RandIndex];
        return mShaderResourceViewPatterns[RandIndex];
    }

    return mShaderResourceViewGround;
}*/

ID3D11ShaderResourceView* InfiniteMap::GetOrCreateFloorTexture(int floorIndex)
{
    auto it = mFloorTextures.find(floorIndex);
    if (it != mFloorTextures.end())
    {
        return it->second;
    }

    if (floorIndex == 0)
    {
        mFloorTextures[0] = mShaderResourceViewGround;
        return mShaderResourceViewGround;
    }

    if (!mShaderResourceViewPatterns.empty() && floorIndex > 0)
    {
        constexpr int FLOORS_PER_STAGE = 10;

        int patternIndex = (floorIndex - 1) / FLOORS_PER_STAGE;

        int maxIndex = static_cast<int>(mShaderResourceViewPatterns.size()) - 1;
        if (patternIndex > maxIndex)
        {
            patternIndex = maxIndex;
        }

        ID3D11ShaderResourceView* selectedTexture = mShaderResourceViewPatterns[patternIndex];
        mFloorTextures[floorIndex] = selectedTexture;
        return selectedTexture;
    }

    return mShaderResourceViewGround;
}

void InfiniteMap::DrawChunk(Renderer& renderer, int floorIndex)
{
    ID3D11ShaderResourceView* Texture = GetOrCreateFloorTexture(floorIndex);
    if (!Texture) return;

    float CenterY = (static_cast<float>(floorIndex) * MapGlobals::CHUNK_HEIGHT) + MapGlobals::CHUNK_HEIGHT / 2.0f;
    Vector3 Center = { MapGlobals::RIGHT_BORDER / 2.0f, CenterY, 0.0f };
    Vector3 HalfExtents = { MapGlobals::RIGHT_BORDER / 2.0f, MapGlobals::CHUNK_HEIGHT / 2.0f, 0.0f };

    renderer.UpdateConstant(Center, HalfExtents, 0.0f);
    renderer.DeviceContext->PSSetShaderResources(0, 1, &Texture);

    UINT Stride = sizeof(VertexTexture);
    UINT Offset = 0;
    renderer.DeviceContext->IASetVertexBuffers(0, 1, &mVertexBufferChunk, &Stride, &Offset);
    renderer.DeviceContext->Draw(6, 0);
}

void InfiniteMap::Render(Renderer& renderer, ID3D11SamplerState* sampler, float cameraY)
{
    renderer.DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    renderer.DeviceContext->PSSetSamplers(0, 1, &sampler);

    int MinFloor = static_cast<int>(std::floor(cameraY / 30.0f));
    int MaxFloor = static_cast<int>(std::floor((cameraY + 40.0f) / 30.0f));

    if (MinFloor < 0) MinFloor = 0;
    for (int f = MinFloor; f <= MaxFloor; ++f)
    {
        DrawChunk(renderer, f);
    }
}

#include "../Map/MapGenerator.h"
#include "../Renderer/Renderer.h"
#include "../Utils/Rnd.hpp"
#include "../Resource/vertexSimple.hpp"
#include <cmath>

void InfiniteMap::Init(Renderer& renderer, ID3D11ShaderResourceView* groundmShaderResourceView, const std::vector<ID3D11ShaderResourceView*>& pattermShaderResourceViews, const std::vector<ID3D11ShaderResourceView*>& transitionShaderResourceViews)
{
    mShaderResourceViewGround = groundmShaderResourceView;
    mShaderResourceViewPatterns = pattermShaderResourceViews;
    mShaderResourceViewTransitions = transitionShaderResourceViews;

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
	if (mShaderResourceViewGround)
	{
		mShaderResourceViewGround->Release();
		mShaderResourceViewGround = nullptr;
	}

    if (!mShaderResourceViewPatterns.empty())
    {
        for (auto& pattern : mShaderResourceViewPatterns)
        {
            if (pattern)
            {
                pattern->Release();
            }
        }
        mShaderResourceViewPatterns.clear();
    }

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

    for (auto* srv : mShaderResourceViewTransitions)
    {
        if (srv)
        {
            srv->Release();
        }
    }
    mShaderResourceViewTransitions.clear();

    mFloorTextures.clear();
}

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

    if (floorIndex % MapGlobals::FLOORS_PER_THEME == 0)
    {
        int transitionIndex = (floorIndex / MapGlobals::FLOORS_PER_THEME) - 1;

        if (transitionIndex < static_cast<int>(mShaderResourceViewTransitions.size()))
        {
            ID3D11ShaderResourceView* transTexture = mShaderResourceViewTransitions[transitionIndex];
            mFloorTextures[floorIndex] = transTexture;
            return transTexture;
        }
    }

    if (!mShaderResourceViewPatterns.empty() && floorIndex > 0)
    {
        int themeIndex = (floorIndex - 1) / MapGlobals::FLOORS_PER_THEME;
        int maxIndex = static_cast<int>(mShaderResourceViewPatterns.size()) - 1;

        if (themeIndex > maxIndex)
        {
            themeIndex = maxIndex;
        }

        ID3D11ShaderResourceView* selectedTexture = mShaderResourceViewPatterns[themeIndex];
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

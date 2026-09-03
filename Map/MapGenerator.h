#pragma once
#include <d3d11.h>
#include <vector>
#include <unordered_map>

#include "../Utils/Rnd.hpp"
#include "../Renderer/Renderer.h"

struct SkyDeco
{
    ID3D11ShaderResourceView* texture = nullptr;
    Vector3 position;     // 월드 좌표 (Center)
    Vector3 halfExtents;  // 크기 (가로/세로 절반 크기)
};

class InfiniteMap
{
private:
    ID3D11ShaderResourceView* mShaderResourceViewGround = nullptr;
    std::vector<ID3D11ShaderResourceView*> mShaderResourceViewPatterns;

    std::unordered_map<int, ID3D11ShaderResourceView*> mFloorTextures;

    ID3D11Buffer* mVertexBufferChunk = nullptr;

public:
    InfiniteMap() = default;
    ~InfiniteMap();

    void Init(
        Renderer& renderer, ID3D11ShaderResourceView* groundSrv,
        const std::vector<ID3D11ShaderResourceView*>& patternSrvs,
        const std::vector<ID3D11ShaderResourceView*>& transitionShaderResourceViews = {},
        const std::vector<std::vector<ID3D11ShaderResourceView*>>& themeDecoResourceViews = {}
    );

    void Render(Renderer& renderer, ID3D11SamplerState* sampler, float cameraY);

private:
    ID3D11ShaderResourceView* GetOrCreateFloorTexture(int floorIndex);
    void GenerateFloorDecos(int floorIndex);
    void DrawChunk(Renderer& renderer, int floorIndex);

    std::vector<std::vector<ID3D11ShaderResourceView*>> mThemeDecoPatterns{};
    std::unordered_map<int, std::vector<SkyDeco>> mFloorDecos{};

    std::vector<ID3D11ShaderResourceView*> mShaderResourceViewTransitions{};
};

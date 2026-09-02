#pragma once
#include <d3d11.h>
#include <vector>
#include <unordered_map>

#include "../Utils/Rnd.hpp"
#include "../Renderer/Renderer.h"

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

    void Init(Renderer& renderer, ID3D11ShaderResourceView* groundSrv, const std::vector<ID3D11ShaderResourceView*>& patternSrvs);

    void Render(Renderer& renderer, ID3D11SamplerState* sampler, float cameraY);

private:
    ID3D11ShaderResourceView* GetOrCreateFloorTexture(int floorIndex);

    void DrawChunk(Renderer& renderer, int floorIndex);
};

#pragma once
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <d3d11.h>
#include "../Resource/vertexSimple.hpp"
#include "../Utils/Math.hpp"

class Renderer;

struct PlatformData
{
    Vector3 mWorldPosition;
    float mWidth;
    float mHeight;
    int mFloorIndex;
    bool mIsVisible;
};

class PlatformManager
{
public:
    PlatformManager() = default;
    ~PlatformManager();

    void Init(Renderer& renderer,
        const std::string& groundMaskPath,
        const std::vector<std::string>& patternMaskPaths);

    void Update(float cameraCenterY);

    void Render(Renderer& renderer, ID3D11ShaderResourceView* platformTexture, float cameraCenterY);

    const std::vector<PlatformData>& GetActivePlatforms() const { return mActivePlatforms; }

private:
    std::vector<PlatformData> LoadMaskTemplate(const std::string& filePath);
    void LoadFloor(int floorIndex);

    std::vector<PlatformData> mGroundTemplates;
    std::vector<std::vector<PlatformData>> mPatternTemplates;

    std::vector<PlatformData> mActivePlatforms;
    std::unordered_set<int> mLoadedFloors;
    std::unordered_map<int, int> mFloorPatternMap;

    ID3D11Buffer* mVertexBuffer = nullptr;
    const float BUFFER_DISTANCE = 5000.0f;
};
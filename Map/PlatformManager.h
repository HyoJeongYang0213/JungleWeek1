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

    Vector3 mPhysicsCenter;
    Vector3 mPhysicsHalfExtents;
};

class PlatformManager
{
public:
    PlatformManager() = default;
    ~PlatformManager();

    void Init(Renderer& Renderer,
        const std::string& GroundMaskPath,
        const std::vector<std::string>& PatternMaskPaths);

    void Update(float CameraCenterY);

    void Render(Renderer& Renderer, ID3D11ShaderResourceView* PlatformTexture);

    const std::vector<PlatformData>& GetActivePlatforms() const { return mActivePlatforms; }

private:
    std::vector<PlatformData> LoadMaskTemplate(const std::string& FilePath);
    void LoadFloor(int FloorIndex);

    std::vector<PlatformData> mGroundTemplates;
    std::vector<std::vector<PlatformData>> mPatternTemplates;

    std::vector<PlatformData> mActivePlatforms;
    std::unordered_set<int> mLoadedFloors;
    std::unordered_map<int, int> mFloorPatternMap;

    ID3D11Buffer* mVertexBuffer = nullptr;
    Renderer* mRenderer = nullptr;

    const float BUFFER_DISTANCE = 5000.0f;
};
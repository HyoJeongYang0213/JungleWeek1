#include "PlatformManager.h"
#include "../Renderer/Renderer.h"
#include "../Map/MapGlobals.hpp"
#include "../Physics/CollisionMask.h"
#include "../Physics/Collider.h"
#include "../Physics/RigidBody.h"
#include "../Physics/CollisionManifold.hpp"
#include "../Physics/CollisionDetector.h"
#include "../Physics/CollisionResolver.h"
#include "../Player/Ball.h"
#include "../Player/PlayerGlobals.hpp"
#include "../Utils/Rnd.hpp"
#include "../Audio/SoundManager.h"
#include <algorithm>
#include <cmath>

#ifndef BUFFER_DISTANCE
constexpr float BUFFER_DISTANCE = 45.0f; // 화면 위아래 버퍼 범위 (1.5층)
#endif

PlatformManager::~PlatformManager()
{
    if (mVertexBuffer)
    {
        mVertexBuffer->Release();
        mVertexBuffer = nullptr;
    }
	if (mRenderer)
	{
		mRenderer = nullptr;
	}
}

void PlatformManager::Init(Renderer& Renderer,
    const std::string& GroundMaskPath,
    const std::vector<std::string>& PatternMaskPaths)
{
    mRenderer = &Renderer;

    mVertexBuffer = Renderer.CreateDynamicVertexBuffer(sizeof(VertexTexture) * 6);

    VertexTexture quad[6] = {
        { -1.0f,  1.0f, 0.0f,  0.0f, 0.0f },
        {  1.0f,  1.0f, 0.0f,  1.0f, 0.0f },
        {  1.0f, -1.0f, 0.0f,  1.0f, 1.0f },

        { -1.0f,  1.0f, 0.0f,  0.0f, 0.0f },
        {  1.0f, -1.0f, 0.0f,  1.0f, 1.0f },
        { -1.0f, -1.0f, 0.0f,  0.0f, 1.0f }
    };
    Renderer.UpdateDynamicVertexBuffer(mVertexBuffer, quad, sizeof(quad));

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

std::vector<PlatformData> PlatformManager::LoadMaskTemplate(const std::string& FilePath)
{
    std::vector<PlatformData> Templates;
    CollisionMask Mask;
    if (!Mask.Load(FilePath.c_str()))
    {
        return Templates;
    }

    std::vector<PlatformCollisionData> RawDataList = Mask.BuildPlatformsNDC();

    for (const auto& Raw : RawDataList)
    {
        PlatformData Plat;

        Plat.mPhysicsCenter = Raw.Center;
        Plat.mPhysicsHalfExtents = Raw.HalfExtents;
        Plat.mWorldPosition = Raw.Center;

        Plat.mWidth = Raw.HalfExtents.x * 2.0f;
        Plat.mHeight = Raw.HalfExtents.y * 2.0f;
        Plat.mFloorIndex = 0;
        Plat.mIsVisible = true;

        Templates.push_back(Plat);
    }

    return Templates;
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
            PatternIndex = static_cast<int>(Rnd::GetRandom(0, static_cast<int>(mPatternTemplates.size() - 1)));
            mFloorPatternMap[FloorIndex] = PatternIndex;
        }
        TargetTemplates = &mPatternTemplates[PatternIndex];
    }

    if (!TargetTemplates) return;

    float FloorOffsetY = static_cast<float>(FloorIndex) * MapGlobals::CHUNK_HEIGHT;

    for (const auto& Tmpl : *TargetTemplates)
    {
        PlatformData PlatformItem = Tmpl;

        PlatformItem.mWorldPosition.y = Tmpl.mWorldPosition.y + FloorOffsetY;
        PlatformItem.mPhysicsCenter.y = Tmpl.mPhysicsCenter.y + FloorOffsetY;

        PlatformItem.mFloorIndex = FloorIndex;
        PlatformItem.mIsVisible = true;

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

    int MinFloor = static_cast<int>(std::floor(ActiveMinY / MapGlobals::CHUNK_HEIGHT));
    int MaxFloor = static_cast<int>(std::floor(ActiveMaxY / MapGlobals::CHUNK_HEIGHT));
    if (MinFloor < 0) MinFloor = 0;

    for (int F = MinFloor; F <= MaxFloor; ++F)
    {
        LoadFloor(F);
    }

    float ViewBottomY = CameraCenterY - 15.0f;
    float ViewTopY = CameraCenterY + 15.0f;

    for (auto& Plat : mActivePlatforms)
    {
        float HalfH = Plat.mHeight * 0.5f;
        float PlatTop = Plat.mWorldPosition.y + HalfH;
        float PlatBottom = Plat.mWorldPosition.y - HalfH;

        Plat.mIsVisible = (PlatBottom <= ViewTopY && PlatTop >= ViewBottomY);
    }

  
    Ball* Player = PlayerGlobals::PLAYERBALL;
    if (Player)
    {
        SphereCollider* Sphere = dynamic_cast<SphereCollider*>(&Player->GetCollider());
        if (Sphere)
        {
            bool bIsColliding = false;

            for (auto& Plat : mActivePlatforms)
            {
                Vector3 PlatPos = Plat.mPhysicsCenter;
                Vector3 PlatVel(0.0f, 0.0f, 0.0f);
                float PlatMass = 0.0f;
                Vector3 HalfExtents = Plat.mPhysicsHalfExtents;

                RigidBody PlatRB(PlatPos, PlatVel, PlatMass, 0.0f);
                BoxCollider PlatBox(PlatRB, HalfExtents);

                CollisionManifold Manifold;
                if (CollisionDetector::FindCollision(*Sphere, PlatBox, Manifold))
                {
                    bIsColliding = true;
                    CollisionResolver::PrepareConstraints(Manifold);
                    CollisionResolver::ResolvePosition(Manifold);
                    CollisionResolver::ResolveRestitution(Manifold);
                    CollisionResolver::ResolveFriction(Manifold);

                    if(mCanPlayHitSound)
                    {
                        SoundManager::GetInstance().PlaySound("BallHit", 0.5f, false);

                        // 접촉 위치에서 작은 공 생성 요청
                        if (mBallSpawnCallback &&
                            Manifold.ContactCount > 0)
                        {
                            mBallSpawnCallback(
                                Manifold.Contacts[0].Point,
                                Manifold.Normal
                            );
                        }

                        mCanPlayHitSound = false;
                    }

                    PlayerGlobals::PLAYERLOCATION = Player->GetRigidBody().GetPosition();
                }
            }

			if (!bIsColliding)
			{
				mCanPlayHitSound = true;
			}
        }
    }
    

    /*
    
    
    */


}

void PlatformManager::Render(Renderer& renderer, ID3D11ShaderResourceView* srv)
{
    if (!srv || !mVertexBuffer) return;

    renderer.DeviceContext->PSSetShaderResources(0, 1, &srv);

    UINT Stride = sizeof(VertexTexture);
    UINT Offset = 0;
    renderer.DeviceContext->IASetVertexBuffers(0, 1, &mVertexBuffer, &Stride, &Offset);

    for (const auto& Plat : mActivePlatforms)
    {
        if (!Plat.mIsVisible) continue;

        renderer.UpdateConstant(Plat.mPhysicsCenter, Plat.mPhysicsHalfExtents, 0.0f);
        renderer.DeviceContext->Draw(6, 0);
    }
}

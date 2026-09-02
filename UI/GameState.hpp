#pragma once

#include "../Renderer/Renderer.h"
#include "../Player/Ball.h"
#include "../Player/PlayerGlobals.hpp"
#include "../Map/PlatformManager.h"

enum class EGameState
{
    Title,
    Playing,
    Ending
};

struct GameContext
{
    EGameState mCurrentState = EGameState::Title;
    bool mBShowOptions = false;

    void ResetGame(Renderer& InRenderer, PlatformManager& InPlatformManager)
    {
        InRenderer.SetCameraPosition(Vector3(0.0f, 0.0f, 0.0f));

        if (InRenderer.PrimitiveCount > 0)
        {
            Ball* Player = dynamic_cast<Ball*>(InRenderer.PrimitiveList[0]);
            if (Player)
            {
                Player->GetRigidBody().SetPosition(Vector3(7.5f, 1.0f, 0.0f));
                PlayerGlobals::PLAYERLOCATION = Player->GetLocation();
            }
        }

        InPlatformManager.Update(15.0f);

        mBShowOptions = false;
    }
};
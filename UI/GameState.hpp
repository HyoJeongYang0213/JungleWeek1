#pragma once

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
};

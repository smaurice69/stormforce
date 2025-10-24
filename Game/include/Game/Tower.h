#pragma once

#include "GameData.h"

#include "Engine/MathTypes.h"

#include <string>

namespace Game
{
    struct TowerInstance
    {
        const TowerDefinition* definition{nullptr};
        Engine::Vector2f worldPosition;
        int gridX{0};
        int gridY{0};
        double cooldown{0.0};
    };
}

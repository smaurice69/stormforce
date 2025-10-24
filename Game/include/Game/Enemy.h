#pragma once

#include "GameData.h"

#include "Engine/MathTypes.h"

#include <vector>

namespace Game
{
    struct EnemyInstance
    {
        const EnemyDefinition* definition{nullptr};
        Engine::Vector2f position;
        Engine::Vector2f velocity;
        std::vector<Engine::Vector2f> path;
        size_t nextNode{0};
        float health{0.0f};
        bool alive{true};
    };
}

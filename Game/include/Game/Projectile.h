#pragma once

#include "Engine/MathTypes.h"

#include <string>

namespace Game
{
    struct Projectile
    {
        Engine::Vector2f position;
        Engine::Vector2f velocity;
        float lifetime{0.0f};
        float maxLifetime{3.0f};
        float damage{0.0f};
        std::wstring color;
        int targetIndex{-1};
        bool active{true};
    };
}

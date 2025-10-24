#pragma once

#include "Engine/MathTypes.h"

namespace Game
{
    class Camera
    {
    public:
        void SetViewport(float width, float height);
        void SetTarget(const Engine::Vector2f& position);
        void SetZoom(float zoom);

        Engine::Vector2f WorldToScreen(const Engine::Vector2f& world) const;
        Engine::Vector2f ScreenToWorld(const Engine::Vector2f& screen) const;

        float GetZoom() const { return m_zoom; }
        Engine::Vector2f GetPosition() const { return m_position; }

    private:
        Engine::Vector2f m_position{0.0f, 0.0f};
        Engine::Vector2f m_viewport{1280.0f, 720.0f};
        float m_zoom{1.0f};
    };
}

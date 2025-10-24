#include "Game/Camera.h"

namespace Game
{
    void Camera::SetViewport(float width, float height)
    {
        m_viewport = {width, height};
    }

    void Camera::SetTarget(const Engine::Vector2f& position)
    {
        m_position = position;
    }

    void Camera::SetZoom(float zoom)
    {
        m_zoom = zoom;
    }

    Engine::Vector2f Camera::WorldToScreen(const Engine::Vector2f& world) const
    {
        return {(world.x - m_position.x) * m_zoom + m_viewport.x * 0.5f,
                (world.y - m_position.y) * m_zoom + m_viewport.y * 0.5f};
    }

    Engine::Vector2f Camera::ScreenToWorld(const Engine::Vector2f& screen) const
    {
        return {((screen.x - m_viewport.x * 0.5f) / m_zoom) + m_position.x,
                ((screen.y - m_viewport.y * 0.5f) / m_zoom) + m_position.y};
    }
}

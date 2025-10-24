#include "Engine/Input.h"

namespace Engine
{
    void InputState::SetKeyDown(uint8_t key, bool isDown)
    {
        m_keys[key] = isDown;
    }

    void InputState::SetMouseButtonDown(int index, bool isDown)
    {
        if (index >= 0 && index < 3)
        {
            m_mouseButtons[index] = isDown;
        }
    }

    void InputState::SetMousePosition(int x, int y)
    {
        m_mouseX = x;
        m_mouseY = y;
    }

    void InputState::AdvanceFrame()
    {
        m_prevKeys = m_keys;
        m_prevMouseButtons = m_mouseButtons;
    }
}

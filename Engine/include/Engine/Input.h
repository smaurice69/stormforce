#pragma once

#include <array>
#include <cstdint>

namespace Engine
{
    class InputState
    {
    public:
        static constexpr size_t KeyCount = 256;

        void SetKeyDown(uint8_t key, bool isDown);
        void SetMouseButtonDown(int index, bool isDown);
        void SetMousePosition(int x, int y);

        bool IsKeyDown(uint8_t key) const { return m_keys[key]; }
        bool WasKeyPressed(uint8_t key) const { return m_keys[key] && !m_prevKeys[key]; }
        bool WasKeyReleased(uint8_t key) const { return !m_keys[key] && m_prevKeys[key]; }

        bool IsMouseDown(int index) const { return index >= 0 && index < 3 ? m_mouseButtons[index] : false; }
        bool WasMousePressed(int index) const { return index >= 0 && index < 3 ? (m_mouseButtons[index] && !m_prevMouseButtons[index]) : false; }
        bool WasMouseReleased(int index) const { return index >= 0 && index < 3 ? (!m_mouseButtons[index] && m_prevMouseButtons[index]) : false; }

        int GetMouseX() const { return m_mouseX; }
        int GetMouseY() const { return m_mouseY; }

        void AdvanceFrame();

    private:
        std::array<bool, KeyCount> m_keys{};
        std::array<bool, KeyCount> m_prevKeys{};
        std::array<bool, 3> m_mouseButtons{};
        std::array<bool, 3> m_prevMouseButtons{};
        int m_mouseX{0};
        int m_mouseY{0};
    };
}

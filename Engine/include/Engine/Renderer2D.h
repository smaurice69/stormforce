#pragma once

#include "Color.h"
#include "MathTypes.h"

#include <windows.h>
#include <vector>

namespace Engine
{
    class Renderer2D
    {
    public:
        explicit Renderer2D(HWND hwnd);
        ~Renderer2D();

        void Resize(int width, int height);
        void BeginFrame(const Color& clearColor);
        void EndFrame();

        void DrawFilledRect(const Rectf& rect, const Color& color);
        void DrawRectOutline(const Rectf& rect, const Color& color, int thickness = 1);
        void DrawLine(const Vector2f& a, const Vector2f& b, const Color& color, int thickness = 1);
        void DrawCircle(const Vector2f& center, float radius, const Color& color, bool filled = true);
        void DrawText(const std::wstring& text, const Vector2f& position, const Color& color, int size = 16);

    private:
        void EnsureBackBuffer();

        HWND m_hwnd;
        HBITMAP m_backBitmap{nullptr};
        HBITMAP m_oldBitmap{nullptr};
        HDC m_memDC{nullptr};
        HDC m_windowDC{nullptr};
        int m_width{0};
        int m_height{0};
    };
}

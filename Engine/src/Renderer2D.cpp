#include "Engine/Renderer2D.h"

#include <string>

namespace Engine
{
    Renderer2D::Renderer2D(HWND hwnd)
        : m_hwnd(hwnd)
    {
        m_windowDC = GetDC(hwnd);
        m_memDC = CreateCompatibleDC(m_windowDC);
    }

    Renderer2D::~Renderer2D()
    {
        if (m_memDC && m_oldBitmap)
        {
            SelectObject(m_memDC, m_oldBitmap);
        }
        if (m_backBitmap)
        {
            DeleteObject(m_backBitmap);
        }
        if (m_memDC)
        {
            DeleteDC(m_memDC);
        }
        if (m_windowDC)
        {
            ReleaseDC(m_hwnd, m_windowDC);
        }
    }

    void Renderer2D::Resize(int width, int height)
    {
        m_width = width;
        m_height = height;
        EnsureBackBuffer();
    }

    void Renderer2D::EnsureBackBuffer()
    {
        if (!m_memDC)
        {
            return;
        }

        if (m_backBitmap)
        {
            SelectObject(m_memDC, m_oldBitmap);
            DeleteObject(m_backBitmap);
            m_backBitmap = nullptr;
        }

        if (m_width > 0 && m_height > 0)
        {
            m_backBitmap = CreateCompatibleBitmap(m_windowDC, m_width, m_height);
            m_oldBitmap = static_cast<HBITMAP>(SelectObject(m_memDC, m_backBitmap));
        }
    }

    void Renderer2D::BeginFrame(const Color& clearColor)
    {
        if (!m_memDC)
        {
            return;
        }

        EnsureBackBuffer();
        const HBRUSH brush = CreateSolidBrush(RGB(clearColor.r, clearColor.g, clearColor.b));
        RECT rect{0, 0, m_width, m_height};
        FillRect(m_memDC, &rect, brush);
        DeleteObject(brush);
    }

    void Renderer2D::EndFrame()
    {
        if (!m_memDC || !m_windowDC)
        {
            return;
        }

        BitBlt(m_windowDC, 0, 0, m_width, m_height, m_memDC, 0, 0, SRCCOPY);
    }

    void Renderer2D::DrawFilledRect(const Rectf& rect, const Color& color)
    {
        if (!m_memDC)
        {
            return;
        }

        RECT rc{static_cast<LONG>(rect.left), static_cast<LONG>(rect.top), static_cast<LONG>(rect.right),
                static_cast<LONG>(rect.bottom)};
        HBRUSH brush = CreateSolidBrush(RGB(color.r, color.g, color.b));
        FillRect(m_memDC, &rc, brush);
        DeleteObject(brush);
    }

    void Renderer2D::DrawRectOutline(const Rectf& rect, const Color& color, int thickness)
    {
        if (!m_memDC)
        {
            return;
        }

        HPEN pen = CreatePen(PS_SOLID, thickness, RGB(color.r, color.g, color.b));
        HGDIOBJ oldPen = SelectObject(m_memDC, pen);
        HGDIOBJ oldBrush = SelectObject(m_memDC, GetStockObject(NULL_BRUSH));
        Rectangle(m_memDC, static_cast<int>(rect.left), static_cast<int>(rect.top), static_cast<int>(rect.right),
                  static_cast<int>(rect.bottom));
        SelectObject(m_memDC, oldPen);
        SelectObject(m_memDC, oldBrush);
        DeleteObject(pen);
    }

    void Renderer2D::DrawLine(const Vector2f& a, const Vector2f& b, const Color& color, int thickness)
    {
        if (!m_memDC)
        {
            return;
        }

        HPEN pen = CreatePen(PS_SOLID, thickness, RGB(color.r, color.g, color.b));
        HGDIOBJ oldPen = SelectObject(m_memDC, pen);
        MoveToEx(m_memDC, static_cast<int>(a.x), static_cast<int>(a.y), nullptr);
        LineTo(m_memDC, static_cast<int>(b.x), static_cast<int>(b.y));
        SelectObject(m_memDC, oldPen);
        DeleteObject(pen);
    }

    void Renderer2D::DrawCircle(const Vector2f& center, float radius, const Color& color, bool filled)
    {
        if (!m_memDC)
        {
            return;
        }

        HPEN pen = CreatePen(PS_SOLID, 1, RGB(color.r, color.g, color.b));
        HGDIOBJ oldPen = SelectObject(m_memDC, pen);
        HGDIOBJ oldBrush = nullptr;
        if (filled)
        {
            HBRUSH brush = CreateSolidBrush(RGB(color.r, color.g, color.b));
            oldBrush = SelectObject(m_memDC, brush);
            Ellipse(m_memDC, static_cast<int>(center.x - radius), static_cast<int>(center.y - radius),
                    static_cast<int>(center.x + radius), static_cast<int>(center.y + radius));
            SelectObject(m_memDC, oldBrush);
            DeleteObject(brush);
        }
        else
        {
            oldBrush = SelectObject(m_memDC, GetStockObject(NULL_BRUSH));
            Ellipse(m_memDC, static_cast<int>(center.x - radius), static_cast<int>(center.y - radius),
                    static_cast<int>(center.x + radius), static_cast<int>(center.y + radius));
            SelectObject(m_memDC, oldBrush);
        }

        SelectObject(m_memDC, oldPen);
        DeleteObject(pen);
    }

    void Renderer2D::DrawText(const std::wstring& text, const Vector2f& position, const Color& color, int size)
    {
        if (!m_memDC)
        {
            return;
        }

        HFONT font = CreateFontW(size, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_OUTLINE_PRECIS,
                                 CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, VARIABLE_PITCH, L"Segoe UI");
        HGDIOBJ oldFont = SelectObject(m_memDC, font);
        SetBkMode(m_memDC, TRANSPARENT);
        SetTextColor(m_memDC, RGB(color.r, color.g, color.b));
        TextOutW(m_memDC, static_cast<int>(position.x), static_cast<int>(position.y), text.c_str(),
                 static_cast<int>(text.size()));
        SelectObject(m_memDC, oldFont);
        DeleteObject(font);
    }
}

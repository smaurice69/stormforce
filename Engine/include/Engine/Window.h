#pragma once

#include <functional>
#include <string>
#include <windows.h>

namespace Engine
{
    class Window
    {
    public:
        using MessageCallback = std::function<LRESULT(HWND, UINT, WPARAM, LPARAM)>;

        Window();
        ~Window();

        bool Create(const std::wstring& title, int width, int height);
        void Destroy();
        void Show();
        void SetMessageCallback(MessageCallback callback);
        bool PumpMessages();

        HWND GetHandle() const { return m_hwnd; }
        int GetWidth() const { return m_width; }
        int GetHeight() const { return m_height; }

    private:
        static LRESULT CALLBACK StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
        LRESULT WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

        HWND m_hwnd{nullptr};
        MessageCallback m_callback;
        int m_width{0};
        int m_height{0};
    };
}

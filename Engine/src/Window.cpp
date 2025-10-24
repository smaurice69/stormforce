#include "Engine/Window.h"

namespace Engine
{
    namespace
    {
        const wchar_t* kWindowClassName = L"FieldrunnersFrontiersWindow";
    }

    Window::Window() = default;

    Window::~Window()
    {
        Destroy();
    }

    bool Window::Create(const std::wstring& title, int width, int height)
    {
        m_width = width;
        m_height = height;

        WNDCLASSEXW wc{};
        wc.cbSize = sizeof(WNDCLASSEXW);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = StaticWndProc;
        wc.hInstance = GetModuleHandle(nullptr);
        wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wc.lpszClassName = kWindowClassName;

        RegisterClassExW(&wc);

        RECT rect{0, 0, width, height};
        AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

        m_hwnd = CreateWindowExW(0, kWindowClassName, title.c_str(), WS_OVERLAPPEDWINDOW,
                                 CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top, nullptr,
                                 nullptr, GetModuleHandle(nullptr), this);

        return m_hwnd != nullptr;
    }

    void Window::Destroy()
    {
        if (m_hwnd)
        {
            DestroyWindow(m_hwnd);
            m_hwnd = nullptr;
        }
    }

    void Window::Show()
    {
        if (m_hwnd)
        {
            ShowWindow(m_hwnd, SW_SHOW);
            UpdateWindow(m_hwnd);
        }
    }

    void Window::SetMessageCallback(MessageCallback callback)
    {
        m_callback = std::move(callback);
    }

    bool Window::PumpMessages()
    {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
            {
                return false;
            }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        return true;
    }

    LRESULT CALLBACK Window::StaticWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (msg == WM_NCCREATE)
        {
            auto cs = reinterpret_cast<CREATESTRUCTW*>(lParam);
            auto window = static_cast<Window*>(cs->lpCreateParams);
            SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(window));
            window->m_hwnd = hwnd;
        }

        auto window = reinterpret_cast<Window*>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
        if (window)
        {
            return window->WndProc(hwnd, msg, wParam, lParam);
        }
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }

    LRESULT Window::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        if (m_callback)
        {
            return m_callback(hwnd, msg, wParam, lParam);
        }
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

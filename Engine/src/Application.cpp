#include "Engine/Application.h"

#include "Engine/StringUtil.h"

#include <shellapi.h>
#include <windowsx.h>

namespace Engine
{
    Application::Application() = default;
    Application::~Application() = default;

    bool Application::Initialize(const std::wstring& title, int width, int height)
    {
        if (!m_window.Create(title, width, height))
        {
            return false;
        }

        m_window.SetMessageCallback([this](HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
            return HandleMessage(hwnd, msg, wParam, lParam);
        });
        DragAcceptFiles(m_window.GetHandle(), TRUE);
        m_window.Show();

        m_renderer = std::make_unique<Renderer2D>(m_window.GetHandle());
        m_renderer->Resize(width, height);

        if (!OnInitialize())
        {
            return false;
        }

        m_timer.Reset();
        m_running = true;
        return true;
    }

    int Application::Run()
    {
        while (m_running)
        {
            if (!m_window.PumpMessages())
            {
                break;
            }

            m_timer.Tick();
            const double deltaSeconds = m_timer.GetDeltaSeconds();
            OnUpdate(deltaSeconds);

            auto& renderer = GetRenderer();
            OnRender(renderer);

            m_input.AdvanceFrame();
        }

        OnShutdown();
        return 0;
    }

    void Application::Shutdown()
    {
        m_running = false;
        m_renderer.reset();
        m_window.Destroy();
    }

    Renderer2D& Application::GetRenderer()
    {
        return *m_renderer;
    }

    LRESULT Application::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
    {
        switch (msg)
        {
        case WM_DESTROY:
            m_running = false;
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
            if (m_renderer)
            {
                const int width = LOWORD(lParam);
                const int height = HIWORD(lParam);
                m_renderer->Resize(width, height);
            }
            break;
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        {
            const auto key = static_cast<uint8_t>(wParam & 0xFF);
            if (!(lParam & (1 << 30)))
            {
                m_input.SetKeyDown(key, true);
                OnKeyEvent(key, true);
            }
            return 0;
        }
        case WM_KEYUP:
        case WM_SYSKEYUP:
        {
            const auto key = static_cast<uint8_t>(wParam & 0xFF);
            m_input.SetKeyDown(key, false);
            OnKeyEvent(key, false);
            return 0;
        }
        case WM_LBUTTONDOWN:
            SetCapture(hwnd);
            m_input.SetMouseButtonDown(0, true);
            OnMouseButton(0, true);
            return 0;
        case WM_LBUTTONUP:
            ReleaseCapture();
            m_input.SetMouseButtonDown(0, false);
            OnMouseButton(0, false);
            return 0;
        case WM_RBUTTONDOWN:
            SetCapture(hwnd);
            m_input.SetMouseButtonDown(1, true);
            OnMouseButton(1, true);
            return 0;
        case WM_RBUTTONUP:
            ReleaseCapture();
            m_input.SetMouseButtonDown(1, false);
            OnMouseButton(1, false);
            return 0;
        case WM_MBUTTONDOWN:
            SetCapture(hwnd);
            m_input.SetMouseButtonDown(2, true);
            OnMouseButton(2, true);
            return 0;
        case WM_MBUTTONUP:
            ReleaseCapture();
            m_input.SetMouseButtonDown(2, false);
            OnMouseButton(2, false);
            return 0;
        case WM_MOUSEMOVE:
        {
            const int x = GET_X_LPARAM(lParam);
            const int y = GET_Y_LPARAM(lParam);
            m_input.SetMousePosition(x, y);
            return 0;
        }
        case WM_MOUSEWHEEL:
        {
            const int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            const auto key = delta > 0 ? static_cast<uint8_t>(0x80) : static_cast<uint8_t>(0x81);
            m_input.SetKeyDown(key, true);
            OnKeyEvent(key, true);
            m_input.SetKeyDown(key, false);
            OnKeyEvent(key, false);
            return 0;
        }
        case WM_DROPFILES:
        {
            const HDROP drop = reinterpret_cast<HDROP>(wParam);
            wchar_t buffer[MAX_PATH];
            if (DragQueryFileW(drop, 0, buffer, MAX_PATH))
            {
                OnFileDrop(buffer);
            }
            DragFinish(drop);
            return 0;
        }
        default:
            break;
        }

        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
}

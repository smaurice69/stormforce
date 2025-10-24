#pragma once

#include "Input.h"
#include "Renderer2D.h"
#include "Time.h"
#include "Window.h"

#include <memory>
#include <string>

namespace Engine
{
    class Application
    {
    public:
        Application();
        virtual ~Application();

        bool Initialize(const std::wstring& title, int width, int height);
        int Run();
        void Shutdown();

    protected:
        virtual bool OnInitialize() = 0;
        virtual void OnUpdate(double deltaSeconds) = 0;
        virtual void OnRender(Renderer2D& renderer) = 0;
        virtual void OnShutdown() {}
        virtual void OnFileDrop(const std::wstring&) {}
        virtual void OnKeyEvent(uint8_t key, bool pressed) {}
        virtual void OnMouseButton(int button, bool pressed) {}

        Window& GetWindow() { return m_window; }
        Renderer2D& GetRenderer();
        InputState& GetInputState() { return m_input; }

    private:
        Window m_window;
        std::unique_ptr<Renderer2D> m_renderer;
        InputState m_input;
        StepTimer m_timer;
        bool m_running{false};

        LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    };
}

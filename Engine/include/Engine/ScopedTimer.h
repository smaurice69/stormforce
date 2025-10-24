#pragma once

#include <chrono>
#include <functional>
#include <string>

namespace Engine
{
    class ScopedTimer
    {
    public:
        using Callback = std::function<void(double)>;

        ScopedTimer(std::wstring label, Callback callback);
        ~ScopedTimer();

    private:
        std::wstring m_label;
        Callback m_callback;
        std::chrono::high_resolution_clock::time_point m_start;
    };
}

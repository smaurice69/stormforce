#include <utility>
#include "Engine/ScopedTimer.h"

namespace Engine
{
    ScopedTimer::ScopedTimer(std::wstring label, Callback callback)
        : m_label(std::move(label))
        , m_callback(std::move(callback))
        , m_start(std::chrono::high_resolution_clock::now())
    {
    }

    ScopedTimer::~ScopedTimer()
    {
        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double, std::milli> duration = end - m_start;
        if (m_callback)
        {
            m_callback(duration.count());
        }
    }
}

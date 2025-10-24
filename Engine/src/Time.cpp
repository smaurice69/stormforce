#include "Engine/Time.h"

namespace Engine
{
    StepTimer::StepTimer()
    {
        Reset();
    }

    void StepTimer::Reset()
    {
        m_start = Clock::now();
        m_prev = m_start;
        m_deltaSeconds = 0.0;
        m_frameCount = 0;
    }

    void StepTimer::Tick()
    {
        auto now = Clock::now();
        std::chrono::duration<double> delta = now - m_prev;
        m_deltaSeconds = delta.count();
        m_prev = now;
        ++m_frameCount;
    }

    double StepTimer::GetElapsedSeconds() const
    {
        std::chrono::duration<double> elapsed = Clock::now() - m_start;
        return elapsed.count();
    }
}

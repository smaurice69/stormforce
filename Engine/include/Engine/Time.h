#pragma once

#include <chrono>

namespace Engine
{
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    using Duration = Clock::duration;

    class StepTimer
    {
    public:
        StepTimer();

        void Reset();
        void Tick();

        double GetDeltaSeconds() const { return m_deltaSeconds; }
        double GetElapsedSeconds() const;
        uint64_t GetFrameCount() const { return m_frameCount; }

    private:
        TimePoint m_start;
        TimePoint m_prev;
        double m_deltaSeconds{0.0};
        uint64_t m_frameCount{0};
    };
}

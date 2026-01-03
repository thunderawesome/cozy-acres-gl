#include "core/time/TimeSystem.h"

namespace cozy::core
{
    TimeSystem::TimeSystem()
        : m_startTime(Clock::now()), m_lastFrameTime(Clock::now())
    {
    }

    void TimeSystem::Update()
    {
        auto currentTime = Clock::now();

        // Calculate Delta Time
        m_deltaTime = std::chrono::duration<float, std::ratio<1>>(currentTime - m_lastFrameTime).count();
        m_lastFrameTime = currentTime;

        // Calculate Total Time since start
        m_totalTime = std::chrono::duration<double, std::ratio<1>>(currentTime - m_startTime).count();
    }
}
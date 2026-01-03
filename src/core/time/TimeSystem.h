#pragma once

#include <chrono>

namespace cozy::core
{
    // 'final' tells the compiler it can devirtualize (optimize) if needed.
    // We removed ITime because time logic is universal.
    class TimeSystem final
    {
    public:
        TimeSystem();

        // No 'virtual' means no v-table, no overhead.
        void Update();

        [[nodiscard]] float GetDeltaTime() const noexcept { return m_deltaTime; }
        [[nodiscard]] float GetTotalTime() const noexcept { return static_cast<float>(m_totalTime); }
        [[nodiscard]] double GetTotalTimeHighPrecision() const noexcept { return m_totalTime; }

    private:
        using Clock = std::chrono::high_resolution_clock;

        std::chrono::time_point<Clock> m_startTime;
        std::chrono::time_point<Clock> m_lastFrameTime;

        float m_deltaTime{0.0f};
        double m_totalTime{0.0};
    };
}
#pragma once

namespace cozy::core
{
    class ITime
    {
    public:
        virtual ~ITime() = default;

        // Calculates the time elapsed since the last call
        virtual void Update() = 0;

        [[nodiscard]] virtual float GetDeltaTime() const noexcept = 0;
        [[nodiscard]] virtual float GetTotalTime() const noexcept = 0;
        [[nodiscard]] virtual double GetTotalTimeHighPrecision() const noexcept = 0;
    };
}
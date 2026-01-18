#pragma once

#include "core/input/InputConfig.h"

namespace cozy::platform
{
    class IWindow;
}
namespace cozy::core
{
    class ICamera;
}

namespace cozy::core
{
    enum class InputAction
    {
        Regenerate,
        Exit,
        ToggleCursor,
        ToggleDebug
    };

    class IInputSystem
    {
    public:
        virtual ~IInputSystem() = default;

        virtual void Update(platform::IWindow &window, ICamera &camera, float deltaTime) = 0;

        virtual bool IsActionTriggered(InputAction action) const noexcept = 0;

        virtual void SetConfig(const InputConfig &config) noexcept = 0;
        virtual const InputConfig &GetConfig() const noexcept = 0;
    };
}
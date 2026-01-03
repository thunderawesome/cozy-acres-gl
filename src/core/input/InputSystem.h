#pragma once

#include "IInputSystem.h"
#include <glm/glm.hpp>

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
    // This is now a platform-agnostic Polling System
    class InputSystem final : public IInputSystem
    {
    private:
        InputConfig m_config;
        bool m_firstMouse{true};
        double m_lastX{0.0};
        double m_lastY{0.0};
        bool m_cursorDisabled{true};

        void handleKeyboard(platform::IWindow &window, ICamera &camera, float deltaTime);
        void handleMouse(const platform::IWindow &window, ICamera &camera);
        void handleCursorToggle(platform::IWindow &window);

    public:
        explicit InputSystem(const InputConfig &config = InputConfig::Default());

        void Update(platform::IWindow &window, ICamera &camera, float deltaTime) override;

        void SetConfig(const InputConfig &config) noexcept override { m_config = config; }
        const InputConfig &GetConfig() const noexcept override { return m_config; }
    };
}
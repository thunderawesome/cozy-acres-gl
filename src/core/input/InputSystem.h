#pragma once

#include "IInputSystem.h"
#include <glm/glm.hpp>
#include <unordered_map>

namespace cozy::core
{
    class InputSystem final : public IInputSystem
    {
    private:
        InputConfig m_config;
        bool m_firstMouse{true};
        double m_lastX{0.0};
        double m_lastY{0.0};
        bool m_cursorDisabled{true};

        // Internal state for edge detection
        std::unordered_map<InputAction, bool> m_actionTriggered;
        std::unordered_map<InputAction, bool> m_lastFrameState;

        void handleKeyboard(platform::IWindow &window, ICamera &camera, float deltaTime);
        void handleMouse(const platform::IWindow &window, ICamera &camera);
        void updateActionState(platform::IWindow &window, InputAction action, int keyCode);

    public:
        explicit InputSystem(const InputConfig &config = InputConfig::Default());

        void Update(platform::IWindow &window, ICamera &camera, float deltaTime) override;

        bool IsActionTriggered(InputAction action) const noexcept override;

        void SetConfig(const InputConfig &config) noexcept override { m_config = config; }
        const InputConfig &GetConfig() const noexcept override { return m_config; }
    };
}
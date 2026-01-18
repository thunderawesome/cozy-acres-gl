#include "core/input/InputSystem.h"
#include "platform/IWindow.h"
#include "core/camera/ICamera.h"

namespace cozy::core
{
    InputSystem::InputSystem(const InputConfig &config)
        : m_config(config)
    {
    }

    void InputSystem::Update(platform::IWindow &window, ICamera &camera, float deltaTime)
    {
        // 1. Update Discrete Action States (Edge Detection)
        updateActionState(window, InputAction::Regenerate, m_config.keyRegenerate);
        updateActionState(window, InputAction::Exit, m_config.keyExit);
        updateActionState(window, InputAction::ToggleCursor, m_config.keyToggleCursor);
        updateActionState(window, InputAction::ToggleDebug, m_config.keyToggleDebug);

        // 2. Handle Continuous Systems
        handleKeyboard(window, camera, deltaTime);
        handleMouse(window, camera);

        // 3. System-level Responses
        if (IsActionTriggered(InputAction::ToggleCursor))
        {
            m_cursorDisabled = !m_cursorDisabled;
            window.SetCursorVisible(!m_cursorDisabled);
        }

        if (IsActionTriggered(InputAction::Exit))
        {
            window.SetShouldClose(true);
        }
    }

    void InputSystem::updateActionState(platform::IWindow &window, InputAction action, int keyCode)
    {
        bool isDown = window.IsKeyPressed(keyCode);
        // Action is triggered only on the frame the key goes from UP to DOWN
        m_actionTriggered[action] = (isDown && !m_lastFrameState[action]);
        m_lastFrameState[action] = isDown;
    }

    bool InputSystem::IsActionTriggered(InputAction action) const noexcept
    {
        auto it = m_actionTriggered.find(action);
        return (it != m_actionTriggered.end()) ? it->second : false;
    }

    void InputSystem::handleKeyboard(platform::IWindow &window, ICamera &camera, float deltaTime)
    {
        glm::vec3 direction{0.0f};

        if (window.IsKeyPressed(m_config.keyForward))
            direction.z += 1.0f;
        if (window.IsKeyPressed(m_config.keyBackward))
            direction.z -= 1.0f;
        if (window.IsKeyPressed(m_config.keyRight))
            direction.x += 1.0f;
        if (window.IsKeyPressed(m_config.keyLeft))
            direction.x -= 1.0f;
        if (window.IsKeyPressed(m_config.keyUp))
            direction.y += 1.0f;
        if (window.IsKeyPressed(m_config.keyDown))
            direction.y -= 1.0f;

        bool isSprinting = window.IsKeyPressed(m_config.keySprint);

        if (glm::length(direction) > 0.01f)
        {
            camera.ProcessKeyboard(glm::normalize(direction), deltaTime, isSprinting);
        }
    }

    void InputSystem::handleMouse(const platform::IWindow &window, ICamera &camera)
    {
        glm::vec2 mousePos = window.GetCursorPosition();

        if (m_firstMouse)
        {
            m_lastX = mousePos.x;
            m_lastY = mousePos.y;
            m_firstMouse = false;
        }

        float xoffset = static_cast<float>(mousePos.x - m_lastX);
        float yoffset = static_cast<float>(m_lastY - mousePos.y);

        m_lastX = mousePos.x;
        m_lastY = mousePos.y;

        if (m_cursorDisabled)
        {
            camera.ProcessMouseMovement(xoffset * m_config.mouseSensitivity,
                                        yoffset * m_config.mouseSensitivity);
        }

        float scrollY = window.GetMouseScroll().y;
        if (std::abs(scrollY) > 0.01f)
        {
            auto config = camera.GetConfig();
            config.fovDegrees = glm::clamp(config.fovDegrees - (scrollY * m_config.scrollSensitivity), 1.0f, 90.0f);
            camera.SetConfig(config);
        }
    }
}
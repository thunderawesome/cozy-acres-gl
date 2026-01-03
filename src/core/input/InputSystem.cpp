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
        handleKeyboard(window, camera, deltaTime);
        handleMouse(window, camera);
        handleCursorToggle(window);
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

        if (glm::length(direction) > 0.01f)
        {
            camera.ProcessKeyboard(glm::normalize(direction), deltaTime);
        }

        if (window.IsKeyPressed(m_config.keyExit))
        {
            window.SetShouldClose(true);
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
    }

    void InputSystem::handleCursorToggle(platform::IWindow &window)
    {
        static bool tabWasDown = false;
        bool tabIsDown = window.IsKeyPressed(m_config.keyToggleCursor);

        if (tabIsDown && !tabWasDown)
        {
            m_cursorDisabled = !m_cursorDisabled;
            window.SetCursorVisible(!m_cursorDisabled);
        }
        tabWasDown = tabIsDown;
    }
}
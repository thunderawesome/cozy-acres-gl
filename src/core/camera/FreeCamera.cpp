#include "FreeCamera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace cozy::core
{

    void FreeCamera::updateVectors() noexcept
    {
        glm::vec3 front;
        front.x = cos(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        front.y = sin(glm::radians(m_pitch));
        front.z = sin(glm::radians(m_yaw)) * cos(glm::radians(m_pitch));
        m_front = glm::normalize(front);

        m_right = glm::normalize(glm::cross(m_front, m_worldUp));
        m_up = glm::normalize(glm::cross(m_right, m_front));
    }

    void FreeCamera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) noexcept
    {
        xoffset *= m_config.mouseSensitivity;
        yoffset *= m_config.mouseSensitivity;

        m_yaw += xoffset;
        m_pitch += yoffset;

        if (constrainPitch)
        {
            if (m_pitch > m_config.pitchClamp)
                m_pitch = m_config.pitchClamp;
            if (m_pitch < -m_config.pitchClamp)
                m_pitch = -m_config.pitchClamp;
        }

        updateVectors();
    }

    void FreeCamera::ProcessKeyboard(glm::vec3 direction, float deltaTime) noexcept
    {
        float velocity = m_config.moveSpeed * deltaTime;
        m_position += direction.x * m_right * velocity;
        m_position += direction.y * m_worldUp * velocity;
        m_position += direction.z * m_front * velocity;
    }

}
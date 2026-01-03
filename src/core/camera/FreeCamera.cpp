#include "FreeCamera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace cozy::core
{

    void FreeCamera::SetPosition(const glm::vec3 &position) noexcept
    {
        m_position = position;
    }

    void FreeCamera::SetRotation(float yaw, float pitch) noexcept
    {
        m_yaw = yaw;
        m_pitch = pitch;
        updateVectors(); // Recalculate Front, Right, and Up based on new angles
    }

    void FreeCamera::LookAt(const glm::vec3 &target) noexcept
    {
        glm::vec3 direction = glm::normalize(target - m_position);

        // Convert direction vector back to Euler angles
        m_pitch = glm::degrees(asin(direction.y));
        m_yaw = glm::degrees(atan2(direction.z, direction.x));

        updateVectors();
    }

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

    void FreeCamera::ProcessKeyboard(glm::vec3 direction, float deltaTime, bool isSprinting) noexcept
    {
        float currentSpeed = m_config.moveSpeed;
        if (isSprinting)
        {
            currentSpeed *= m_config.boostMultiplier;
        }

        float velocity = currentSpeed * deltaTime;
        m_position += direction.x * m_right * velocity;
        m_position += direction.y * m_worldUp * velocity;
        m_position += direction.z * m_front * velocity;
    }

}
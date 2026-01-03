#pragma once
#include "ICamera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace cozy::core
{

    class FreeCamera final : public ICamera
    {
    private:
        glm::vec3 m_position{0.0f, 1.0f, 10.0f}; // Good starting view over town
        glm::vec3 m_front{0.0f, 0.0f, -1.0f};
        glm::vec3 m_up{0.0f, 1.0f, 0.0f};
        glm::vec3 m_right{};
        glm::vec3 m_worldUp{0.0f, 1.0f, 0.0f};

        float m_yaw{-90.0f};
        float m_pitch{0.0f};

        CameraConfig m_config;

        void updateVectors() noexcept;

    public:
        explicit FreeCamera(const CameraConfig &config = CameraConfig::FreeFlyPreset())
            : m_config(config)
        {
            updateVectors();
        }

        glm::mat4 GetViewMatrix() const noexcept override
        {
            return glm::lookAt(m_position, m_position + m_front, m_up);
        }

        glm::mat4 GetProjectionMatrix(float aspectRatio) const noexcept override
        {
            return glm::perspective(glm::radians(m_config.fovDegrees), aspectRatio,
                                    m_config.nearPlane, m_config.farPlane);
        }

        glm::vec3 GetPosition() const noexcept override { return m_position; }
        glm::vec3 GetFront() const noexcept override { return m_front; }
        glm::vec3 GetUp() const noexcept override { return m_up; }

        void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) noexcept override;
        void ProcessKeyboard(glm::vec3 direction, float deltaTime) noexcept override;

        void SetConfig(const CameraConfig &config) noexcept override { m_config = config; }
        const CameraConfig &GetConfig() const noexcept override { return m_config; }
    };

}
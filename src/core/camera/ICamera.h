#pragma once
#include <glm/glm.hpp>
#include "CameraConfig.h"

namespace cozy::core
{

    class ICamera
    {
    public:
        virtual ~ICamera() = default;

        // Core queries (used by Renderer)
        [[nodiscard]] virtual glm::mat4 GetViewMatrix() const noexcept = 0;
        [[nodiscard]] virtual glm::mat4 GetProjectionMatrix(float aspectRatio) const noexcept = 0;
        [[nodiscard]] virtual glm::vec3 GetPosition() const noexcept = 0;
        [[nodiscard]] virtual glm::vec3 GetFront() const noexcept = 0;
        [[nodiscard]] virtual glm::vec3 GetUp() const noexcept = 0;

        // Update (called by InputSystem or AI)
        virtual void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) noexcept = 0;
        virtual void ProcessKeyboard(glm::vec3 direction, float deltaTime) noexcept = 0;

        // Config
        virtual void SetConfig(const CameraConfig &config) noexcept = 0;
        virtual const CameraConfig &GetConfig() const noexcept = 0;
    };

} // namespace cozy::core
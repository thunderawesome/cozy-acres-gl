#pragma once
#include <glm/glm.hpp>

namespace cozy::core
{

    struct CameraConfig
    {
        float fovDegrees{45.0f}; // Classic AC feel
        float nearPlane{0.1f};
        float farPlane{1000.0f};
        float moveSpeed{8.0f}; // Smooth movement in large town
        float mouseSensitivity{0.1f};
        float pitchClamp{89.0f}; // Prevent flip

        // Animal Crossing-style top-down/isometric preset
        static constexpr CameraConfig IsometricPreset() noexcept
        {
            return {60.0f, 0.1f, 1000.0f, 10.0f, 0.05f, 80.0f};
        }

        // Free-fly debug preset
        static constexpr CameraConfig FreeFlyPreset() noexcept
        {
            return {45.0f, 0.1f, 1000.0f, 15.0f, 0.1f, 89.0f};
        }
    };

}
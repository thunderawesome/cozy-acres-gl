#pragma once

#include <glm/glm.hpp>

// Core camera constants
constexpr float CAMERA_SPEED = 5.0f;
constexpr float PITCH_CLAMP = 89.0f;

// Global camera state – position and orientation only
extern glm::vec3 cameraPos;
extern glm::vec3 cameraFront;
extern glm::vec3 cameraUp;
extern float yaw;
extern float pitch;

class Shader;

class Camera
{
public:
    void updateShaderUniforms(Shader &shader) const;
};
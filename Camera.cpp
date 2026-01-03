#include "Camera.h"
#include "Projection.h"
#include "Shader.h"
#include <glm/gtc/matrix_transform.hpp>

// Definitions of global camera state
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float yaw = -90.0f;
float pitch = 0.0f;

void Camera::updateShaderUniforms(Shader &shader) const
{
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    glm::mat4 projection = glm::perspective(glm::radians(FOV_DEGREES),
                                            ASPECT_RATIO,
                                            NEAR_PLANE,
                                            FAR_PLANE);

    shader.setMat4("view", view);
    shader.setMat4("projection", projection);
    shader.setVec3("viewPos", cameraPos);
}
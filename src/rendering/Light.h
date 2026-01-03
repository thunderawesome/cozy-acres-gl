#pragma once
#include <glm/glm.hpp>

namespace cozy::rendering
{
    struct DirectionalLight
    {
        glm::vec3 direction{-0.2f, -1.0f, -0.3f};
        glm::vec3 color{1.0f, 1.0f, 1.0f};
        float ambient{0.1f};
        float diffuse{0.8f};
        float specular{0.5f};
    };

    struct PointLight
    {
        glm::vec3 position;
        glm::vec3 color;
        float constant{1.0f};
        float linear{0.09f};
        float quadratic{0.032f};
    };
}
#pragma once
#include <glm/glm.hpp>

namespace cozy::rendering
{
    struct DirectionalLight
    {
        glm::vec3 direction{0.0f, 0.0f, 0.0f};
        glm::vec3 color{0.0f, 0.0f, 0.0f};
        float ambient{0.0};
        float diffuse{0.0f};
        float specular{0.0f};
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
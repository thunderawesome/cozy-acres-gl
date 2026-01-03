#pragma once
#include <glm/glm.hpp>

namespace cozy::rendering
{
    struct TileInstance
    {
        glm::mat4 modelMatrix;
        glm::vec3 color;
        float padding; // Keep 16-byte alignment for performance
    };
}
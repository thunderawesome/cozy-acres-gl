#pragma once
#include <glm/glm.hpp>
#include <vector>

namespace cozy::rendering::debug
{
    struct DebugVertex
    {
        glm::vec3 position;
        glm::vec3 color;
    };

    // Simple icosphere for point lights
    std::vector<DebugVertex> CreateIcosphere(float radius, const glm::vec3 &color);

    // Arrow for directional lights
    std::vector<DebugVertex> CreateArrow(const glm::vec3 &color);

    // Line from point A to B
    std::vector<DebugVertex> CreateLine(
        const glm::vec3 &start,
        const glm::vec3 &end,
        const glm::vec3 &color);
}
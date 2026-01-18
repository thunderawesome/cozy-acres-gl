#include "rendering/debug/DebugPrimitives.h"
#include <glm/gtc/constants.hpp>

namespace cozy::rendering::debug
{
    std::vector<DebugVertex> CreateIcosphere(float radius, const glm::vec3 &color)
    {
        std::vector<DebugVertex> vertices;

        // Simple octahedron (8 triangular faces)
        // You can use a proper icosphere generator for smoother spheres
        const float r = radius;

        glm::vec3 positions[] = {
            {0, r, 0},  // Top
            {r, 0, 0},  // Right
            {0, 0, r},  // Front
            {-r, 0, 0}, // Left
            {0, 0, -r}, // Back
            {0, -r, 0}  // Bottom
        };

        // Create triangles (simplified - full icosphere would have more)
        int indices[] = {
            0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 1, // Top pyramid
            5, 2, 1, 5, 3, 2, 5, 4, 3, 5, 1, 4  // Bottom pyramid
        };

        for (int i = 0; i < 24; i++)
        {
            vertices.push_back({positions[indices[i]], color});
        }

        return vertices;
    }

    std::vector<DebugVertex> CreateArrow(const glm::vec3 &color)
    {
        std::vector<DebugVertex> vertices;

        // Arrow shaft (line)
        vertices.push_back({{0.0f, 0.0f, 0.0f}, color});
        vertices.push_back({{0.0f, -1.0f, 0.0f}, color});

        // Arrow head (cone - simplified as lines)
        glm::vec3 tip = {0.0f, -1.0f, 0.0f};
        float headSize = 0.2f;

        for (int i = 0; i < 8; i++)
        {
            float angle = (i / 8.0f) * glm::two_pi<float>();
            glm::vec3 base = {
                headSize * std::cos(angle),
                -0.8f,
                headSize * std::sin(angle)};

            vertices.push_back({tip, color});
            vertices.push_back({base, color});
        }

        return vertices;
    }

    std::vector<DebugVertex> CreateLine(
        const glm::vec3 &start,
        const glm::vec3 &end,
        const glm::vec3 &color)
    {
        return {
            {start, color},
            {end, color}};
    }
}
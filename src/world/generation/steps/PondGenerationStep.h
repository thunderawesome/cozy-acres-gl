#pragma once

#include <random>
#include <vector>
#include <glm/glm.hpp>

namespace cozy::world
{
    class Town;
    struct TownConfig; // Forward declaration is fine as long as we don't need sizes here

    namespace ponds
    {
        // Composition: Encapsulates the specific parameters and shape logic
        struct PondBlob
        {
            glm::ivec2 center;
            float radius_x;
            float radius_z;
            int target_elevation;
            int seed;

            // SRP: Only handles the mathematical bounds check
            // Defined in .cpp
            bool IsPointInside(int wx, int wz, const TownConfig &config) const;
        };

        // Helper: Logic to ensure pond stays away from map edges and obstacles
        // Exporting this allows other generation steps (like Rivers) to check
        // if they are about to collide with a pond.
        bool IsAreaClearForPond(Town &town, glm::ivec2 center, int max_radius, const TownConfig &config);

        void Execute(Town &town, std::mt19937_64 &rng, const TownConfig &config);
    }
}
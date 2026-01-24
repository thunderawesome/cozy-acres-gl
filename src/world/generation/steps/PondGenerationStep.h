#pragma once

#include <random>
#include <vector>
#include <glm/glm.hpp>

namespace cozy::world
{
    class Town;
    struct TownConfig;

    namespace ponds
    {
        struct PondBlob
        {
            glm::ivec2 center;
            float radius_x;
            float radius_z;
            int target_elevation;
            int seed;

            bool IsPointInside(int wx, int wz, const TownConfig &config) const;
        };

        bool IsAreaClearForPond(Town &town, glm::ivec2 center, int max_radius, const TownConfig &config);

        void Execute(Town &town, std::mt19937_64 &rng, const TownConfig &config);
    }
}
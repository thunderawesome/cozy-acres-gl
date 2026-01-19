#pragma once

#include "world/Town.h"
#include "world/data/TownConfig.h"
#include <random>

namespace cozy::world
{
    namespace rivers
    {
        void CarveRiverSection(Town &town, int center_x, int center_z, int half_width);
        void CreateRiverMouths(Town &town, std::mt19937_64 &rng);

        void Execute(
            Town &town,
            std::mt19937_64 &rng,
            const TownConfig &config);

    }
}
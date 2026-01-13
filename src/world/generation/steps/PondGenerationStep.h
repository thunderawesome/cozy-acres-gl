#pragma once

#include "world/Town.h"
#include "world/data/TownConfig.h"
#include <random>

namespace cozy::world
{
    namespace ponds
    {

        void Execute(
            Town &town,
            std::mt19937_64 &rng,
            const TownConfig &config);

    }
}
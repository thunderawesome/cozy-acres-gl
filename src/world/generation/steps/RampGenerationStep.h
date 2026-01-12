#pragma once

#include <random>

namespace cozy::world
{
    class Town;
    struct TownConfig;

    namespace ramps
    {
        void Execute(
            Town &town,
            std::mt19937_64 &rng,
            const TownConfig &config);
    }
}
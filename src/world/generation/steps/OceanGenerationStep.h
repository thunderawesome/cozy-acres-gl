#pragma once

#include <random>

namespace cozy::world
{
    class Town;
    struct TownConfig;

    namespace ocean
    {
        // Execute ocean generation - should run BEFORE river generation
        // so the river knows where the ocean is and can create the river mouth
        void Execute(
            Town &town,
            std::mt19937_64 &rng,
            const TownConfig &config);
    }
}
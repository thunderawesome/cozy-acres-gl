#pragma once

#include <random>

namespace cozy::world
{

    struct TownConfig;
    class Town;

    namespace cliffs
    {
        void Execute(Town &town, std::mt19937_64 &rng, const TownConfig &config);
    }

}
// world/generation/steps/RiverGenerationStep.h
#pragma once

#include "world/Town.h"
#include "world/data/TownConfig.h"
#include <random>

namespace cozy::world
{
    namespace rivers
    {

        void CreateRiverMouths(Town &town, std::mt19937_64 &rng);

        void Execute(
            Town &town,
            std::mt19937_64 &rng,
            const TownConfig &config);

    }
}
#pragma once

#include <glm/glm.hpp>

namespace cozy::world
{
    struct TownConfig
    {
        // Cliff parameters
        float cliffSmoothness = 0.25f; // 0.0 = sharp → 1.0 = very smooth transitions
        int minPlateauRow = 1;         // Usually row B (0-based)
        int maxPlateauRow = 4;         // Usually row E
        int minHighPlateauRowOffset = 1;
        int maxHighPlateauRowOffset = 2;
        float highPlateauChance = 0.75f; // 0.0f-1.0f - Probability of having the third (highest) tier

        static constexpr int CLIFF_CONNECTION_POINT_OFFSET = 12;
        static constexpr int RIVER_CONNECTION_POINT_OFFSET = 3;

        // River parameters
        int riverWidth = 3;
        int riverMeanderChance = 50;    // 0–100 %
        int riverHorizontalChance = 50; // 0–100 %

        // Pond parameters
        int minPondRadius = 2;
        int maxPondRadius = 3;
    };
}
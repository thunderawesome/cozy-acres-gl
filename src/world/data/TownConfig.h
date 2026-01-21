#pragma once
#include <glm/glm.hpp>
namespace cozy::world
{
    struct TownConfig
    {
        // Cliff parameters
        int cliffVariationAmount = 2; // In tiles, for organic edges
        int cliffSmoothIterations = 1;
        static constexpr float CLIFF_NOISE_SCALE = 0.1f;
        int minPlateauRow = 1; // Usually row B (0-based)
        int maxPlateauRow = 4; // Usually row E
        int minHighPlateauRowOffset = 1;
        int maxHighPlateauRowOffset = 2;
        float highPlateauChance = 0.75f; // 0.0f-1.0f - Probability of having the third (highest) tier
        static constexpr int CLIFF_CONNECTION_POINT_OFFSET = 12;
        static constexpr int RIVER_CONNECTION_POINT_OFFSET = 6;

        // Ramp parameters
        static constexpr int RAMP_LENGTH = 5;               // Tiles long (Z direction)
        static constexpr int RAMP_CORRIDOR_HALF_WIDTH = 3;  // ±3 tiles = 7 total width
        static constexpr int RAMP_NORMALIZE_HALF_WIDTH = 2; // ±2 tiles for side normalization
        int rampWaterClearance = 2;                         // Minimum buffer from water features
        int rampTopCandidates = 3;                          // Consider top N candidates when placing

        // River parameters
        int riverWidth = 3;
        int riverMeanderChance = 50;    // 0–100 %
        int riverHorizontalChance = 50; // 0–100 %

        // Pond parameters
        int minPondRadius = 2;
        int maxPondRadius = 3;
    };
}
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
        static constexpr int RAMP_CORRIDOR_HALF_WIDTH = 1;  // ±3 tiles = 7 total width
        static constexpr int RAMP_NORMALIZE_HALF_WIDTH = 1; // ±2 tiles for side normalization
        int rampBuffer = 3;                                 // Minimum buffer from water features, obstacles, and bounds
        int rampTopCandidates = 3;                          // Consider top N candidates when placing

        // River parameters
        int riverWidth = 3;
        int riverMeanderChance = 50;    // 0–100 %
        int riverHorizontalChance = 50; // 0–100 %

        // Pond parameters
        int minPondRadius = 4;
        int maxPondRadius = 5;
        float pondNoiseScale = 0.05f;   // How frequent the "wiggles" are (0.05 - 0.2)
        float pondNoiseStrength = 3.0f; // How deep the coves/protrusions are
        int pondMargin = 6;             // Extra tiles to scan around the radius
        int pondMinNeighbors = 1;       // Minimum water neighbors to avoid being deleted
    };
}
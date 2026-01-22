#pragma once
#include "world/Town.h"
#include "world/data/TownConfig.h"
#include "world/generation/utils/WorldGenUtils.h"
#include <random>
#include <vector>
#include <unordered_set>

namespace cozy::world
{
    namespace rivers
    {
        // Public API functions
        void CarveRiverSection(
            Town &town,
            int center_x,
            int center_z,
            int half_width,
            std::unordered_set<glm::ivec2, utils::PairHash> &painted_tracker);
        void CreateRiverMouths(Town &town, std::mt19937_64 &rng);
        void Execute(Town &town, std::mt19937_64 &rng, const TownConfig &config);

        // Helper functions
        int CalculateWiggle(int position, float amplitude, float frequency, float phase);
        int CalculateRiverWidth(int z, int base_width, float variation_amplitude, float variation_frequency, float variation_phase);
        int GetBaseRiverX(int z, const std::vector<int> &column_targets);
        int ApplyCornerRounding(int z, int base_x, const std::vector<int> &column_targets);
        void FillGaps(Town &town, int from_x, int to_x, int z, int half_width,
                      std::unordered_set<glm::ivec2, utils::PairHash> &tracker);
        bool CheckPathValid(const Town &town, int acre_z, int entry_col, int exit_col);
    }
}
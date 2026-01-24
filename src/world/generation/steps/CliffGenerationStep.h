#pragma once
#include <random>
#include <vector>

namespace cozy::world
{
    struct TownConfig;
    class Town;

    namespace cliffs
    {
        // Internal helper to handle the math of the cliff line
        struct CliffBoundary
        {
            std::vector<int> z_values;

            void Generate(const TownConfig &config, std::mt19937_64 &rng, const std::vector<int> &targets);
            void Smooth(int iterations);
            void RoundCorners(const std::vector<int> &targets, int connection_point);
        };

        void Execute(Town &town, std::mt19937_64 &rng, const TownConfig &config);
    }
}
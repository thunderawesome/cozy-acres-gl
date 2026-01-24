#include "CliffGenerationStep.h"
#include "world/generation/utils/WorldGenUtils.h"
#include "world/Town.h"
#include "world/data/Acre.h"
#include "world/data/Tile.h"
#include "world/data/TownConfig.h"

#include <algorithm>
#include <array>

namespace cozy::world::cliffs
{
    // --- CliffBoundary Logic (Composition) ---

    void CliffBoundary::Generate(const TownConfig &config, std::mt19937_64 &rng, const std::vector<int> &targets)
    {
        const int total_width = utils::GetWorldWidth();
        z_values.assign(total_width, 0);

        std::uniform_int_distribution<int> seed_dist(0, 100000);
        int seed = seed_dist(rng);

        for (int x = 0; x < total_width; ++x)
        {
            int curr_acre = x / Acre::SIZE;
            int next_acre = std::min(curr_acre + 1, Town::WIDTH - 1);
            int local_x = x % Acre::SIZE;

            int base_z = (local_x <= config.CLIFF_CONNECTION_POINT_OFFSET) ? targets[curr_acre] : targets[next_acre];

            float noise = utils::SmoothNoise(x * TownConfig::CLIFF_NOISE_SCALE, 0, seed);
            int variation = static_cast<int>((noise - 0.5f) * 2.0f * config.cliffVariationAmount);

            z_values[x] = base_z + variation;
        }
    }

    void CliffBoundary::Smooth(int iterations)
    {
        for (int iter = 0; iter < iterations; ++iter)
        {
            std::vector<int> temp = z_values;
            for (int x = 2; x < (int)z_values.size() - 2; ++x)
            {
                temp[x] = (z_values[x - 2] + z_values[x - 1] + z_values[x] + z_values[x + 1] + z_values[x + 2]) / 5;
            }
            z_values = std::move(temp);
        }
    }

    void CliffBoundary::RoundCorners(const std::vector<int> &targets, int connection_point)
    {
        int width = z_values.size();
        int radius = 8;

        for (int ax = 0; ax < Town::WIDTH - 1; ++ax)
        {
            if (targets[ax] == targets[ax + 1])
                continue;

            int transition_x = ax * Acre::SIZE + connection_point;
            for (int x = std::max(0, transition_x - radius); x <= std::min(width - 1, transition_x + radius); ++x)
            {
                int sum = 0, count = 0;
                for (int sx = std::max(0, x - 3); sx <= std::min(width - 1, x + 3); ++sx)
                {
                    sum += z_values[sx];
                    count++;
                }
                z_values[x] = sum / count;
            }
        }
    }

    // --- Mutation Helpers (Single Responsibility) ---

    void ApplyElevations(Town &town, const CliffBoundary &mid, const CliffBoundary &high, bool threeTiers)
    {
        for (int wx = 0; wx < utils::GetWorldWidth(); ++wx)
        {
            for (int wz = 0; wz < utils::GetWorldHeight(); ++wz)
            {
                int elevation = 0;
                if (wz < mid.z_values[wx])
                    elevation = 1;
                if (threeTiers && wz < high.z_values[wx])
                    elevation = 2;

                if (auto *tile = utils::GetTileSafe(town, wx, wz))
                {
                    tile->elevation = static_cast<std::int8_t>(elevation);
                }
            }
        }
    }

    void TagCliffFaces(Town &town)
    {
        for (int wx = 0; wx < utils::GetWorldWidth(); ++wx)
        {
            for (int wz = 0; wz < utils::GetWorldHeight(); ++wz)
            {
                auto *tile = utils::GetTileSafe(town, wx, wz);
                if (!tile || tile->elevation <= 0)
                    continue;

                for (auto &neighborPos : utils::GetNeighbors4(wx, wz))
                {
                    if (auto *nt = utils::GetTileSafe(town, neighborPos.x, neighborPos.y))
                    {
                        if (nt->elevation < tile->elevation)
                        {
                            tile->type = TileType::CLIFF;
                            break;
                        }
                    }
                }
            }
        }
    }

    // --- Main Orchestration (Dependency Injection via Town/Config) ---

    void Execute(Town &town, std::mt19937_64 &rng, const TownConfig &config)
    {
        std::bernoulli_distribution high_dist(config.highPlateauChance);
        bool use_three_tiers = high_dist(rng);

        auto GenerateTargets = [&](int minRow)
        {
            std::vector<int> targets(Town::WIDTH);
            std::uniform_int_distribution<int> dist(minRow, config.maxPlateauRow);
            for (int i = 0; i < Town::WIDTH; ++i)
                targets[i] = (dist(rng) + 1) * Acre::SIZE;
            return targets;
        };

        // Mid Plateau
        auto mid_targets = GenerateTargets(use_three_tiers ? config.minPlateauRow + 2 : config.minPlateauRow);
        CliffBoundary mid;
        mid.Generate(config, rng, mid_targets);
        mid.Smooth(config.cliffSmoothIterations);
        mid.RoundCorners(mid_targets, config.CLIFF_CONNECTION_POINT_OFFSET);

        // High Plateau
        CliffBoundary high;
        if (use_three_tiers)
        {
            std::vector<int> high_targets(Town::WIDTH);
            std::uniform_int_distribution<int> h_dist(config.minHighPlateauRowOffset, config.maxHighPlateauRowOffset);
            for (int i = 0; i < Town::WIDTH; ++i)
            {
                int candidate = (h_dist(rng) + 1) * Acre::SIZE;
                high_targets[i] = std::clamp(candidate, Acre::SIZE, mid_targets[i] - Acre::SIZE);
            }
            high.Generate(config, rng, high_targets);
            high.Smooth(config.cliffSmoothIterations);
            high.RoundCorners(high_targets, config.CLIFF_CONNECTION_POINT_OFFSET);
        }

        ApplyElevations(town, mid, high, use_three_tiers);
        TagCliffFaces(town);
    }
}
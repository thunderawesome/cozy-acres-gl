#include "CliffGenerationStep.h"
#include "world/generation/utils/WorldGenUtils.h"

#include "world/Town.h"
#include "world/data/Acre.h"
#include "world/data/Tile.h"
#include "world/data/TownConfig.h"

#include <algorithm>
#include <array>
#include <vector>
#include <random>
#include <cstdint>
#include <cmath>
#include <glm/glm.hpp>

namespace cozy::world
{
    namespace cliffs
    {
        namespace
        {
            inline std::pair<glm::ivec2, glm::ivec2> GetTileCoords(int wx, int wz)
            {
                return {
                    {wx / Acre::SIZE, wz / Acre::SIZE},
                    {wx % Acre::SIZE, wz % Acre::SIZE}};
            }

            // Build stepped boundary with organic variation
            std::vector<int> BuildOrganicBoundary(
                const std::array<int, Town::WIDTH> &targets,
                int connection_point,
                std::mt19937_64 &rng,
                int variation_amount = 3)
            {
                const int total_width = Town::WIDTH * Acre::SIZE;
                std::vector<int> boundary(total_width);

                std::uniform_int_distribution<int> seed_dist(0, 100000);
                int seed = seed_dist(rng);

                for (int x = 0; x < total_width; ++x)
                {
                    int curr_acre = x / Acre::SIZE;
                    int next_acre = std::min(curr_acre + 1, Town::WIDTH - 1);
                    int local_x = x % Acre::SIZE;

                    // Get base stepped value
                    int base_z = (local_x <= connection_point)
                                     ? targets[curr_acre]
                                     : targets[next_acre];

                    // Add organic variation using noise
                    float noise = utils::SmoothNoise(x * TownConfig::CLIFF_NOISE_SCALE, 0, seed);
                    // Map noise from [0,1] to [-variation, +variation]
                    int variation = static_cast<int>((noise - 0.5f) * 2.0f * variation_amount);

                    boundary[x] = base_z + variation;
                }

                return boundary;
            }

            // Smooth boundary horizontally for gentler curves
            void SmoothBoundary(std::vector<int> &boundary, int iterations = 3)
            {
                int width = boundary.size();

                for (int iter = 0; iter < iterations; ++iter)
                {
                    std::vector<int> temp = boundary;

                    for (int x = 2; x < width - 2; ++x)
                    {
                        // 5-point moving average for smooth curves
                        int sum = boundary[x - 2] + boundary[x - 1] + boundary[x] +
                                  boundary[x + 1] + boundary[x + 2];
                        temp[x] = sum / 5;
                    }

                    boundary = temp;
                }
            }

            // Round corners by smoothing the boundary near acre transitions
            void RoundBoundaryCorners(
                std::vector<int> &boundary,
                const std::array<int, Town::WIDTH> &targets,
                int connection_point)
            {
                int width = boundary.size();

                // Find acre transition points and apply extra smoothing
                for (int ax = 0; ax < Town::WIDTH - 1; ++ax)
                {
                    if (targets[ax] == targets[ax + 1])
                        continue; // No elevation change, skip

                    // Find the transition region
                    int transition_x = ax * Acre::SIZE + connection_point;
                    int radius = 8; // Smooth 8 tiles on each side of transition

                    for (int x = std::max(0, transition_x - radius);
                         x <= std::min(width - 1, transition_x + radius); ++x)
                    {
                        int left_x = std::max(0, x - 3);
                        int right_x = std::min(width - 1, x + 3);

                        // Extra smoothing at corners
                        int sum = 0;
                        int count = 0;
                        for (int sx = left_x; sx <= right_x; ++sx)
                        {
                            sum += boundary[sx];
                            count++;
                        }

                        boundary[x] = sum / count;
                    }
                }
            }

            // Apply elevation based on boundaries with smooth transitions
            void ApplyElevations(
                Town &town,
                const std::vector<int> &mid_boundary,
                const std::vector<int> &high_boundary,
                bool use_three_tiers)
            {
                const int w = Town::WIDTH * Acre::SIZE;
                const int h = Town::HEIGHT * Acre::SIZE;

                for (int wx = 0; wx < w; ++wx)
                {
                    for (int wz = 0; wz < h; ++wz)
                    {
                        int elevation = 0;

                        if (wz < mid_boundary[wx])
                            elevation = 1;
                        if (use_three_tiers && wz < high_boundary[wx])
                            elevation = 2;

                        auto [acre_pos, local_pos] = GetTileCoords(wx, wz);
                        town.GetAcre(acre_pos.x, acre_pos.y)
                            .tiles[local_pos.y][local_pos.x]
                            .elevation = static_cast<std::int8_t>(elevation);
                    }
                }
            }

            void TagCliffFaces(Town &town)
            {
                const int w = Town::WIDTH * Acre::SIZE;
                const int h = Town::HEIGHT * Acre::SIZE;

                constexpr int dx[4] = {1, -1, 0, 0};
                constexpr int dz[4] = {0, 0, 1, -1};

                for (int wx = 0; wx < w; ++wx)
                {
                    for (int wz = 0; wz < h; ++wz)
                    {
                        auto [a, l] = GetTileCoords(wx, wz);
                        Tile &tile = town.GetAcre(a.x, a.y).tiles[l.y][l.x];

                        if (tile.elevation <= 0)
                            continue;

                        bool is_cliff = false;
                        for (int d = 0; d < 4; ++d)
                        {
                            int nx = wx + dx[d];
                            int nz = wz + dz[d];

                            if (nx < 0 || nx >= w || nz < 0 || nz >= h)
                                continue;

                            auto [na, nl] = GetTileCoords(nx, nz);
                            if (town.GetAcre(na.x, na.y).tiles[nl.y][nl.x].elevation < tile.elevation)
                            {
                                is_cliff = true;
                                break;
                            }
                        }

                        if (is_cliff)
                        {
                            tile.type = TileType::CLIFF;
                        }
                    }
                }
            }

        } // anonymous namespace

        void Execute(
            Town &town,
            std::mt19937_64 &rng,
            const TownConfig &config)
        {
            const int total_width = Town::WIDTH * Acre::SIZE;

            // 1. Decide structure
            std::bernoulli_distribution high_plateau_dist(config.highPlateauChance);
            bool use_three_tiers = high_plateau_dist(rng);

            int minPlateauRow = config.minPlateauRow;
            if (use_three_tiers)
            {
                minPlateauRow += 2;
            }

            // 2. Generate acre-level targets (maintains connection rules)
            std::uniform_int_distribution<int> mid_dist(
                minPlateauRow,
                config.maxPlateauRow);

            std::array<int, Town::WIDTH> mid_targets{};
            for (int ax = 0; ax < Town::WIDTH; ++ax)
            {
                mid_targets[ax] = (mid_dist(rng) + 1) * Acre::SIZE;
            }

            // 3. Build organic boundary from targets
            auto mid_boundary = BuildOrganicBoundary(
                mid_targets,
                config.CLIFF_CONNECTION_POINT_OFFSET,
                rng,
                config.cliffVariationAmount); // variation_amount in tiles

            // 4. Smooth for gentle curves
            SmoothBoundary(mid_boundary, config.cliffSmoothIterations);

            // 5. Round corners at acre transitions
            RoundBoundaryCorners(mid_boundary, mid_targets, config.CLIFF_CONNECTION_POINT_OFFSET);

            // 6. Optional high plateau
            std::vector<int> high_boundary(total_width, 0);
            if (use_three_tiers)
            {
                std::uniform_int_distribution<int> high_dist(
                    config.minHighPlateauRowOffset,
                    config.maxHighPlateauRowOffset);

                std::array<int, Town::WIDTH> high_targets{};
                for (int ax = 0; ax < Town::WIDTH; ++ax)
                {
                    int candidate = (high_dist(rng) + 1) * Acre::SIZE;
                    int max_allowed = mid_targets[ax] - Acre::SIZE;
                    high_targets[ax] = std::clamp(candidate, Acre::SIZE, max_allowed);
                }

                high_boundary = BuildOrganicBoundary(
                    high_targets,
                    config.CLIFF_CONNECTION_POINT_OFFSET,
                    rng,
                    config.cliffVariationAmount);

                SmoothBoundary(high_boundary, config.cliffSmoothIterations);
                RoundBoundaryCorners(high_boundary, high_targets, config.CLIFF_CONNECTION_POINT_OFFSET);

                // Safety clamp
                for (int x = 0; x < total_width; ++x)
                {
                    int acre_x = x / Acre::SIZE;
                    int safe_max = mid_targets[acre_x] - Acre::SIZE;
                    high_boundary[x] = std::min(high_boundary[x], safe_max);
                    high_boundary[x] = std::max(high_boundary[x], 0);
                }
            }

            // 7. Apply elevations
            ApplyElevations(town, mid_boundary, high_boundary, use_three_tiers);

            // 8. Tag cliff faces
            TagCliffFaces(town);
        }

    } // namespace cliffs
} // namespace cozy::world
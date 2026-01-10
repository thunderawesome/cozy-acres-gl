#include "CliffGenerationStep.h"

#include "world/Town.h"
#include "world/data/Acre.h"
#include "world/data/Tile.h"
#include "world/data/TownConfig.h"

#include <algorithm>
#include <array>
#include <vector>
#include <random>
#include <cstdint> // for std::int8_t
#include <glm/glm.hpp>

namespace cozy::world
{
    namespace cliffs
    {
        // ── Helper functions ────────────────────────────────────────────────────────
        namespace
        {

            inline std::pair<glm::ivec2, glm::ivec2> GetTileCoords(int wx, int wz)
            {
                return {
                    {wx / Acre::SIZE, wz / Acre::SIZE},
                    {wx % Acre::SIZE, wz % Acre::SIZE}};
            }

            std::vector<int> BuildSteppedBoundary(
                const std::array<int, Town::WIDTH> &targets,
                int connection_point)
            {
                const int total_width = Town::WIDTH * Acre::SIZE;
                std::vector<int> boundary(total_width);

                for (int x = 0; x < total_width; ++x)
                {
                    int curr_acre = x / Acre::SIZE;
                    int next_acre = std::min(curr_acre + 1, Town::WIDTH - 1);
                    int local_x = x % Acre::SIZE;

                    boundary[x] = (local_x <= connection_point)
                                      ? targets[curr_acre]
                                      : targets[next_acre];
                }

                return boundary;
            }

            int ComputeSnappedElevation(
                int wx, int wz, int naive_elev,
                const std::vector<int> &mid_line,
                const std::vector<int> &high_line,
                bool has_high,
                int conn_point)
            {
                int local_z = wz % Acre::SIZE;
                if (local_z == conn_point)
                {
                    return naive_elev;
                }

                int curr_acre_z = wz / Acre::SIZE;
                int target_acre_z = (local_z < conn_point) ? curr_acre_z : curr_acre_z + 1;

                if (target_acre_z >= Town::HEIGHT)
                {
                    return naive_elev;
                }

                int check_z = target_acre_z * Acre::SIZE + conn_point;
                if (check_z >= Town::HEIGHT * Acre::SIZE)
                {
                    return naive_elev;
                }

                int result = 0;
                if (check_z < mid_line[wx])
                    result = 1;
                if (has_high && check_z < high_line[wx])
                    result = 2;

                return result;
            }

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
                        int naive = 0;
                        if (wz < mid_boundary[wx])
                            naive = 1;
                        if (use_three_tiers && wz < high_boundary[wx])
                            naive = 2;

                        int snapped = ComputeSnappedElevation(
                            wx, wz, naive,
                            mid_boundary, high_boundary,
                            use_three_tiers,
                            TownConfig::CLIFF_CONNECTION_POINT_OFFSET);

                        auto [acre_pos, local_pos] = GetTileCoords(wx, wz);
                        town.GetAcre(acre_pos.x, acre_pos.y)
                            .tiles[local_pos.y][local_pos.x]
                            .elevation = static_cast<std::int8_t>(snapped);
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

        // ── Public interface ────────────────────────────────────────────────────────
        void Execute(
            Town &town,
            std::mt19937_64 &rng,
            const TownConfig &config)
        {
            const int total_width = Town::WIDTH * Acre::SIZE;
            const int total_height = Town::HEIGHT * Acre::SIZE;

            // 1. Decide structure
            std::bernoulli_distribution high_plateau_dist(config.highPlateauChance);
            bool use_three_tiers = high_plateau_dist(rng);

            int minPlateauRow = config.minPlateauRow;

            // 2. Mid plateau (always present)
            if (use_three_tiers)
            {
                minPlateauRow += 2;
            }
            std::uniform_int_distribution<int> mid_dist(
                minPlateauRow,
                config.maxPlateauRow);

            std::array<int, Town::WIDTH> mid_targets{};
            for (int ax = 0; ax < Town::WIDTH; ++ax)
            {
                mid_targets[ax] = (mid_dist(rng) + 1) * Acre::SIZE;
            }

            auto mid_boundary = BuildSteppedBoundary(mid_targets, config.CLIFF_CONNECTION_POINT_OFFSET);

            // 3. Optional high plateau
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

                high_boundary = BuildSteppedBoundary(high_targets, config.CLIFF_CONNECTION_POINT_OFFSET);

                // Safety clamp
                for (int x = 0; x < total_width; ++x)
                {
                    int acre_x = x / Acre::SIZE;
                    int safe_max = mid_targets[acre_x] - Acre::SIZE;
                    high_boundary[x] = std::min(high_boundary[x], safe_max);
                    high_boundary[x] = std::max(high_boundary[x], 0);
                }
            }

            // 4. Apply & tag
            ApplyElevations(town, mid_boundary, high_boundary, use_three_tiers);
            TagCliffFaces(town);
        }

    }
}
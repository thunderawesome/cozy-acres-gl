#include "RiverGenerationStep.h"

#include "world/Town.h"
#include "world/data/Acre.h"
#include "world/data/Tile.h"
#include "world/data/TownConfig.h"
#include "world/generation/utils/WorldGenUtils.h"
#include "world/generation/utils/AutoTileUtils.h"

#include <vector>
#include <algorithm>
#include <random>
#include <cmath>
#include <unordered_set>
#include <glm/glm.hpp>

namespace cozy::world
{
    namespace rivers
    {
        // Calculate wiggle offset using sine wave
        int CalculateWiggle(int position, float amplitude, float frequency, float phase)
        {
            float offset = amplitude * std::sin(frequency * position + phase);
            return static_cast<int>(std::round(offset));
        }

        // Get base X position for a given Z coordinate
        int GetBaseRiverX(int z, const std::vector<int> &column_targets)
        {
            int curr_acre = z / Acre::SIZE;
            int next_acre = std::min(curr_acre + 1, static_cast<int>(column_targets.size()) - 1);
            int local_z = z % Acre::SIZE;

            int target_col = (local_z < TownConfig::RIVER_CONNECTION_POINT_OFFSET)
                                 ? column_targets[curr_acre]
                                 : column_targets[next_acre];

            return target_col * Acre::SIZE + TownConfig::RIVER_CONNECTION_POINT_OFFSET;
        }

        // Apply corner rounding to a position
        int ApplyCornerRounding(int z, int base_x, const std::vector<int> &column_targets)
        {
            int curr_acre = z / Acre::SIZE;
            int local_z = z % Acre::SIZE;

            if (curr_acre >= static_cast<int>(column_targets.size()) - 1)
                return base_x;

            int from_col = column_targets[curr_acre];
            int to_col = column_targets[curr_acre + 1];

            // No corner rounding needed if going straight
            if (from_col == to_col)
                return base_x;

            // Define corner region (larger radius for gentler curves)
            int corner_start = TownConfig::RIVER_CONNECTION_POINT_OFFSET - TownConfig::RIVER_CORNER_RADIUS;
            int corner_end = TownConfig::RIVER_CONNECTION_POINT_OFFSET + TownConfig::RIVER_CORNER_RADIUS;

            // Only apply rounding within the corner region
            if (local_z < corner_start || local_z > corner_end)
                return base_x;

            // Calculate smooth interpolation factor
            float t = static_cast<float>(local_z - corner_start) / (corner_end - corner_start);
            // Apply smoothstep twice for even smoother curves
            t = utils::SmoothStep(utils::SmoothStep(utils::SmoothStep(t)));

            // Interpolate between start and end positions
            int from_x = from_col * Acre::SIZE + TownConfig::RIVER_CONNECTION_POINT_OFFSET;
            int to_x = to_col * Acre::SIZE + TownConfig::RIVER_CONNECTION_POINT_OFFSET;

            return static_cast<int>(from_x + t * (to_x - from_x));
        }

        // Fill gaps between consecutive river positions
        void FillGaps(Town &town, int from_x, int to_x, int z, int half_width,
                      std::unordered_set<glm::ivec2, utils::PairHash> &tracker)
        {
            if (from_x == to_x)
                return;

            int dir = (to_x > from_x) ? 1 : -1;
            for (int x = from_x; x != to_x; x += dir)
            {
                CarveRiverSection(town, x, z, half_width, tracker);
            }
        }

        bool CheckPathValid(
            const Town &town,
            int acre_z,
            int entry_col,
            int exit_col)
        {
            const int entry_x = entry_col * Acre::SIZE + TownConfig::RIVER_CONNECTION_POINT_OFFSET;
            const int exit_x = exit_col * Acre::SIZE + TownConfig::RIVER_CONNECTION_POINT_OFFSET;
            const int base_z = acre_z * Acre::SIZE;

            int river_elev = town.GetElevation(entry_x, base_z);
            if (river_elev == -1)
                return false;

            auto update = [&](int x, int z) -> bool
            {
                int elev = town.GetElevation(x, z);
                if (elev == -1 || elev > river_elev)
                    return false;
                river_elev = std::min(river_elev, elev);
                return true;
            };

            // Vertical segment before bend
            for (int lz = 0; lz < TownConfig::RIVER_CONNECTION_POINT_OFFSET; ++lz)
            {
                if (!update(entry_x, base_z + lz))
                    return false;
            }

            // Bend point
            if (!update(entry_x, base_z + TownConfig::RIVER_CONNECTION_POINT_OFFSET))
                return false;

            // Horizontal segment (bend)
            int dx = (exit_x > entry_x) ? 1 : ((exit_x < entry_x) ? -1 : 0);
            int steps = std::abs(exit_x - entry_x);
            int curr_x = entry_x;
            for (int i = 0; i < steps; ++i)
            {
                curr_x += dx;
                if (!update(curr_x, base_z + TownConfig::RIVER_CONNECTION_POINT_OFFSET))
                    return false;
            }

            // Vertical segment after bend
            for (int lz = TownConfig::RIVER_CONNECTION_POINT_OFFSET + 1; lz < Acre::SIZE; ++lz)
            {
                if (!update(exit_x, base_z + lz))
                    return false;
            }

            return true;
        }

        void CarveRiverSection(
            Town &town,
            int center_x,
            int center_z,
            int half_width,
            std::unordered_set<glm::ivec2, utils::PairHash> &painted_tracker) // Added tracker
        {
            const int w = Town::WIDTH * Acre::SIZE;
            const int h = Town::HEIGHT * Acre::SIZE;

            for (int dx = -half_width; dx <= half_width; ++dx)
            {
                for (int dz = -half_width; dz <= half_width; ++dz)
                {
                    int wx = center_x + dx;
                    int wz = center_z + dz;

                    if (wx >= 0 && wx < w && wz >= 0 && wz < h)
                    {
                        auto [a, l] = utils::GetTileCoords(wx, wz);
                        Tile &tile = town.GetAcre(a.x, a.y).tiles[l.y][l.x];

                        if (tile.type == TileType::OCEAN)
                            continue;

                        bool is_cliff_drop = false;
                        const int neighbor_offsets[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
                        for (auto &off : neighbor_offsets)
                        {
                            int nx = wx + off[0];
                            int nz = wz + off[1];
                            if (nx >= 0 && nx < w && nz >= 0 && nz < h)
                            {
                                int neighbor_elev = town.GetElevation(nx, nz);
                                if (tile.elevation > neighbor_elev && neighbor_elev != -1)
                                {
                                    is_cliff_drop = true;
                                    break;
                                }
                            }
                        }

                        if (is_cliff_drop || tile.type == TileType::CLIFF)
                            tile.type = TileType::WATERFALL;
                        else if (tile.type == TileType::SAND)
                            tile.type = TileType::RIVER_MOUTH;
                        else
                            tile.type = TileType::RIVER;

                        // Track this tile for the auto-tiling pass
                        painted_tracker.insert({wx, wz});
                    }
                }
            }
        }

        void CreateRiverMouths(Town &town, std::mt19937_64 &rng)
        {
            const int total_w = Town::WIDTH * Acre::SIZE;
            const int total_h = Town::HEIGHT * Acre::SIZE;
            const int ocean_acre_row = Town::BEACH_ACRE_ROW;

            std::vector<int> mouth_x_coords;
            int mouth_z = -1;

            // Detect where the river first hits the beach/ocean
            for (int z = 0; z < total_h; ++z)
            {
                for (int x = 0; x < total_w; ++x)
                {
                    auto [a, l] = utils::GetTileCoords(x, z);
                    Tile &tile = town.GetAcre(a.x, a.y).tiles[l.y][l.x];

                    if (tile.type != TileType::RIVER)
                        continue;

                    bool touches_ocean_or_sand = false;
                    const int offsets[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

                    for (auto &off : offsets)
                    {
                        int nx = x + off[0];
                        int nz = z + off[1];
                        if (nx < 0 || nx >= total_w || nz < 0 || nz >= total_h)
                            continue;

                        auto [na, nl] = utils::GetTileCoords(nx, nz);
                        const Tile &nTile = town.GetAcre(na.x, na.y).tiles[nl.y][nl.x];

                        if (nTile.type == TileType::OCEAN || nTile.type == TileType::SAND)
                        {
                            touches_ocean_or_sand = true;
                            break;
                        }
                    }

                    if (touches_ocean_or_sand)
                    {
                        tile.type = TileType::RIVER_MOUTH;

                        if (mouth_z == -1 || mouth_z == z)
                        {
                            mouth_z = z;
                            mouth_x_coords.push_back(x);
                        }
                    }
                }
            }

            // 2. Spawn the rounded grass inlets (Teardrops)
            if (!mouth_x_coords.empty())
            {
                // Start higher up to tuck the base under mainland grass
                int start_z_local = (mouth_z % Acre::SIZE) - 2;

                // 3. Cleanup Pass: Prevent sand/river contact and isolated sand pockets
                for (int z = 0; z < total_h; ++z)
                {
                    for (int x = 0; x < total_w; ++x)
                    {
                        auto [a, l] = utils::GetTileCoords(x, z);
                        Tile &tile = town.GetAcre(a.x, a.y).tiles[l.y][l.x];

                        // Remove Sand touching the River/Mouth — expanded to ~3 tiles radius
                        if (tile.type == TileType::RIVER || tile.type == TileType::RIVER_MOUTH)
                        {
                            const int RADIUS = 3;

                            for (int dz = -RADIUS; dz <= RADIUS; ++dz)
                            {
                                for (int dx = -RADIUS; dx <= RADIUS; ++dx)
                                {
                                    // Optional: diamond shape (Manhattan distance) — looks more natural
                                    if (std::abs(dx) + std::abs(dz) > RADIUS)
                                        continue;

                                    int nx = x + dx;
                                    int nz = z + dz;

                                    if (nx < 0 || nx >= total_w || nz < 0 || nz >= total_h)
                                        continue;

                                    auto [na, nl] = utils::GetTileCoords(nx, nz);
                                    Tile &nTile = town.GetAcre(na.x, na.y).tiles[nl.y][nl.x];

                                    if (nTile.type == TileType::SAND)
                                    {
                                        nTile.type = TileType::GRASS;
                                        nTile.elevation = 0; // keep it flat
                                    }
                                }
                            }
                        }

                        // Remove isolated Sand trapped by Grass
                        if (tile.type == TileType::SAND)
                        {
                            int grass_adj = 0;
                            const int n_off[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
                            for (auto &off : n_off)
                            {
                                int nx = x + off[0], nz = z + off[1];
                                if (nx < 0 || nx >= total_w || nz < 0 || nz >= total_h)
                                {
                                    grass_adj++; // Count map edge as grass for cleanup
                                    continue;
                                }
                                auto [na, nl] = utils::GetTileCoords(nx, nz);
                                if (town.GetAcre(na.x, na.y).tiles[nl.y][nl.x].type == TileType::GRASS)
                                {
                                    grass_adj++;
                                }
                            }

                            if (grass_adj == 4)
                            {
                                tile.type = TileType::GRASS;
                            }
                        }
                    }
                }
            }
        }

        void Execute(
            Town &town,
            std::mt19937_64 &rng,
            const TownConfig &config)
        {
            const int width = config.riverWidth;
            const int halfWidth = width / 2;
            std::uniform_int_distribution<int> col_dist(0, Town::WIDTH - 1);
            std::uniform_int_distribution<int> meander_chance(0, 99);
            std::uniform_int_distribution<int> horizontal_length(0, 2);

            // Tracker for all tiles changed to water to perform autotiling at the end
            std::unordered_set<glm::ivec2, utils::PairHash> river_tiles;

            // 1. Generate target column for each acre row
            std::vector<int> column_targets(Town::HEIGHT);
            int current_col = col_dist(rng);
            column_targets[0] = current_col;

            int consecutive_straight = 0;
            const int max_consecutive_straight = 2;

            for (int az = 0; az < Town::HEIGHT - 1; ++az)
            {
                int next_col = current_col;
                bool straight_ok = CheckPathValid(town, az, current_col, current_col);
                bool wants_meander = (meander_chance(rng) < config.riverMeanderChance);
                bool force_meander = (consecutive_straight >= max_consecutive_straight);

                if (!straight_ok || wants_meander || force_meander)
                {
                    bool wants_long_horizontal = (meander_chance(rng) < config.riverHorizontalChance);
                    int target_col_change = wants_long_horizontal ? horizontal_length(rng) : 1;

                    std::vector<int> candidates;
                    // Right move check
                    if (current_col + target_col_change < Town::WIDTH)
                    {
                        bool valid = true;
                        int test_col = current_col;
                        for (int step = 0; step < target_col_change && valid; ++step)
                        {
                            if (!CheckPathValid(town, az, test_col, test_col + 1))
                                valid = false;
                            else
                                test_col++;
                        }
                        if (valid)
                            candidates.push_back(current_col + target_col_change);
                    }
                    // Left move check
                    if (current_col - target_col_change >= 0)
                    {
                        bool valid = true;
                        int test_col = current_col;
                        for (int step = 0; step < target_col_change && valid; ++step)
                        {
                            if (!CheckPathValid(town, az, test_col, test_col - 1))
                                valid = false;
                            else
                                test_col--;
                        }
                        if (valid)
                            candidates.push_back(current_col - target_col_change);
                    }

                    if (candidates.empty())
                    {
                        if (current_col > 0 && CheckPathValid(town, az, current_col, current_col - 1))
                            candidates.push_back(current_col - 1);
                        if (current_col < Town::WIDTH - 1 && CheckPathValid(town, az, current_col, current_col + 1))
                            candidates.push_back(current_col + 1);
                    }

                    if (!candidates.empty())
                    {
                        std::shuffle(candidates.begin(), candidates.end(), rng);
                        next_col = candidates[0];
                        consecutive_straight = 0;
                    }
                    else
                        consecutive_straight++;
                }
                else
                    consecutive_straight++;

                column_targets[az + 1] = next_col;
                current_col = next_col;
            }

            // 2. Generate river wiggle parameters
            std::uniform_real_distribution<float> amplitude_dist(1.35f, 1.4f);
            std::uniform_real_distribution<float> frequency_dist(0.35f, 0.55f);
            std::uniform_real_distribution<float> phase_dist(0.0f, 6.28318f);

            const float wiggle_amplitude = amplitude_dist(rng);
            const float wiggle_frequency = frequency_dist(rng);
            const float wiggle_phase_x = phase_dist(rng);
            const float wiggle_phase_z = phase_dist(rng);

            const int total_height = Town::HEIGHT * Acre::SIZE;
            std::vector<std::pair<int, int>> river_path;

            for (int z = 0; z < total_height; ++z)
            {
                int base_x = GetBaseRiverX(z, column_targets);
                int rounded_x = ApplyCornerRounding(z, base_x, column_targets);
                int wiggle_x = CalculateWiggle(z, wiggle_amplitude, wiggle_frequency, wiggle_phase_x);

                int local_z = z % Acre::SIZE;
                int curr_acre = z / Acre::SIZE;
                bool is_horizontal_section = false;

                if (curr_acre < Town::HEIGHT - 1 && local_z == TownConfig::RIVER_CONNECTION_POINT_OFFSET)
                {
                    if (column_targets[curr_acre] != column_targets[curr_acre + 1])
                        is_horizontal_section = true;
                }

                int final_z = z;
                if (is_horizontal_section)
                {
                    int wiggle_z = CalculateWiggle(rounded_x, wiggle_amplitude, wiggle_frequency, wiggle_phase_z);
                    final_z = z + wiggle_z;
                }
                river_path.push_back({rounded_x + wiggle_x, final_z});
            }

            // 3. Carve the river and track painted tiles
            for (size_t i = 0; i < river_path.size(); ++i)
            {
                auto [x, z] = river_path[i];

                if (i > 0)
                {
                    auto [prev_x, prev_z] = river_path[i - 1];

                    // Fill X gaps
                    if (prev_x != x && prev_z == z)
                    {
                        int dir = (x > prev_x) ? 1 : -1;
                        for (int fill_x = prev_x; fill_x != x; fill_x += dir)
                            CarveRiverSection(town, fill_x, z, halfWidth, river_tiles);
                    }
                    // Fill Z gaps
                    else if (prev_z != z && prev_x == x)
                    {
                        int dir = (z > prev_z) ? 1 : -1;
                        for (int fill_z = prev_z; fill_z != z; fill_z += dir)
                            CarveRiverSection(town, x, fill_z, halfWidth, river_tiles);
                    }
                    // Fill diagonal gaps (Bresenham)
                    else if (prev_x != x && prev_z != z)
                    {
                        int dx = std::abs(x - prev_x), dz = std::abs(z - prev_z);
                        int sx = (x > prev_x) ? 1 : -1, sz = (z > prev_z) ? 1 : -1;
                        int err = dx - dz;
                        int curr_x = prev_x, curr_z = prev_z;
                        while (curr_x != x || curr_z != z)
                        {
                            CarveRiverSection(town, curr_x, curr_z, halfWidth, river_tiles);
                            int e2 = 2 * err;
                            if (e2 > -dz)
                            {
                                err -= dz;
                                curr_x += sx;
                            }
                            if (e2 < dx)
                            {
                                err += dx;
                                curr_z += sz;
                            }
                        }
                    }
                }
                CarveRiverSection(town, x, z, halfWidth, river_tiles);
            }

            // 4. Create River Mouths (updates river_tiles types to RIVER_MOUTH and handles sand/grass cleanup)
            CreateRiverMouths(town, rng);

            // 5. Autotile Pass
            // We iterate over the tracked tiles to set the correct bitmask indices
            for (const auto &pos : river_tiles)
            {
                auto [a, l] = utils::GetTileCoords(pos.x, pos.y);
                Tile &tile = town.GetAcre(a.x, a.y).tiles[l.y][l.x];

                // Only autotile water types (River, Mouth, etc.)
                if (utils::IsAnyWater(tile.type))
                {
                    // Note: Ensure CalculatePondBlobIndex checks for IsAnyWater neighbors
                    tile.autotileIndex = utils::CalculatePondBlobIndex(town, pos.x, pos.y);
                }
            }
        }
    }
}
#include "RiverGenerationStep.h"

#include "world/Town.h"
#include "world/data/Acre.h"
#include "world/data/Tile.h"
#include "world/data/TownConfig.h"
#include "world/generation/utils/WorldGenUtils.h"

#include <vector>
#include <algorithm>
#include <random>
#include <glm/glm.hpp>

namespace cozy::world
{
    namespace rivers
    {
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
            int half_width)
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
                        auto [a, l] = town.WorldToTile({static_cast<float>(wx),
                                                        0.f,
                                                        static_cast<float>(wz)});
                        Tile &tile = town.GetAcre(a.x, a.y).tiles[l.y][l.x];

                        // Decide per-tile: if it's sand or ocean, make it a mouth, else river
                        const TileType t = tile.type;
                        const bool is_river_mouth =
                            (t == TileType::SAND || t == TileType::OCEAN);

                        tile.type = is_river_mouth ? TileType::RIVER_MOUTH : TileType::RIVER;
                    }
                }
            }
        }

        void CreateRiverMouths(Town &town, std::mt19937_64 &rng)
        {
            const int total_w = Town::WIDTH * Acre::SIZE;
            const int total_h = Town::HEIGHT * Acre::SIZE;
            const int ocean_acre_row = Town::HEIGHT - 1;

            // Track the river's X-coordinates at the mouth to find the center
            std::vector<int> mouth_x_coords;
            int mouth_z = -1;

            // 1. Detect where the river first hits the beach/ocean
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
                auto [min_x_it, max_x_it] = std::minmax_element(mouth_x_coords.begin(), mouth_x_coords.end());
                int river_min_x = *min_x_it;
                int river_max_x = *max_x_it;

                // Dimensions for refined, rounded "ears"
                int drop_width = 5;
                int drop_depth = 8;
                float curve = 1.0f;

                int left_drop_center = river_min_x - (drop_width / 2);
                int right_drop_center = river_max_x + (drop_width / 2);

                // Start higher up to "tuck" the base under the mainland grass seamlessly
                int start_z_local = (mouth_z % Acre::SIZE) - 4;

                // Left Bank Inlet
                utils::CreateGrassTeardrop(town, ocean_acre_row, left_drop_center,
                                           start_z_local, drop_width, drop_depth, -curve, total_w);

                // Right Bank Inlet
                utils::CreateGrassTeardrop(town, ocean_acre_row, right_drop_center,
                                           start_z_local, drop_width, drop_depth, curve, total_w);

                // 3. Cleanup Pass: Prevent sand/river contact and isolated sand pockets
                for (int z = 0; z < total_h; ++z)
                {
                    for (int x = 0; x < total_w; ++x)
                    {
                        auto [a, l] = utils::GetTileCoords(x, z);
                        Tile &tile = town.GetAcre(a.x, a.y).tiles[l.y][l.x];

                        // Remove Sand touching the River/Mouth
                        if (tile.type == TileType::RIVER || tile.type == TileType::RIVER_MOUTH)
                        {
                            const int n_off[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
                            for (auto &off : n_off)
                            {
                                int nx = x + off[0], nz = z + off[1];
                                if (nx < 0 || nx >= total_w || nz < 0 || nz >= total_h)
                                    continue;

                                auto [na, nl] = utils::GetTileCoords(nx, nz);
                                Tile &nTile = town.GetAcre(na.x, na.y).tiles[nl.y][nl.x];
                                if (nTile.type == TileType::SAND)
                                {
                                    nTile.type = TileType::GRASS;
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
            std::uniform_int_distribution<int> horizontal_length(1, 3);

            // Generate target column for each acre row (Z direction)
            // River flows through ALL acres including the ocean row
            std::vector<int> column_targets(Town::HEIGHT);
            int current_col = col_dist(rng);
            column_targets[0] = current_col;

            int consecutive_straight = 0;
            const int max_consecutive_straight = 2;

            // Generate river path through all acres (including ocean acre)
            for (int az = 0; az < Town::HEIGHT - 1; ++az)
            {
                int next_col = current_col;
                bool straight_ok = CheckPathValid(town, az, current_col, current_col);
                bool wants_meander = (meander_chance(rng) < config.riverMeanderChance);

                bool force_meander = (consecutive_straight >= max_consecutive_straight);

                if (!straight_ok || wants_meander || force_meander)
                {
                    bool wants_long_horizontal = (meander_chance(rng) < config.riverHorizontalChance);
                    int target_col_change = 1;

                    if (wants_long_horizontal)
                    {
                        target_col_change = horizontal_length(rng);
                    }

                    std::vector<int> candidates;

                    // Try moving right
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

                    // Try moving left
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
                    else if (!force_meander)
                    {
                        consecutive_straight++;
                    }
                    else
                    {
                        consecutive_straight++;
                    }
                }
                else
                {
                    consecutive_straight++;
                }

                column_targets[az + 1] = next_col;
                current_col = next_col;
            }

            const int total_height = Town::HEIGHT * Acre::SIZE;

            std::vector<int> river_center_x(total_height);
            for (int z = 0; z < total_height; ++z)
            {
                int curr_acre = z / Acre::SIZE;
                int next_acre = std::min(curr_acre + 1, Town::HEIGHT - 1);
                int local_z = z % Acre::SIZE;
                int target_col = (local_z < TownConfig::RIVER_CONNECTION_POINT_OFFSET)
                                     ? column_targets[curr_acre]
                                     : column_targets[next_acre];
                river_center_x[z] = target_col * Acre::SIZE + TownConfig::RIVER_CONNECTION_POINT_OFFSET;
            }

            // Carve the river through ALL acres (including into ocean)
            for (int z = 0; z < total_height; ++z)
            {
                int center_x = river_center_x[z];
                int center_z = z;

                if (z > 0 && (z % Acre::SIZE) == TownConfig::RIVER_CONNECTION_POINT_OFFSET)
                {
                    int prev_x = river_center_x[z - 1];
                    if (prev_x != center_x)
                    {
                        int dir = (center_x > prev_x) ? 1 : -1;
                        for (int x = prev_x; x != center_x + dir; x += dir)
                        {
                            CarveRiverSection(town, x, z, halfWidth);
                        }
                        continue;
                    }
                }

                CarveRiverSection(town, center_x, center_z, halfWidth);
            }
            CreateRiverMouths(town, rng);
        }
    }
}

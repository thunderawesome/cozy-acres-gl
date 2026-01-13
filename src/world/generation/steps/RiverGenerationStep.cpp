// world/generation/steps/RiverGenerationStep.cpp
#include "RiverGenerationStep.h"

#include "world/Town.h"
#include "world/data/Acre.h"
#include "world/data/Tile.h"
#include "world/data/TownConfig.h"

#include <vector>
#include <algorithm>
#include <random>
#include <glm/glm.hpp>

namespace cozy::world
{
    namespace rivers
    {

        namespace
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
                int half_width,
                bool is_river_mouth = false)
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
                            auto [a, l] = town.WorldToTile({static_cast<float>(wx), 0.f, static_cast<float>(wz)});
                            Tile &tile = town.GetAcre(a.x, a.y).tiles[l.y][l.x];

                            // Use RIVER_MOUTH for transition, otherwise WATER
                            tile.type = is_river_mouth ? TileType::RIVER_MOUTH : TileType::RIVER;
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

            // Calculate where sand and ocean begin
            const int ocean_acre_start_z = (Town::HEIGHT - 1) * Acre::SIZE;
            const int sand_start_z = ocean_acre_start_z + 10;  // Row 10 of bottom acre
            const int ocean_start_z = ocean_acre_start_z + 13; // Row 13 of bottom acre

            // Carve the river through ALL acres (including into ocean)
            for (int z = 0; z < total_height; ++z)
            {
                int center_x = river_center_x[z];
                int center_z = z;

                // Determine if this should be river mouth (only in sand/ocean area)
                bool is_river_mouth = (z > sand_start_z);

                if (z > 0 && (z % Acre::SIZE) == TownConfig::RIVER_CONNECTION_POINT_OFFSET)
                {
                    int prev_x = river_center_x[z - 1];
                    if (prev_x != center_x)
                    {
                        int dir = (center_x > prev_x) ? 1 : -1;
                        for (int x = prev_x; x != center_x + dir; x += dir)
                        {
                            CarveRiverSection(town, x, z, halfWidth, is_river_mouth);
                        }
                        continue;
                    }
                }
                CarveRiverSection(town, center_x, center_z, halfWidth, is_river_mouth);
            }
        }
    }
}
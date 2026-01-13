// world/generation/steps/OceanGenerationStep.cpp
#include "OceanGenerationStep.h"

#include "world/Town.h"
#include "world/data/Acre.h"
#include "world/data/Tile.h"
#include "world/data/TownConfig.h"

#include <random>

namespace cozy::world
{
    namespace ocean
    {
        void Execute(
            Town &town,
            std::mt19937_64 &rng,
            const TownConfig &config)
        {
            // In Animal Crossing (GameCube), the F row (bottom acre row) has:
            // - Top 10 rows: normal terrain (can have cliffs, etc.)
            // - Rows 10-12 (3 rows): SAND (beach)
            // - Rows 13-15 (3 rows): OCEAN (water)

            const int ocean_acre_row = Town::HEIGHT - 1;
            const int sand_start_row = 10;  // Local Z within the acre
            const int ocean_start_row = 13; // Local Z within the acre

            for (int acre_x = 0; acre_x < Town::WIDTH; ++acre_x)
            {
                Acre &acre = town.GetAcre(acre_x, ocean_acre_row);

                for (int local_z = 0; local_z < Acre::SIZE; ++local_z)
                {
                    for (int local_x = 0; local_x < Acre::SIZE; ++local_x)
                    {
                        if (local_z >= ocean_start_row)
                        {
                            // Bottom 3 rows: Ocean
                            acre.tiles[local_z][local_x].type = TileType::OCEAN;
                            acre.tiles[local_z][local_x].elevation = 0;
                        }
                        else if (local_z >= sand_start_row)
                        {
                            // Middle 3 rows: Sand/Beach
                            acre.tiles[local_z][local_x].type = TileType::SAND;
                            acre.tiles[local_z][local_x].elevation = 0;
                        }
                        // else: top 10 rows remain as-is (will be set by other generation steps)
                    }
                }
            }
        }
    }
}
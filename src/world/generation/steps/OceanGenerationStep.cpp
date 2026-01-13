#include "OceanGenerationStep.h"

#include "world/Town.h"
#include "world/data/Acre.h"
#include "world/data/Tile.h"
#include "world/data/TownConfig.h"
#include "world/generation/utils/WorldGenUtils.h"

#include <random>
#include <cmath>
#include <vector>

namespace cozy::world
{
    namespace ocean
    {
        void Execute(
            Town &town,
            std::mt19937_64 &rng,
            const TownConfig &config)
        {
            const int ocean_acre_row = Town::HEIGHT - 1;
            const int total_width = Town::WIDTH * Acre::SIZE;

            // 1. Sine Wave Parameters
            std::uniform_real_distribution<float> phase_dist(0.0f, 6.28318f);
            std::uniform_real_distribution<float> freq_dist(0.1f, 0.2f);
            std::uniform_int_distribution<int> amplitude_dist(1, 3);

            float beach_phase = phase_dist(rng);
            float beach_freq = freq_dist(rng);
            int beach_amp = amplitude_dist(rng);

            std::vector<int> sand_boundary(total_width);

            // 2. Calculate wavy beach boundary
            for (int x = 0; x < total_width; ++x)
            {
                float wave = std::sin(x * beach_freq + beach_phase) * beach_amp;
                sand_boundary[x] = 10 + static_cast<int>(std::round(wave));
                sand_boundary[x] = std::clamp(sand_boundary[x], 7, 12);
            }

            // 3. Fill with sand and ocean (Base Layer)
            for (int acre_x = 0; acre_x < Town::WIDTH; ++acre_x)
            {
                Acre &acre = town.GetAcre(acre_x, ocean_acre_row);
                for (int local_z = 0; local_z < Acre::SIZE; ++local_z)
                {
                    for (int local_x = 0; local_x < Acre::SIZE; ++local_x)
                    {
                        int world_x = acre_x * Acre::SIZE + local_x;
                        int sand_start = sand_boundary[world_x];
                        int ocean_start = sand_start + 3;

                        if (local_z >= ocean_start)
                        {
                            acre.tiles[local_z][local_x].type = TileType::OCEAN;
                            acre.tiles[local_z][local_x].elevation = 0;
                        }
                        else if (local_z >= sand_start)
                        {
                            acre.tiles[local_z][local_x].type = TileType::SAND;
                            acre.tiles[local_z][local_x].elevation = 0;
                        }
                    }
                }
            }

            // 4. Draw refined grass "Inlet" teardrops
            // Using 1-3 blobs creates variety without cluttering the beach
            std::uniform_int_distribution<int> blob_count_dist(1, 1);
            std::uniform_int_distribution<int> x_dist(15, total_width - 15);

            // Match River Mouth "Refined" dimensions
            std::uniform_int_distribution<int> width_dist(3, 4);
            std::uniform_int_distribution<int> depth_dist(4, 6);
            float curve = 1.0f;

            int num_blobs = blob_count_dist(rng);
            for (int i = 0; i < num_blobs; ++i)
            {
                int random_x = x_dist(rng);

                // Start at z=8 or 9 (near the beach boundary) so they jut out
                // just enough to break the sand line without being massive.
                int random_start_z = sand_boundary[random_x] - 2;

                utils::CreateGrassTeardrop(
                    town,
                    ocean_acre_row,
                    random_x,
                    random_start_z,
                    width_dist(rng),
                    depth_dist(rng),
                    curve,
                    total_width);
            }
        }
    }
}
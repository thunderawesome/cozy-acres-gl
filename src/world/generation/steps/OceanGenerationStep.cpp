#include "OceanGenerationStep.h"

#include "world/Town.h"
#include "world/data/Acre.h"
#include "world/data/Tile.h"
#include "world/data/TownConfig.h"
#include "world/generation/utils/WorldGenUtils.h"

#include <random>
#include <cmath>
#include <vector>
#include <algorithm> // for std::clamp

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
                int base = 10 + static_cast<int>(std::round(wave));
                sand_boundary[x] = std::clamp(base, 7, 12);
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

                        Tile &tile = acre.tiles[local_z][local_x];

                        if (local_z >= ocean_start)
                        {
                            tile.type = TileType::OCEAN;
                            tile.elevation = 0;
                        }
                        else if (local_z >= sand_start)
                        {
                            tile.type = TileType::SAND;
                            tile.elevation = 0;
                        }
                        // else: keep higher-elevation terrain
                    }
                }
            }

            // 4. Find river-mouth acres on the ocean row
            std::vector<bool> acre_has_mouth(Town::WIDTH, false);

            for (int acre_x = 0; acre_x < Town::WIDTH; ++acre_x)
            {
                Acre &acre = town.GetAcre(acre_x, ocean_acre_row);
                bool found = false;

                for (int local_z = 0; local_z < Acre::SIZE && !found; ++local_z)
                {
                    for (int local_x = 0; local_x < Acre::SIZE && !found; ++local_x)
                    {
                        if (acre.tiles[local_z][local_x].type == TileType::RIVER_MOUTH)
                        {
                            acre_has_mouth[acre_x] = true;
                            found = true;
                        }
                    }
                }
            }

            // 5. Build candidate acres for the blob (no mouth, not near world bounds)
            std::vector<int> candidate_acres;
            const int min_acre_x = 1;               // avoid very left edge
            const int max_acre_x = Town::WIDTH - 2; // avoid very right edge

            for (int acre_x = min_acre_x; acre_x <= max_acre_x; ++acre_x)
            {
                if (!acre_has_mouth[acre_x])
                {
                    candidate_acres.push_back(acre_x);
                }
            }

            if (!candidate_acres.empty())
            {
                // 6. Choose one acre at random
                std::uniform_int_distribution<int> cand_dist(0, (int)candidate_acres.size() - 1);
                int chosen_acre_x = candidate_acres[cand_dist(rng)];

                // World-space X center: middle of chosen acre
                int center_x = chosen_acre_x * Acre::SIZE + Acre::SIZE / 2;

                // Clamp X center further away from world bounds if you want extra safety
                const int margin_tiles = 4; // do not let center be too close to world edges
                center_x = std::clamp(center_x, margin_tiles, total_width - 1 - margin_tiles);

                // Place the blob around the sand/grass boundary at this column
                int sand_here = sand_boundary[center_x];
                sand_here = std::clamp(sand_here, 8, 11);

                int start_z_local = sand_here - 2;

                // Rounded, autotile-friendly blob: width ≈ depth, slight bend
                std::uniform_int_distribution<int> width_dist(7, 8); // diameters
                std::uniform_int_distribution<int> depth_dist(6, 8);
                std::uniform_int_distribution<int> curve_sign_dist(0, 1); // 0 = negative, 1 = positive

                const int blob_width = width_dist(rng);
                const int blob_depth = depth_dist(rng);

                // Magnitude of curve (tweak 0.5f–1.5f to taste)
                const float curve_magnitude = 1.0f;

                // Randomly choose left or right bend
                const float blob_curve = curve_sign_dist(rng) == 0 ? -curve_magnitude : curve_magnitude;

                utils::CreateGrassTeardrop(
                    town,
                    ocean_acre_row,
                    center_x,
                    start_z_local,
                    blob_width,
                    blob_depth,
                    blob_curve,
                    total_width);
            }
        }

    }
}

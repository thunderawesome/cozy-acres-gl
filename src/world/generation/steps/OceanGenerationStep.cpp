#include "OceanGenerationStep.h"

#include "world/Town.h"
#include "world/data/Acre.h"
#include "world/data/Tile.h"
#include "world/data/TownConfig.h"
#include "world/generation/utils/WorldGenUtils.h"

#include <random>
#include <cmath>
#include <vector>
#include <algorithm>

namespace cozy::world::ocean
{
    void Execute(
        Town &town,
        std::mt19937_64 &rng,
        const TownConfig &config)
    {
        const int ocean_acre_row = Town::BEACH_ACRE_ROW;
        const int total_width = Town::WIDTH * Acre::SIZE;

        // Beach only applies to Acre F
        const int max_beach_acre = 5;

        // 1. Sine Wave Parameters from Config
        std::uniform_real_distribution<float> phase_dist(0.0f, 6.28318f);
        std::uniform_real_distribution<float> freq_dist(config.beachFreqMin, config.beachFreqMax);
        std::uniform_int_distribution<int> amplitude_dist(config.beachAmplitudeMin, config.beachAmplitudeMax);

        float beach_phase = phase_dist(rng);
        float beach_freq = freq_dist(rng);
        int beach_amp = amplitude_dist(rng);

        std::vector<int> sand_boundary(total_width);

        // 2. Calculate wavy beach boundary
        for (int x = 0; x < total_width; ++x)
        {
            float wave = std::sin(x * beach_freq + beach_phase) * beach_amp;
            int base = config.beachBaseDepth + static_cast<int>(std::round(wave));
            sand_boundary[x] = std::clamp(base, 7, 12);
        }

        // 3. Fill beach/ocean ONLY for acres F
        for (int acre_x = 0; acre_x <= max_beach_acre && acre_x < Town::WIDTH; ++acre_x)
        {
            Acre &acre = town.GetAcre(acre_x, ocean_acre_row);

            for (int local_z = 0; local_z < Acre::SIZE; ++local_z)
            {
                for (int local_x = 0; local_x < Acre::SIZE; ++local_x)
                {
                    int world_x = acre_x * Acre::SIZE + local_x;
                    int sand_start = sand_boundary[world_x];
                    int ocean_start = sand_start + config.beachSandToOceanBuffer;

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
                }
            }
        }

        // 3.5. Vertically fill pure ocean (Acre G and beyond)
        const int first_ocean_acre_z = 6;
        for (int acre_z = first_ocean_acre_z; acre_z < Town::HEIGHT; ++acre_z)
        {
            for (int acre_x = 0; acre_x < Town::WIDTH; ++acre_x)
            {
                Acre &acre = town.GetAcre(acre_x, acre_z);
                for (int local_z = 0; local_z < Acre::SIZE; ++local_z)
                {
                    for (int local_x = 0; local_x < Acre::SIZE; ++local_x)
                    {
                        acre.tiles[local_z][local_x].type = TileType::OCEAN;
                        acre.tiles[local_z][local_x].elevation = 0;
                    }
                }
            }
        }

        // 4. Find river-mouth acres (used to avoid placing the grass blob on top of a river)
        std::vector<bool> acre_has_mouth(Town::WIDTH, false);
        for (int acre_x = 0; acre_x <= max_beach_acre && acre_x < Town::WIDTH; ++acre_x)
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

        // 5. Candidate acres for the grass blob
        std::vector<int> candidate_acres;
        const int min_acre_x = 1;
        const int max_blob_acre = std::min(max_beach_acre, Town::WIDTH - 2);

        for (int acre_x = min_acre_x; acre_x <= max_blob_acre; ++acre_x)
        {
            if (!acre_has_mouth[acre_x])
            {
                candidate_acres.push_back(acre_x);
            }
        }

        if (!candidate_acres.empty())
        {
            std::uniform_int_distribution<int> cand_dist(0, (int)candidate_acres.size() - 1);
            int chosen_acre_x = candidate_acres[cand_dist(rng)];

            int center_x = chosen_acre_x * Acre::SIZE + Acre::SIZE / 2;
            const int margin_tiles = 4;
            center_x = std::clamp(center_x, margin_tiles, total_width - 1 - margin_tiles);

            int sand_here = sand_boundary[center_x];
            sand_here = std::clamp(sand_here, 8, 11);
            int start_z_local = sand_here - 2;

            // Use Config for blob dimensions
            std::uniform_int_distribution<int> size_dist(config.grassBlobSizeMin, config.grassBlobSizeMax);
            std::uniform_int_distribution<int> curve_sign_dist(0, 1);

            const int blob_width = size_dist(rng);
            const int blob_depth = size_dist(rng);
            const float blob_curve = (curve_sign_dist(rng) == 0) ? -config.grassBlobCurveMagnitude : config.grassBlobCurveMagnitude;

            utils::CreateGrassTeardrop(
                town,
                ocean_acre_row,
                center_x,
                start_z_local,
                blob_width,
                blob_depth,
                blob_curve, total_width);
        }
    }
}
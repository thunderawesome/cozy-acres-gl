#include "world/generation/utils/WorldGenUtils.h"

namespace cozy::world::utils
{
    void CreateGrassTeardrop(
        Town &town,
        int ocean_acre_row,
        int center_x,
        int start_z,
        int max_width,
        int depth,
        float curve_amount,
        int total_width)
    {
        for (int dz = 0; dz <= depth; ++dz)
        {
            int local_z = start_z + dz;

            if (local_z < 0 || local_z >= Acre::SIZE)
                continue;

            float t = static_cast<float>(dz) / depth;
            float width_factor = (t < 0.4f) ? 1.0f : 1.0f - std::pow((t - 0.4f) / 0.6f, 2);

            int half_width = static_cast<int>(max_width * width_factor);
            int curve_offset = static_cast<int>(curve_amount * t * t);

            for (int dx = -half_width; dx <= half_width; ++dx)
            {
                int wx = center_x + dx + curve_offset;

                if (wx >= 0 && wx < total_width)
                {
                    int acre_x = wx / Acre::SIZE;
                    int local_x = wx % Acre::SIZE;

                    Acre &acre = town.GetAcre(acre_x, ocean_acre_row);
                    Tile &tile = acre.tiles[local_z][local_x];

                    if (half_width > 0)
                    {
                        float dist = std::abs(static_cast<float>(dx) / (half_width + 0.5f));

                        if (dist <= 1.0f)
                        {
                            // 1. Only change the type to GRASS if it's NOT water.
                            // This allows the "teardrop" to exist "under" the river.
                            if (tile.type != TileType::RIVER && tile.type != TileType::RIVER_MOUTH)
                            {
                                tile.type = TileType::GRASS;
                            }

                            // 2. Always set elevation to 0.
                            // This ensures the river bed and the grass banks are
                            // at the same height, preventing weird cliff artifacts.
                            tile.elevation = 0;
                        }
                    }
                }
            }
        }
    }
}
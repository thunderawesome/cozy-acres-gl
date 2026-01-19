#include "world/generation/utils/WorldGenUtils.h"

namespace cozy::world::utils
{
    void CreateGrassTeardrop(
        Town &town,
        int ocean_acre_row,
        int center_x,    // world tile X
        int start_z,     // local Z in ocean acre where blob starts (top of blob)
        int max_width,   // X diameter in tiles
        int depth,       // Z diameter in tiles
        float curve,     // negative = bend left, positive = bend right, 0 = symmetric
        int total_width) // world width in tiles
    {
        const int world_h = Town::HEIGHT * Acre::SIZE;

        // Semi-axes (in tiles)
        const float rx = max_width * 0.5f;
        const float rz = depth * 0.5f;

        // World-space Z center
        const int acre_z0 = ocean_acre_row * Acre::SIZE;
        const float center_z = static_cast<float>(acre_z0 + start_z) + rz; // middle of the blob

        // Track world tiles this call changed, plus their original type
        struct PaintedTile
        {
            glm::ivec2 pos;
            TileType original_type;
        };

        std::vector<PaintedTile> painted;
        painted.reserve(max_width * depth);

        // Scan a tight bounding box
        const int z_min = std::max(0, static_cast<int>(std::floor(center_z - rz - 1)));
        const int z_max = std::min(world_h - 1, static_cast<int>(std::ceil(center_z + rz + 1)));

        for (int wz = z_min; wz <= z_max; ++wz)
        {
            const float dz = static_cast<float>(wz) - center_z;
            const float nz = dz / rz;
            if (nz * nz > 1.0f)
                continue;

            // Optional “teardrop” bend: slide X center a little over Z
            const float bend = curve * (dz / static_cast<float>(depth)); // tiny offset
            const float cx = static_cast<float>(center_x) + bend;

            const int x_min = std::max(0, static_cast<int>(std::floor(cx - rx - 1)));
            const int x_max = std::min(total_width - 1, static_cast<int>(std::ceil(cx + rx + 1)));

            for (int wx = x_min; wx <= x_max; ++wx)
            {
                const float dx = static_cast<float>(wx) - cx;
                const float nx = dx / rx;

                // Inside ellipse?
                if (nx * nx + nz * nz > 1.0f)
                    continue;

                auto [a, l] = utils::GetTileCoords(wx, wz);
                Tile &tile = town.GetAcre(a.x, a.y).tiles[l.y][l.x];

                // Only paint over beach/ocean; do not touch river/grass/other
                if (tile.type == TileType::SAND || tile.type == TileType::OCEAN)
                {
                    PaintedTile pt;
                    pt.pos = {wx, wz};
                    pt.original_type = tile.type;

                    tile.type = TileType::GRASS;
                    tile.elevation = 0;

                    painted.push_back(pt);
                }
            }
        }

        // Connectivity cleanup: remove isolated or near-isolated GRASS tips
        const int n_off[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};

        for (const PaintedTile &pt : painted)
        {
            int wx = pt.pos.x;
            int wz = pt.pos.y;

            int grass_neighbors = 0;

            for (int i = 0; i < 4; ++i)
            {
                int nx = wx + n_off[i][0];
                int nz = wz + n_off[i][1];

                if (nx < 0 || nx >= total_width || nz < 0 || nz >= world_h)
                    continue;

                auto [na, nl] = utils::GetTileCoords(nx, nz);
                Tile &nt = town.GetAcre(na.x, na.y).tiles[nl.y][nl.x];

                if (nt.type == TileType::GRASS)
                    ++grass_neighbors;
            }

            // If this GRASS tile has 0 or 1 GRASS neighbors, it is a spike: revert it
            if (grass_neighbors <= 1)
            {
                auto [a, l] = utils::GetTileCoords(wx, wz);
                Tile &tile = town.GetAcre(a.x, a.y).tiles[l.y][l.x];

                // Only revert if still GRASS (in case something else changed it later)
                if (tile.type == TileType::GRASS)
                {
                    tile.type = pt.original_type;
                    tile.elevation = 0;
                }
            }
        }
    }
}

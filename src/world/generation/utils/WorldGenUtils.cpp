#include "world/generation/utils/WorldGenUtils.h"
#include <algorithm>

namespace cozy::world::utils
{
    std::vector<glm::ivec2> GetNeighbors4(int wx, int wz)
    {
        const int offsets[4][2] = {{1, 0}, {-1, 0}, {0, 1}, {0, -1}};
        std::vector<glm::ivec2> neighbors;
        neighbors.reserve(4);
        for (auto &off : offsets)
        {
            int nx = wx + off[0];
            int nz = wz + off[1];
            if (IsInBounds(nx, nz))
                neighbors.push_back({nx, nz});
        }
        return neighbors;
    }

    std::vector<glm::ivec2> GetNeighbors8(int wx, int wz)
    {
        std::vector<glm::ivec2> neighbors;
        neighbors.reserve(8);
        for (int dz = -1; dz <= 1; ++dz)
        {
            for (int dx = -1; dx <= 1; ++dx)
            {
                if (dx == 0 && dz == 0)
                    continue;
                int nx = wx + dx;
                int nz = wz + dz;
                if (IsInBounds(nx, nz))
                    neighbors.push_back({nx, nz});
            }
        }
        return neighbors;
    }

    float SmoothStep(float t)
    {
        return t * t * (3.0f - 2.0f * t);
    }

    float Noise2D(int x, int z, int seed)
    {
        int n = x + z * 57 + seed * 131;
        n = (n << 13) ^ n;
        int nn = (n * (n * n * 60493 + 19990303) + 1376312589) & 0x7fffffff;
        return 1.0f - (static_cast<float>(nn) / 1073741824.0f);
    }

    float SmoothNoise(float x, float z, int seed)
    {
        int ix = static_cast<int>(std::floor(x));
        int iz = static_cast<int>(std::floor(z));
        float fx = x - ix;
        float fz = z - iz;

        fx = SmoothStep(fx);
        fz = SmoothStep(fz);

        float v00 = Noise2D(ix, iz, seed);
        float v10 = Noise2D(ix + 1, iz, seed);
        float v01 = Noise2D(ix, iz + 1, seed);
        float v11 = Noise2D(ix + 1, iz + 1, seed);

        float v0 = v00 * (1.0f - fx) + v10 * fx;
        float v1 = v01 * (1.0f - fz) + v11 * fz; // Corrected lerp logic
        return v0 * (1.0f - fz) + v1 * fz;
    }

    void CreateGrassTeardrop(
        Town &town,
        int ocean_acre_row,
        int center_x,
        int start_z,
        int max_width,
        int depth,
        float curve,
        int total_width)
    {
        const int world_h = GetWorldHeight();
        const float rx = max_width * 0.5f;
        const float rz = depth * 0.5f;
        const int acre_z0 = ocean_acre_row * Acre::SIZE;
        const float center_z = static_cast<float>(acre_z0 + start_z) + rz;

        struct PaintedTile
        {
            glm::ivec2 pos;
            TileType original_type;
        };
        std::vector<PaintedTile> painted;

        const int z_min = std::max(0, static_cast<int>(std::floor(center_z - rz - 1)));
        const int z_max = std::min(world_h - 1, static_cast<int>(std::ceil(center_z + rz + 1)));

        for (int wz = z_min; wz <= z_max; ++wz)
        {
            const float dz = static_cast<float>(wz) - center_z;
            const float nz = dz / rz;
            if (nz * nz > 1.0f)
                continue;

            const float bend = curve * (dz / static_cast<float>(depth));
            const float cx = static_cast<float>(center_x) + bend;

            const int x_min = std::max(0, static_cast<int>(std::floor(cx - rx - 1)));
            const int x_max = std::min(total_width - 1, static_cast<int>(std::ceil(cx + rx + 1)));

            for (int wx = x_min; wx <= x_max; ++wx)
            {
                const float dx = static_cast<float>(wx) - cx;
                const float nx = dx / rx;
                if (nx * nx + nz * nz > 1.0f)
                    continue;

                Tile *tile = GetTileSafe(town, wx, wz);
                if (tile && (tile->type == TileType::SAND || tile->type == TileType::OCEAN))
                {
                    painted.push_back({{wx, wz}, tile->type});
                    tile->type = TileType::GRASS;
                    tile->elevation = 0;
                }
            }
        }

        // Cleanup isolated grass using the new neighbor helper
        for (const auto &pt : painted)
        {
            int grass_neighbors = 0;
            for (auto &neighborPos : GetNeighbors4(pt.pos.x, pt.pos.y))
            {
                if (GetTileTypeSafe(town, neighborPos.x, neighborPos.y) == TileType::GRASS)
                    grass_neighbors++;
            }

            if (grass_neighbors <= 1)
            {
                Tile *tile = GetTileSafe(town, pt.pos.x, pt.pos.y);
                if (tile && tile->type == TileType::GRASS)
                {
                    tile->type = pt.original_type;
                }
            }
        }
    }
}
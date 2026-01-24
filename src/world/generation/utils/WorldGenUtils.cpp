#include "WorldGenUtils.h"
#include "world/Town.h"
#include <cmath>
#include <algorithm>

namespace cozy::world::utils
{
    int GetWorldWidth() { return Town::WIDTH * Acre::SIZE; }
    int GetWorldHeight() { return Town::HEIGHT * Acre::SIZE; }

    int GetElevation(const Town &town, int x, int z)
    {
        if (!IsInBounds(x, z))
            return -1;
        auto [a, l] = WorldToTile(x, z);
        return town.GetAcre(a.x, a.y).tiles[l.y][l.x].elevation;
    }

    TileType GetTileTypeSafe(const Town &town, int wx, int wz)
    {
        if (!IsInBounds(wx, wz))
            return TileType::EMPTY;
        auto [a, l] = WorldToTile(wx, wz);
        return town.GetAcre(a.x, a.y).tiles[l.y][l.x].type;
    }

    Tile *GetTileSafe(Town &town, int wx, int wz)
    {
        if (!IsInBounds(wx, wz))
            return nullptr;
        auto [a, l] = WorldToTile(wx, wz);
        return &town.GetAcre(a.x, a.y).tiles[l.y][l.x];
    }

    std::vector<glm::ivec2> GetNeighbors4(int wx, int wz)
    {
        return {{wx + 1, wz}, {wx - 1, wz}, {wx, wz + 1}, {wx, wz - 1}};
    }

    std::vector<glm::ivec2> GetNeighbors8(int wx, int wz)
    {
        std::vector<glm::ivec2> n;
        for (int dz = -1; dz <= 1; ++dz)
            for (int dx = -1; dx <= 1; ++dx)
                if (dx != 0 || dz != 0)
                    n.push_back({wx + dx, wz + dz});
        return n;
    }

    float SmoothStep(float t) { return t * t * (3.0f - 2.0f * t); }

    float Noise2D(int x, int z, int seed)
    {
        int n = x + z * 57 + seed * 131;
        n = (n << 13) ^ n;
        return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7fffffff) / 1073741824.0f);
    }

    float SmoothNoise(float x, float z, int seed)
    {
        int xi = (int)std::floor(x);
        int zi = (int)std::floor(z);
        float xf = x - xi;
        float zf = z - zi;
        float v1 = Noise2D(xi, zi, seed);
        float v2 = Noise2D(xi + 1, zi, seed);
        float v3 = Noise2D(xi, zi + 1, seed);
        float v4 = Noise2D(xi + 1, zi + 1, seed);
        float i1 = v1 + SmoothStep(xf) * (v2 - v1);
        float i2 = v3 + SmoothStep(xf) * (v4 - v3);
        return i1 + SmoothStep(zf) * (i2 - i1);
    }

    void CreateGrassTeardrop(Town &town, int row, int cx, int sz, int mw, int d, float curve, int tw)
    {
        for (int dz = 0; dz < d; ++dz)
        {
            float t = (float)dz / d;
            int width = (int)(mw * (1.0f - t * t));
            int offset = (int)(curve * std::sin(t * 3.14159f));

            for (int dx = -width / 2; dx <= width / 2; ++dx)
            {
                int final_x = cx + dx + offset;

                // Now using 'tw' to skip unnecessary work
                if (final_x < 0 || final_x >= tw)
                    continue;

                if (auto *tile = GetTileSafe(town, final_x, row * Acre::SIZE + sz + dz))
                    tile->type = TileType::GRASS;
            }
        }
    }
}
#pragma once
#include "world/Town.h"
#include "world/data/Tile.h"
#include "world/data/Acre.h"
#include <glm/glm.hpp>
#include <utility> // for std::pair

namespace cozy::world::utils
{
    // Hashing for using glm::ivec2 in unordered_sets
    struct PairHash
    {
        std::size_t operator()(const glm::ivec2 &v) const
        {
            return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1);
        }
    };

    // Math: World X/Z to {Acre Coords, Local Tile Coords}
    inline std::pair<glm::ivec2, glm::ivec2> GetTileCoords(int wx, int wz)
    {
        return {
            {wx / Acre::SIZE, wz / Acre::SIZE},
            {wx % Acre::SIZE, wz % Acre::SIZE}};
    }

    inline TileType GetTileTypeSafe(const Town &town, int wx, int wz)
    {
        const int world_w = Town::WIDTH * Acre::SIZE;
        const int world_h = Town::HEIGHT * Acre::SIZE;

        if (wx < 0 || wx >= world_w || wz < 0 || wz >= world_h)
            return TileType::EMPTY;

        auto [a, l] = GetTileCoords(wx, wz);
        return town.GetAcre(a.x, a.y).tiles[l.y][l.x].type;
    }

    inline bool IsAnyWater(TileType type)
    {
        return type == TileType::RIVER ||
               type == TileType::RIVER_MOUTH ||
               type == TileType::OCEAN ||
               type == TileType::WATERFALL ||
               type == TileType::POND;
    }

    float SmoothStep(float t);
    float Noise2D(int x, int z, int seed);
    float SmoothNoise(float x, float z, int seed);

    void CreateGrassTeardrop(
        Town &town,
        int ocean_acre_row,
        int center_x,
        int start_z,
        int max_width,
        int depth,
        float curve_amount,
        int total_width);
}
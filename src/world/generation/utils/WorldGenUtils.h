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

    inline bool IsAnyWater(TileType type)
    {
        return type == TileType::RIVER ||
               type == TileType::RIVER_MOUTH ||
               type == TileType::OCEAN ||
               type == TileType::POND;
    }

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
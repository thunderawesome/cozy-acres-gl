#pragma once
#include "world/data/Tile.h"
#include "world/data/Acre.h"
#include <glm/glm.hpp>
#include <utility>
#include <vector>
#include <functional>

namespace cozy::world
{
    class Town; // Forward declaration to avoid circular includes
}

namespace cozy::world::utils
{
    // --- Hashing ---
    struct PairHash
    {
        std::size_t operator()(const glm::ivec2 &v) const
        {
            return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1);
        }
    };

    // --- Dimension Helpers ---
    // Note: We access these via a helper to avoid needing Town.h in this header
    int GetWorldWidth();
    int GetWorldHeight();

    inline bool IsInBounds(int wx, int wz)
    {
        return wx >= 0 && wx < GetWorldWidth() && wz >= 0 && wz < GetWorldHeight();
    }

    // --- Coordinate Math (Inline for Speed) ---
    inline std::pair<glm::ivec2, glm::ivec2> WorldToTile(int x, int z)
    {
        return {{x / Acre::SIZE, z / Acre::SIZE}, {x % Acre::SIZE, z % Acre::SIZE}};
    }

    inline std::pair<glm::ivec2, glm::ivec2> GetTileCoords(int wx, int wz)
    {
        return WorldToTile(wx, wz);
    }

    // --- Logic Helpers ---
    inline bool IsAnyWater(TileType type)
    {
        return type == TileType::RIVER || type == TileType::RIVER_MOUTH ||
               type == TileType::OCEAN || type == TileType::WATERFALL ||
               type == TileType::POND;
    }

    // --- Declarations (Implemented in .cpp) ---
    int GetElevation(const Town &town, int x, int z);
    TileType GetTileTypeSafe(const Town &town, int wx, int wz);
    Tile *GetTileSafe(Town &town, int wx, int wz);

    std::vector<glm::ivec2> GetNeighbors4(int wx, int wz);
    std::vector<glm::ivec2> GetNeighbors8(int wx, int wz);

    float SmoothStep(float t);
    float Noise2D(int x, int z, int seed);
    float SmoothNoise(float x, float z, int seed);

    void CreateGrassTeardrop(Town &town, int row, int cx, int sz, int mw, int d, float curve, int tw);
}
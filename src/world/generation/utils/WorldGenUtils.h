#pragma once
#include "world/Town.h"
#include "world/data/Tile.h"
#include "world/data/Acre.h"
#include <glm/glm.hpp>
#include <utility>
#include <vector>

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

    // --- Dimension Helpers ---
    inline int GetWorldWidth() { return Town::WIDTH * Acre::SIZE; }
    inline int GetWorldHeight() { return Town::HEIGHT * Acre::SIZE; }

    inline bool IsInBounds(int wx, int wz)
    {
        return wx >= 0 && wx < GetWorldWidth() && wz >= 0 && wz < GetWorldHeight();
    }

    // --- Coordinate Math ---
    inline std::pair<glm::ivec2, glm::ivec2> GetTileCoords(int wx, int wz)
    {
        return {
            {wx / Acre::SIZE, wz / Acre::SIZE},
            {wx % Acre::SIZE, wz % Acre::SIZE}};
    }

    // --- Safe Access ---
    inline TileType GetTileTypeSafe(const Town &town, int wx, int wz)
    {
        if (!IsInBounds(wx, wz))
            return TileType::EMPTY;
        auto [a, l] = GetTileCoords(wx, wz);
        return town.GetAcre(a.x, a.y).tiles[l.y][l.x].type;
    }

    // Returns pointer to allow modification, or nullptr if out of bounds
    inline Tile *GetTileSafe(Town &town, int wx, int wz)
    {
        if (!IsInBounds(wx, wz))
            return nullptr;
        auto [a, l] = GetTileCoords(wx, wz);
        return &town.GetAcre(a.x, a.y).tiles[l.y][l.x];
    }

    // --- Neighbor Utilities ---
    // Returns 4-way neighbors (N, S, E, W)
    std::vector<glm::ivec2> GetNeighbors4(int wx, int wz);
    // Returns 8-way neighbors
    std::vector<glm::ivec2> GetNeighbors8(int wx, int wz);

    inline bool IsAnyWater(TileType type)
    {
        return type == TileType::RIVER ||
               type == TileType::RIVER_MOUTH ||
               type == TileType::OCEAN ||
               type == TileType::WATERFALL ||
               type == TileType::POND;
    }

    // --- Math & Generation ---
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
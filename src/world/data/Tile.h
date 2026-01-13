#pragma once
#include <cstdint>

namespace cozy::world
{
    enum class TileType : uint8_t
    {
        EMPTY,
        GRASS,
        DIRT,
        SAND,

        // Water types - keeping them grouped for clarity
        RIVER,       // Flowing river water (north-south)
        POND,        // Still pond water
        OCEAN,       // Ocean at southern edge
        RIVER_MOUTH, // Where river meets ocean (delta/estuary)

        TREE,
        ROCK,
        BUILDING,
        CLIFF,
        RAMP
    };

    struct Tile
    {
        TileType type = TileType::EMPTY;
        int8_t elevation = 0; // 0 = base, 1 = mid, 2 = high
    };
}
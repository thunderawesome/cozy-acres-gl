#pragma once

#include <cstdint>

namespace cozy::world
{

    enum class TileType : uint8_t
    {
        EMPTY,
        GRASS,
        DIRT,
        WATER,
        TREE,
        ROCK,
        BUILDING,
        CLIFF
    };

    struct Tile
    {
        TileType type = TileType::EMPTY;
        int8_t elevation = 0; // 0 = base, 1 = mid, 2 = high
    };

}
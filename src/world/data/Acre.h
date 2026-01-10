#pragma once

#include <array>
#include <vector>
#include <unordered_map>
#include <cstdint>
#include "Tile.h"
#include <glm/glm.hpp>

namespace cozy::world
{

    struct ObjectConfig
    {
        glm::ivec2 pos;
        glm::ivec2 size;
        bool blocks_path = true;
    };

    class Acre
    {
    public:
        static constexpr int SIZE = 16;

        std::array<std::array<Tile, SIZE>, SIZE> tiles{};
        std::vector<ObjectConfig> objects;
        std::unordered_map<uint16_t, const ObjectConfig *> object_lookup;

        void RebuildLookup();
    };

}
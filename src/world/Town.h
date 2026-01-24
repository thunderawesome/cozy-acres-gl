#pragma once

#include <array>
#include <vector>
#include <memory>
#include "data/Acre.h"

namespace cozy::world
{
    struct TownConfig;
}
namespace cozy::rendering
{
    struct TileInstance;
}

namespace cozy::world
{
    class Town
    {
    public:
        // Layout Constants
        static constexpr int WIDTH = 5;
        static constexpr int HEIGHT = 7; // Only 6 render on the town map. 7 (or ACRE G) is for pure ocean acres beyond the normal town map
        static constexpr int BEACH_ACRE_ROW = HEIGHT - 2;

        Town();

        // Core Actions
        void Generate(uint64_t seed, const TownConfig &config);
        void Reset();

        // Data Accessors
        Acre &GetAcre(int ax, int az) { return m_acres[ax][az]; }
        const Acre &GetAcre(int ax, int az) const { return m_acres[ax][az]; }

    private:
        std::array<std::array<Acre, HEIGHT>, WIDTH> m_acres;
    };
}
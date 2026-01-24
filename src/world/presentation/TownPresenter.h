#pragma once
#include <vector>
#include "world/Town.h"
#include "rendering/InstanceData.h"

namespace cozy::world
{
    class TownPresenter
    {
    public:
        // Generates the GPU instance data for rendering
        static std::vector<rendering::TileInstance> GenerateRenderData(const Town &town);

        // Prints the ASCII map to the console
        static void DebugDump(const Town &town);

    private:
        static glm::vec3 GetTileColor(const Tile &tile, int y);
    };
}
#include "Town.h"
#include <glm/gtc/matrix_transform.hpp>

namespace cozy::world
{
    void Acre::RebuildLookup()
    {
        object_lookup.clear();
        for (const auto &obj : objects)
        {
            for (int x = 0; x < obj.size.x; ++x)
            {
                for (int y = 0; y < obj.size.y; ++y)
                {
                    uint16_t key = (obj.pos.x + x) | ((obj.pos.y + y) << 8);
                    object_lookup[key] = &obj;
                }
            }
        }
    }

    void Town::Generate(uint64_t seed)
    {
        std::mt19937_64 rng(seed);
        // Initialization: Fill with grass
        for (auto &col : m_Acres)
            for (auto &acre : col)
                for (auto &row : acre.tiles)
                    for (auto &tile : row)
                        tile.type = TileType::GRASS;

        CarveRiver(rng);
        CarvePond(rng);
    }

    std::pair<glm::ivec2, glm::ivec2> Town::WorldToTile(glm::vec3 world_pos) const
    {
        glm::ivec2 acre_idx(
            static_cast<int>(world_pos.x / 16.0f),
            static_cast<int>(world_pos.z / 16.0f));
        glm::ivec2 local_tile(
            static_cast<int>(world_pos.x) % 16,
            static_cast<int>(world_pos.z) % 16);
        return {acre_idx, local_tile};
    }

    std::vector<rendering::TileInstance> Town::GenerateRenderData() const
    {
        std::vector<rendering::TileInstance> instances;
        instances.reserve(WIDTH * HEIGHT * 16 * 16);

        for (int ax = 0; ax < WIDTH; ++ax)
        {
            for (int ay = 0; ay < HEIGHT; ++ay)
            {
                for (int lx = 0; lx < 16; ++lx)
                {
                    for (int ly = 0; ly < 16; ++ly)
                    {
                        const Tile &tile = m_Acres[ax][ay].tiles[ly][lx];

                        rendering::TileInstance inst;

                        // Calculate world position
                        float worldX = static_cast<float>(ax * 16 + lx);
                        float worldZ = static_cast<float>(ay * 16 + ly);

                        glm::mat4 model = glm::translate(glm::mat4(1.0f), glm::vec3(worldX, (float)tile.elevation, worldZ));
                        inst.modelMatrix = model;

                        // Logic-based coloring
                        if (tile.type == TileType::WATER)
                            inst.color = {0.1f, 0.4f, 0.8f};
                        else if (tile.type == TileType::GRASS)
                            inst.color = {0.3f, 0.6f, 0.2f};
                        else
                            inst.color = {0.5f, 0.5f, 0.5f};

                        instances.push_back(inst);
                    }
                }
            }
        }
        return instances;
    }

    void Town::CarveRiver(std::mt19937_64 &rng)
    {
        // Placeholder for your DFS logic
    }

    void Town::CarvePond(std::mt19937_64 &rng)
    {
        // Placeholder for your BFS logic
    }
}
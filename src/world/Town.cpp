#include "Town.h"
#include <glm/gtc/matrix_transform.hpp>
#include <stack> // Required for DFS
#include <queue> // Required for BFS
#include <array>
#include <cmath> // For std::floor

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

    std::pair<glm::ivec2, glm::ivec2> Town::WorldToTile(glm::vec3 world_pos) const
    {
        int x = static_cast<int>(std::floor(world_pos.x));
        int z = static_cast<int>(std::floor(world_pos.z));

        // Clamp to prevent out-of-bounds access
        x = std::max(0, std::min(x, (WIDTH * 16) - 1));
        z = std::max(0, std::min(z, (HEIGHT * 16) - 1));

        glm::ivec2 acre_idx(x / 16, z / 16);
        glm::ivec2 local_tile(x % 16, z % 16);
        return {acre_idx, local_tile};
    }

    void Town::Generate(uint64_t seed)
    {
        std::mt19937_64 rng(seed);

        // Step 1: Initialize with Grass
        for (auto &col : m_Acres)
            for (auto &acre : col)
                for (auto &row : acre.tiles)
                    for (auto &tile : row)
                        tile.type = TileType::GRASS;

        // Step 2: Carve Features
        CarveRiver(rng);
        CarvePond(rng);
    }

    void Town::CarveRiver(std::mt19937_64 &rng)
    {
        std::uniform_int_distribution<int> dist(0, (WIDTH * 16) - 1);
        int currentX = dist(rng);
        int currentZ = 0;

        while (currentZ < (HEIGHT * 16))
        {
            // Thicken the river (3-tile width)
            for (int dx = -1; dx <= 1; ++dx)
            {
                int targetX = currentX + dx;
                if (targetX >= 0 && targetX < WIDTH * 16)
                {
                    auto [acre, local] = WorldToTile(glm::vec3(targetX, 0, currentZ));
                    m_Acres[acre.x][acre.y].tiles[local.y][local.x].type = TileType::WATER;
                }
            }

            std::uniform_int_distribution<int> moveDist(0, 10);
            int move = moveDist(rng);

            if (move < 2 && currentX > 1)
                currentX--;
            else if (move > 8 && currentX < (WIDTH * 16) - 2)
                currentX++;
            else
                currentZ++;
        }
    }

    void Town::CarvePond(std::mt19937_64 &rng)
    {
        std::uniform_int_distribution<int> xDist(5, (WIDTH * 16) - 6);
        std::uniform_int_distribution<int> zDist(5, (HEIGHT * 16) - 6);

        glm::ivec2 seedPos(xDist(rng), zDist(rng));

        // Explicitly using std::queue for BFS
        std::queue<glm::ivec2> q;
        q.push(seedPos);

        int maxTiles = 25;
        int carved = 0;

        while (!q.empty() && carved < maxTiles)
        {
            glm::ivec2 curr = q.front();
            q.pop();

            auto [acre, local] = WorldToTile(glm::vec3(curr.x, 0, curr.y));

            // Skip if already processed
            if (m_Acres[acre.x][acre.y].tiles[local.y][local.x].type == TileType::WATER)
                continue;

            m_Acres[acre.x][acre.y].tiles[local.y][local.x].type = TileType::WATER;
            carved++;

            // Use simple offsets for neighbors to avoid "type name not allowed" errors
            const int dx[] = {1, -1, 0, 0};
            const int dz[] = {0, 0, 1, -1};

            for (int i = 0; i < 4; ++i)
            {
                int nx = curr.x + dx[i];
                int nz = curr.y + dz[i];

                if (nx >= 0 && nx < WIDTH * 16 && nz >= 0 && nz < HEIGHT * 16)
                {
                    std::uniform_int_distribution<int> chance(0, 10);
                    if (chance(rng) > 3)
                    {
                        q.push(glm::ivec2(nx, nz));
                    }
                }
            }
        }
    }

    std::vector<rendering::TileInstance> Town::GenerateRenderData() const
    {
        std::vector<rendering::TileInstance> instances;
        instances.reserve(WIDTH * HEIGHT * 16 * 16);

        for (int ax = 0; ax < WIDTH; ++ax)
        {
            for (int ay = 0; ay < HEIGHT; ++ay)
            {
                for (int ly = 0; ly < 16; ++ly)
                {
                    for (int lx = 0; lx < 16; ++lx)
                    {
                        const Tile &tile = m_Acres[ax][ay].tiles[ly][lx];
                        rendering::TileInstance inst;

                        float worldX = static_cast<float>(ax * 16 + lx);
                        float worldZ = static_cast<float>(ay * 16 + ly);

                        inst.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(worldX, (float)tile.elevation, worldZ));

                        if (tile.type == TileType::WATER)
                            inst.color = {0.1f, 0.4f, 0.8f};
                        else if (tile.type == TileType::GRASS)
                            inst.color = {0.3f, 0.6f, 0.2f};
                        else
                            inst.color = {0.5f, 0.5f, 0.5f};

                        inst.padding = 0.0f;
                        instances.push_back(inst);
                    }
                }
            }
        }
        return instances;
    }
}
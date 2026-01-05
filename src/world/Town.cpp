#include "Town.h"
#include <glm/gtc/matrix_transform.hpp>
#include <stack>
#include <queue>
#include <array>
#include <cmath>
#include <algorithm>
#include <iostream>

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

        x = std::clamp(x, 0, (WIDTH * 16) - 1);
        z = std::clamp(z, 0, (HEIGHT * 16) - 1);

        return {{x / 16, z / 16}, {x % 16, z % 16}};
    }

    void Town::Generate(uint64_t seed, const TownConfig &config)
    {
        std::mt19937_64 rng(seed);
        GenContext ctx{rng, config};

        // Step 1: Initialize/Reset Town
        for (auto &col : m_Acres)
            for (auto &acre : col)
                for (auto &row : acre.tiles)
                    for (auto &tile : row)
                    {
                        tile.type = TileType::GRASS;
                        tile.elevation = 0;
                    }

        // Step 2: Run Pipeline
        GenerateCliffs(ctx);
        CarveRiver(ctx);
        CarvePond(ctx);
    }

    void Town::GenerateCliffs(GenContext &ctx)
    {
        float transition_start = 1.0f - ctx.config.cliffSmoothness;
        std::uniform_int_distribution<int> row_dist(ctx.config.minPlateauRow, ctx.config.maxPlateauRow);

        std::array<int, WIDTH> column_targets;
        for (int ax = 0; ax < WIDTH; ++ax)
            column_targets[ax] = (row_dist(ctx.rng) + 1) * 16;

        std::vector<int> boundary_line(WIDTH * 16);
        for (int x = 0; x < WIDTH * 16; ++x)
        {
            int current_acre = x / 16;
            int next_acre = std::min(current_acre + 1, WIDTH - 1);
            float t = (x % 16) / 16.0f;

            float transition = 0.0f;
            if (ctx.config.cliffSmoothness > 0.0f && t >= transition_start)
            {
                float local_t = (t - transition_start) / ctx.config.cliffSmoothness;
                // Smoothstep for rounded corners
                transition = local_t * local_t * (3.0f - 2.0f * local_t);
            }

            float bz = (1.0f - transition) * column_targets[current_acre] +
                       (transition * column_targets[next_acre]);
            boundary_line[x] = static_cast<int>(std::round(bz));
        }

        // Pass 1: Set Elevations
        for (int x = 0; x < WIDTH * 16; ++x)
        {
            for (int z = 0; z < HEIGHT * 16; ++z)
            {
                auto [a, l] = WorldToTile(glm::vec3(x, 0, z));
                m_Acres[a.x][a.y].tiles[l.y][l.x].elevation = (z < boundary_line[x]) ? 1 : 0;
            }
        }

        // Pass 2: 4-Way Boundary Tagging (Handles Left/Top edges)
        for (int x = 0; x < WIDTH * 16; ++x)
        {
            for (int z = 0; z < HEIGHT * 16; ++z)
            {
                auto [a, l] = WorldToTile(glm::vec3(x, 0, z));
                auto &tile = m_Acres[a.x][a.y].tiles[l.y][l.x];
                if (tile.elevation == 0)
                    continue;

                const int dx[] = {1, -1, 0, 0}, dz[] = {0, 0, 1, -1};
                for (int i = 0; i < 4; ++i)
                {
                    int nx = x + dx[i], nz = z + dz[i];
                    if (nx >= 0 && nx < WIDTH * 16 && nz >= 0 && nz < HEIGHT * 16)
                    {
                        auto [aN, lN] = WorldToTile(glm::vec3(nx, 0, nz));
                        if (m_Acres[aN.x][aN.y].tiles[lN.y][lN.x].elevation < tile.elevation)
                        {
                            tile.type = TileType::CLIFF;
                            break;
                        }
                    }
                }
            }
        }
    }

    void Town::CarveRiver(GenContext &ctx)
    {
        std::uniform_int_distribution<int> xDist(0, (WIDTH * 16) - 1);
        int currentX = xDist(ctx.rng);
        int currentZ = 0;

        int halfWidth = ctx.config.riverWidth / 2;

        while (currentZ < (HEIGHT * 16))
        {
            for (int dx = -halfWidth; dx <= (ctx.config.riverWidth - halfWidth - 1); ++dx)
            {
                int tx = currentX + dx;
                if (tx >= 0 && tx < WIDTH * 16)
                {
                    auto [a, l] = WorldToTile(glm::vec3(tx, 0, currentZ));
                    m_Acres[a.x][a.y].tiles[l.y][l.x].type = TileType::WATER;
                }
            }

            std::uniform_int_distribution<int> roll(0, 100);
            int m = roll(ctx.rng);
            if (m < ctx.config.riverMeanderChance / 2 && currentX > 1)
                currentX--;
            else if (m > 100 - (ctx.config.riverMeanderChance / 2) && currentX < (WIDTH * 16) - 2)
                currentX++;
            else
                currentZ++;
        }
    }

    void Town::CarvePond(GenContext &ctx)
    {
        std::uniform_int_distribution<int> xDist(5, (WIDTH * 16) - 6);
        std::uniform_int_distribution<int> zDist(5, (HEIGHT * 16) - 6);

        std::queue<glm::ivec2> q;
        q.push({xDist(ctx.rng), zDist(ctx.rng)});

        int carved = 0;
        while (!q.empty() && carved < ctx.config.maxPondSize)
        {
            glm::ivec2 curr = q.front();
            q.pop();
            auto [a, l] = WorldToTile(glm::vec3(curr.x, 0, curr.y));

            if (m_Acres[a.x][a.y].tiles[l.y][l.x].type == TileType::WATER)
                continue;

            m_Acres[a.x][a.y].tiles[l.y][l.x].type = TileType::WATER;
            carved++;

            const int dx[] = {1, -1, 0, 0}, dz[] = {0, 0, 1, -1};
            for (int i = 0; i < 4; ++i)
            {
                int nx = curr.x + dx[i], nz = curr.y + dz[i];
                if (nx >= 0 && nx < WIDTH * 16 && nz >= 0 && nz < HEIGHT * 16)
                {
                    std::uniform_int_distribution<int> chance(0, 100);
                    if (chance(ctx.rng) < ctx.config.pondSpreadChance)
                        q.push({nx, nz});
                }
            }
        }
    }

    std::vector<rendering::TileInstance> Town::GenerateRenderData() const
    {
        std::vector<rendering::TileInstance> instances;
        instances.reserve(WIDTH * HEIGHT * Acre::SIZE * Acre::SIZE * 2);

        for (int ax = 0; ax < WIDTH; ++ax)
        {
            for (int ay = 0; ay < HEIGHT; ++ay)
            {
                for (int ly = 0; ly < Acre::SIZE; ++ly)
                {
                    for (int lx = 0; lx < Acre::SIZE; ++lx)
                    {
                        const Tile &tile = m_Acres[ax][ay].tiles[ly][lx];
                        float worldX = static_cast<float>(ax * Acre::SIZE + lx);
                        float worldZ = static_cast<float>(ay * Acre::SIZE + ly);

                        for (int y = 0; y <= tile.elevation; ++y)
                        {
                            rendering::TileInstance inst;
                            inst.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(worldX, (float)y, worldZ));

                            if (y < tile.elevation)
                            {
                                inst.color = {0.45f, 0.35f, 0.25f}; // Dirt filler
                            }
                            else
                            {
                                if (tile.type == TileType::WATER)
                                {
                                    float depthShade = 1.0f - (y * 0.1f);
                                    inst.color = {0.1f * depthShade, 0.4f * depthShade, 0.8f * depthShade};
                                }
                                else if (tile.type == TileType::CLIFF)
                                {
                                    inst.color = {0.5f, 0.45f, 0.4f};
                                }
                                else
                                {
                                    float elevationLight = 0.5f + (y * 0.15f);
                                    inst.color = {0.3f * elevationLight, 0.6f * elevationLight, 0.2f * elevationLight};
                                }
                            }
                            inst.padding = 0.0f;
                            instances.push_back(inst);
                        }
                    }
                }
            }
        }
        return instances;
    }

    void Town::DebugDump() const
    {
        // 1. Print Horizontal Header (Row Numbers)
        // Shift right to account for the vertical header space
        std::cout << "    ";
        for (int ax = 0; ax < WIDTH; ++ax)
        {
            // Center the number over the 16 tiles of the acre
            std::cout << " Acre " << (ax + 1) << "        ";
        }
        std::cout << "\n";

        // 2. Iterate through every world row (Z axis)
        for (int z = 0; z < HEIGHT * 16; ++z)
        {
            // Print Vertical Header (Column Letters A, B, C...)
            // We only print the letter at the start of a new Acre row
            if (z % 16 == 0)
            {
                char acreLetter = 'A' + (z / 16);
                std::cout << acreLetter << " | ";
            }
            else
            {
                std::cout << "  | ";
            }

            // 3. Print the Tiles in this row
            for (int x = 0; x < WIDTH * 16; ++x)
            {
                auto [a, l] = WorldToTile(glm::vec3(x, 0, z));
                const auto &tile = m_Acres[a.x][a.y].tiles[l.y][l.x];

                // Print TileType as an int (0=Empty, 1=Grass, 3=Water, 7=Cliff, etc.)
                std::cout << static_cast<int>(tile.type);

                // Optional: Add a separator between acres for clarity
                if ((x + 1) % 16 == 0)
                    std::cout << " ";
            }
            std::cout << "\n";

            // Optional: Add a horizontal line between acre rows
            if ((z + 1) % 16 == 0)
            {
                std::cout << "  " << std::string((WIDTH * 17) + 4, '-') << "\n";
            }
        }
    }
}
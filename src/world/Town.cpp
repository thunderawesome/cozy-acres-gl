#include "Town.h"
#include "generation/GenerationPipeline.h"
#include "generation/steps/CliffGenerationStep.h"
#include "generation/steps/RiverGenerationStep.h"
#include "generation/steps/PondGenerationStep.h"

#include <iostream>
#include <algorithm>
#include <cmath>
#include <string>

#include <glm/gtc/matrix_transform.hpp>

namespace cozy::world
{

    // ── Coordinate conversion ──────────────────────────────────────────────────
    std::pair<glm::ivec2, glm::ivec2> Town::WorldToTile(glm::vec3 world_pos) const
    {
        int x = static_cast<int>(std::floor(world_pos.x));
        int z = static_cast<int>(std::floor(world_pos.z));

        x = std::clamp(x, 0, (WIDTH * Acre::SIZE) - 1);
        z = std::clamp(z, 0, (HEIGHT * Acre::SIZE) - 1);

        return {
            {x / Acre::SIZE, z / Acre::SIZE},
            {x % Acre::SIZE, z % Acre::SIZE}};
    }

    // ── Get elevation at world coordinates ─────────────────────────────────────
    int Town::GetElevation(int wx, int wz) const
    {
        if (wx < 0 || wx >= WIDTH * Acre::SIZE ||
            wz < 0 || wz >= HEIGHT * Acre::SIZE)
        {
            return -1;
        }

        auto [acre_pos, local_pos] = WorldToTile(
            glm::vec3(static_cast<float>(wx), 0.0f, static_cast<float>(wz)));

        return m_acres[acre_pos.x][acre_pos.y].tiles[local_pos.y][local_pos.x].elevation;
    }

    // ── Full town generation using composition-based pipeline ──────────────────
    void Town::Generate(uint64_t seed, const TownConfig &config)
    {
        // 1. Reset / initialize all tiles
        for (auto &column : m_acres)
        {
            for (auto &acre : column)
            {
                for (auto &row : acre.tiles)
                {
                    for (auto &tile : row)
                    {
                        tile.type = TileType::GRASS;
                        tile.elevation = 0;
                    }
                }
            }
        }

        // 2. Create and configure generation pipeline
        GenerationPipeline pipeline(*this);

        pipeline.AddStep(cliffs::Execute);
        pipeline.AddStep(rivers::Execute);
        pipeline.AddStep(ponds::Execute);

        // 3. Run the whole generation process
        pipeline.Execute(seed, config);

        // 4. Optional post-generation actions
        DebugDump();
        std::cout << "Generation completed - seed: " << seed << "\n";
    }

    // ── Generate render data ──────────────────────────
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
                        const Tile &tile = m_acres[ax][ay].tiles[ly][lx];

                        float worldX = static_cast<float>(ax * Acre::SIZE + lx);
                        float worldZ = static_cast<float>(ay * Acre::SIZE + ly);

                        for (int y = 0; y <= tile.elevation; ++y)
                        {
                            rendering::TileInstance inst;
                            inst.modelMatrix = glm::translate(
                                glm::mat4(1.0f),
                                glm::vec3(worldX, static_cast<float>(y), worldZ));

                            if (y < tile.elevation)
                            {
                                inst.color = {0.45f, 0.35f, 0.25f}; // Dirt filler
                            }
                            else
                            {
                                if (tile.type == TileType::WATER)
                                {
                                    float depthShade = 1.0f - (y * 0.1f);
                                    inst.color = {
                                        0.1f * depthShade,
                                        0.4f * depthShade,
                                        0.8f * depthShade};
                                }
                                else if (tile.type == TileType::CLIFF)
                                {
                                    inst.color = {0.5f, 0.45f, 0.4f};
                                }
                                else
                                {
                                    float elevationLight = 0.5f + (y * 0.15f);
                                    inst.color = {
                                        0.3f * elevationLight,
                                        0.6f * elevationLight,
                                        0.2f * elevationLight};
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

    // ── Debug dump visualization ────────────────────────────
    void Town::DebugDump() const
    {
        constexpr int TOTAL_WIDTH = WIDTH * Acre::SIZE;

        // Header
        std::cout << "     ";
        for (int ax = 0; ax < WIDTH; ++ax)
        {
            std::string label = "Acre " + std::to_string(ax + 1);
            std::cout << label;

            // Calculate padding based on string length
            for (size_t i = label.length(); i < 17; ++i)
                std::cout << " ";
        }
        std::cout << "\n";

        std::cout << "    +";
        for (int i = 0; i < TOTAL_WIDTH + WIDTH; ++i)
            std::cout << "-";
        std::cout << "+\n";

        // Grid
        for (int az = 0; az < HEIGHT; ++az)
        {
            char rowLetter = 'A' + az;

            for (int local_z = 0; local_z < Acre::SIZE; ++local_z)
            {
                int z = az * Acre::SIZE + local_z;

                if (local_z == 0)
                    std::cout << rowLetter << rowLetter << " | ";
                else
                    std::cout << "   | ";

                for (int x = 0; x < TOTAL_WIDTH; ++x)
                {
                    auto [a, l] = WorldToTile(glm::vec3(static_cast<float>(x), 0.0f, static_cast<float>(z)));
                    // Inside the grid printing loop
                    const auto &tile = m_acres[a.x][a.y].tiles[l.y][l.x];

                    // Always show the CURRENT tile's top surface
                    char symbol;
                    if (tile.type == TileType::WATER)
                    {
                        switch (tile.elevation)
                        {
                        case 2:
                            symbol = 'W';
                            break;
                        case 1:
                            symbol = 'w';
                            break;
                        case 0:
                            symbol = '~';
                            break;
                        default:
                            symbol = '?';
                            break;
                        }
                    }
                    else if (tile.type == TileType::CLIFF)
                    {
                        symbol = (tile.elevation == 2) ? '#' : '=';
                    }
                    else
                    {
                        switch (tile.elevation)
                        {
                        case 2:
                            symbol = '^';
                            break;
                        case 1:
                            symbol = 'o';
                            break;
                        case 0:
                            symbol = '.';
                            break;
                        default:
                            symbol = '?';
                            break;
                        }
                    }

                    std::cout << symbol;

                    if ((x + 1) % Acre::SIZE == 0)
                        std::cout << " ";
                }
                std::cout << "\n";
            }

            std::cout << "    +";
            for (int i = 0; i < TOTAL_WIDTH + WIDTH; ++i)
                std::cout << "-";
            std::cout << "+\n";
        }

        // Legend
        std::cout << "\nLegend:\n"
                  << "  . = Ground / Beach (level 0)\n"
                  << "  o = Mid Plateau (level 1)\n"
                  << "  ^ = High Plateau (level 2)\n"
                  << "  = = Mid-level Cliff Face\n"
                  << "  # = High-level Cliff Face\n"
                  << "  W = Water (High Plateau)\n"
                  << "  w = Water (Mid Plateau)\n"
                  << "  ~ = Water (Ground / Beach)\n\n";
    }

}
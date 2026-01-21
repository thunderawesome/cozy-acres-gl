#include "Town.h"
#include "generation/GenerationPipeline.h"
#include "generation/steps/OceanGenerationStep.h"
#include "generation/steps/CliffGenerationStep.h"
#include "generation/steps/RiverGenerationStep.h"
#include "generation/steps/RampGenerationStep.h"
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

        pipeline.AddStep(ocean::Execute);
        pipeline.AddStep(cliffs::Execute);
        pipeline.AddStep(rivers::Execute);
        pipeline.AddStep(ramps::Execute);
        pipeline.AddStep(ponds::Execute);

        // 3. Run the whole generation process
        pipeline.Execute(seed, config);

        // 4. Optional post-generation actions
        DebugDump();
        std::cout << "Generation completed - seed: " << seed << "\n";
    }

    glm::vec3 Town::GetTileColor(const Tile &tile, int y) const
    {
        float depthShade = 1.0f - (y * 0.1f);
        float elevationLight = 0.5f + (y * 0.15f);

        switch (tile.type)
        {
        case TileType::RIVER:
            return {0.12f * depthShade, 0.45f * depthShade, 0.85f * depthShade};

        case TileType::POND:
        {
            // Index 46 is the "Full Interior" (water on all 8 sides)
            if (tile.autotileIndex == 46)
            {
                return {0.08f, 0.42f, 0.75f}; // Dark Blue Interior
            }
            else
            {
                return {0.5f, 1.0f, 1.0f}; // Bright Teal Border (Any of the other 46 tiles)
            }
        }
        case TileType::WATERFALL:
            return {0.28f * depthShade, 0.72f * depthShade, 0.95f * depthShade};

        case TileType::OCEAN:
            return {0.05f * depthShade, 0.2f * depthShade, 0.6f * depthShade};

        case TileType::RIVER_MOUTH:
            return {0.08f, 0.32f, 0.72f};

        case TileType::SAND:
            return {0.86f, 0.78f, 0.62f};

        case TileType::CLIFF:
            return {0.5f, 0.45f, 0.4f};

        case TileType::RAMP:
            return {0.55f * elevationLight, 0.50f * elevationLight, 0.35f * elevationLight};

        case TileType::GRASS:
        default:
            return {0.3f * elevationLight, 0.6f * elevationLight, 0.2f * elevationLight};
        }
    }

    // ── Generate render data ──────────────────────────
    std::vector<rendering::TileInstance> Town::GenerateRenderData() const
    {
        std::vector<rendering::TileInstance> instances;
        instances.reserve(WIDTH * HEIGHT * Acre::SIZE * Acre::SIZE * 2);

        for (int ax = 0; ax < WIDTH; ++ax) // Rows (A, B, C...)
        {
            for (int az = 0; az < HEIGHT; ++az) // Columns (1, 2, 3...)
            {
                for (int lx = 0; lx < Acre::SIZE; ++lx)
                {
                    for (int lz = 0; lz < Acre::SIZE; ++lz)
                    {
                        const Tile &tile = m_acres[ax][az].tiles[lz][lx];

                        float worldX = static_cast<float>(ax * Acre::SIZE + lx);
                        float worldZ = static_cast<float>(az * Acre::SIZE + lz);

                        // Generate geometry for the surface and dirt filler below
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
                                // Surface coloring logic
                                inst.color = GetTileColor(tile, y);
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
            int label_spacing = 17;
            for (size_t i = label.length(); i < label_spacing; ++i)
                std::cout << " ";
        }
        std::cout << "\n";

        std::cout << "  +";
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
                    std::cout << rowLetter << " | ";
                else
                    std::cout << "  | ";

                for (int x = 0; x < TOTAL_WIDTH; ++x)
                {
                    // Note: WorldToTile is used here for safety, though direct acre access is faster
                    auto [a, l] = WorldToTile(glm::vec3(static_cast<float>(x), 0.0f, static_cast<float>(z)));
                    const auto &tile = m_acres[a.x][a.y].tiles[l.y][l.x];

                    char symbol;
                    switch (tile.type)
                    {
                    case TileType::OCEAN:
                        symbol = 'W';
                        break; // 'W' for Water/Waves
                    case TileType::SAND:
                        symbol = 's';
                        break; // Lowercase 's' for sand
                    case TileType::RIVER_MOUTH:
                        symbol = 'M';
                        break; // 'M' for Mouth
                    case TileType::WATERFALL:
                        symbol = 'V';
                        break; // 'V' for Waterfall
                    case TileType::RIVER:
                        if (tile.elevation == 2)
                            symbol = 'R';
                        else if (tile.elevation == 1)
                            symbol = 'r';
                        else
                            symbol = '~';
                        break;
                    case TileType::POND:
                        if (tile.elevation == 2)
                            symbol = 'P';
                        else if (tile.elevation == 1)
                            symbol = 'p';
                        else
                            symbol = 'o';
                        break;
                    case TileType::CLIFF:
                        symbol = (tile.elevation == 2) ? '#' : '=';
                        break;
                    case TileType::RAMP:
                        symbol = (tile.elevation == 2) ? '/' : '\\';
                        break;
                    default: // GRASS
                        if (tile.elevation == 2)
                            symbol = '^'; // Peak
                        else if (tile.elevation == 1)
                            symbol = ';'; // Mid-level
                        else
                            symbol = '.'; // Ground
                        break;
                    }

                    std::cout << symbol;

                    if ((x + 1) % Acre::SIZE == 0)
                        std::cout << " ";
                }
                std::cout << "\n";
            }

            std::cout << "  +";
            for (int i = 0; i < TOTAL_WIDTH + WIDTH; ++i)
                std::cout << "-";
            std::cout << "+\n";
        }

        // Legend
        std::cout << "\nLegend:\n"
                  << "  Terrain:\n"
                  << "    . = Grass (Lvl 0)    ; = Grass (Lvl 1)    ^ = Grass (Lvl 2)\n"
                  << "    s = Sand (Beach)     W = Ocean\n"
                  << "  Water:\n"
                  << "    ~ = River (Lvl 0)    r = River (Lvl 1)    R = River (Lvl 2)\n"
                  << "    o = Pond (Lvl 0)     p = Pond (Lvl 1)     P = Pond (Lvl 2)\n"
                  << "    M = River Mouth      V = Waterfall\n"
                  << "  Vertical:\n"
                  << "    = = Mid Cliff        # = High Cliff\n"
                  << "    / , \\ = Ramps\n\n";
    }
}
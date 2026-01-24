#include "TownPresenter.h"
#include "world/generation/utils/WorldGenUtils.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <string>

namespace cozy::world
{
    std::vector<rendering::TileInstance> TownPresenter::GenerateRenderData(const Town &town)
    {
        std::vector<rendering::TileInstance> instances;
        instances.reserve(Town::WIDTH * Town::HEIGHT * Acre::SIZE * Acre::SIZE);

        for (int ax = 0; ax < Town::WIDTH; ++ax)
        {
            for (int az = 0; az < Town::HEIGHT; ++az)
            {
                for (int lx = 0; lx < Acre::SIZE; ++lx)
                {
                    for (int lz = 0; lz < Acre::SIZE; ++lz)
                    {
                        const Tile &tile = town.GetAcre(ax, az).tiles[lz][lx];

                        rendering::TileInstance inst;
                        float wx = static_cast<float>(ax * Acre::SIZE + lx);
                        float wz = static_cast<float>(az * Acre::SIZE + lz);

                        inst.modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(wx, (float)tile.elevation, wz));
                        inst.color = GetTileColor(tile, tile.elevation);
                        instances.push_back(inst);
                    }
                }
            }
        }
        return instances;
    }

    glm::vec3 TownPresenter::GetTileColor(const Tile &tile, int y)
    {
        float depthShade = 1.0f - (y * 0.1f);
        float elevationLight = 0.5f + (y * 0.15f);

        switch (tile.type)
        {
        // Grouped Water Types (Rivers/Ponds)
        case TileType::RIVER:
        case TileType::POND:
        case TileType::RIVER_MOUTH:
            // Index 46 is the "Full Interior" (deep water)
            return (tile.autotileIndex == 46)
                       ? glm::vec3{0.08f, 0.42f, 0.75f} * depthShade // Deep blue
                       : glm::vec3{0.4f, 0.9f, 1.0f} * depthShade;   // Shoreline teal

        case TileType::WATERFALL:
            // Foamy, lighter look
            return glm::vec3{0.5f, 0.8f, 1.0f} * depthShade;

        case TileType::OCEAN:
            // Very deep blue
            return glm::vec3{0.05f, 0.2f, 0.6f} * depthShade;

        case TileType::SAND:
            return {0.86f, 0.78f, 0.62f};

        case TileType::CLIFF:
            // Cliffs usually look better without elevation light to keep them "grounded"
            return {0.5f, 0.45f, 0.4f};

        case TileType::RAMP:
            // Uses elevation light to blend better with grass transitions
            return glm::vec3{0.55f, 0.50f, 0.35f} * elevationLight;

        case TileType::GRASS:
        default:
            return glm::vec3{0.3f, 0.6f, 0.2f} * elevationLight;
        }
    }

    void TownPresenter::DebugDump(const Town &town)
    {
        const int TOTAL_WIDTH = Town::WIDTH * Acre::SIZE;
        const int TOTAL_HEIGHT = Town::HEIGHT * Acre::SIZE;

        std::cout << "\n--- Town Generation Debug Dump ---\n";

        // 1. Column Headers (Acre 1, Acre 2, etc.)
        std::cout << "     ";
        for (int ax = 0; ax < Town::WIDTH; ++ax)
        {
            std::string label = "Acre " + std::to_string(ax + 1);
            std::cout << label;
            // Padding to align with Acre size (16 tiles + 1 space)
            int label_spacing = Acre::SIZE + 1;
            for (size_t i = label.length(); i < (size_t)label_spacing; ++i)
                std::cout << " ";
        }
        std::cout << "\n";

        // 2. Top Border
        std::cout << "  +";
        for (int i = 0; i < TOTAL_WIDTH + Town::WIDTH; ++i)
            std::cout << "-";
        std::cout << "+\n";

        // 3. Grid Rendering
        for (int z = 0; z < TOTAL_HEIGHT; ++z)
        {
            // Acre Row Labels (A, B, C...)
            if (z % Acre::SIZE == 0)
            {
                char rowLetter = static_cast<char>('A' + (z / Acre::SIZE));
                std::cout << rowLetter << " | ";
            }
            else
            {
                std::cout << "  | ";
            }

            for (int x = 0; x < TOTAL_WIDTH; ++x)
            {
                // Use refactored Utils for safe access
                TileType type = utils::GetTileTypeSafe(town, x, z);
                int elevation = utils::GetElevation(town, x, z);

                char symbol;
                switch (type)
                {
                case TileType::OCEAN:
                    symbol = 'W';
                    break;
                case TileType::SAND:
                    symbol = 's';
                    break;
                case TileType::RIVER_MOUTH:
                    symbol = 'M';
                    break;
                case TileType::WATERFALL:
                    symbol = 'V';
                    break;
                case TileType::RIVER:
                    if (elevation == 2)
                        symbol = 'R';
                    else if (elevation == 1)
                        symbol = 'r';
                    else
                        symbol = '~';
                    break;
                case TileType::POND:
                    if (elevation == 2)
                        symbol = 'P';
                    else if (elevation == 1)
                        symbol = 'p';
                    else
                        symbol = 'o';
                    break;
                case TileType::CLIFF:
                    symbol = (elevation == 2) ? '#' : '=';
                    break;
                case TileType::RAMP:
                    symbol = (elevation == 2) ? '/' : '\\';
                    break;
                case TileType::EMPTY:
                    symbol = ' ';
                    break;
                default: // GRASS
                    if (elevation == 2)
                        symbol = '^';
                    else if (elevation == 1)
                        symbol = ';';
                    else
                        symbol = '.';
                    break;
                }

                std::cout << symbol;

                // Visual separator between Acres
                if ((x + 1) % Acre::SIZE == 0)
                    std::cout << " ";
            }
            std::cout << "|\n";

            // Acre Row Horizontal Separator
            if ((z + 1) % Acre::SIZE == 0 && (z + 1) != TOTAL_HEIGHT)
            {
                std::cout << "  +";
                for (int i = 0; i < TOTAL_WIDTH + Town::WIDTH; ++i)
                    std::cout << "-";
                std::cout << "+\n";
            }
        }

        // 4. Bottom Border
        std::cout << "  +";
        for (int i = 0; i < TOTAL_WIDTH + Town::WIDTH; ++i)
            std::cout << "-";
        std::cout << "+\n";

        // 5. Legend
        std::cout << "\nLegend:\n"
                  << "  Terrain: . = Grass (L0)  ; = Grass (L1)  ^ = Grass (L2)\n"
                  << "           s = Sand        W = Ocean\n"
                  << "  Water:   ~ = River (L0)  r = River (L1)  R = River (L2)\n"
                  << "           o = Pond (L0)   p = Pond (L1)   P = Pond (L2)\n"
                  << "           M = Mouth       V = Waterfall\n"
                  << "  Vertical: = = Mid Cliff  # = High Cliff  / \\ = Ramps\n\n";
    }
}
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
        // CarvePond(ctx);
        DebugDump();
        std::cout << "Seed: " << seed;
        std::cout << "\n";
    }

    void Town::GenerateCliffs(GenContext &ctx)
    {
        const int CONNECTION_POINT = 12; // Fixed connection point within each acre

        // === Mid Plateau (Level 1) Boundary ===
        std::uniform_int_distribution<int> mid_row_dist(ctx.config.minPlateauRow, ctx.config.maxPlateauRow);

        // Generate per-acre targets
        std::array<int, WIDTH> mid_column_targets;
        for (int ax = 0; ax < WIDTH; ++ax)
        {
            mid_column_targets[ax] = (mid_row_dist(ctx.rng) + 1) * Acre::SIZE;
        }

        // Build boundary line with step transitions at z=12
        std::vector<int> mid_boundary_line(WIDTH * Acre::SIZE);
        for (int x = 0; x < WIDTH * Acre::SIZE; ++x)
        {
            int current_acre = x / Acre::SIZE;
            int next_acre = std::min(current_acre + 1, WIDTH - 1);
            int local_x = x % Acre::SIZE;

            // At x=12: transition to next acre's target
            if (local_x < CONNECTION_POINT)
            {
                mid_boundary_line[x] = mid_column_targets[current_acre];
            }
            else if (local_x == CONNECTION_POINT)
            {
                // Transition point - could average or pick one
                mid_boundary_line[x] = (current_acre == WIDTH - 1) ? mid_column_targets[current_acre] : mid_column_targets[next_acre];
            }
            else
            {
                mid_boundary_line[x] = (current_acre == WIDTH - 1) ? mid_column_targets[current_acre] : mid_column_targets[next_acre];
            }
        }

        // === High Plateau (Level 2) Boundary ===
        std::uniform_int_distribution<int> high_row_dist(
            ctx.config.minHighPlateauRowOffset,
            ctx.config.maxHighPlateauRowOffset);

        std::array<int, WIDTH> high_column_targets;
        for (int ax = 0; ax < WIDTH; ++ax)
        {
            int row = high_row_dist(ctx.rng);
            int candidate = (row + 1) * Acre::SIZE;

            // Strictly enforce: high plateau must start at least one full acre behind the mid plateau
            int max_allowed = mid_column_targets[ax] - Acre::SIZE;
            high_column_targets[ax] = std::min(candidate, max_allowed);
            high_column_targets[ax] = std::max(high_column_targets[ax], Acre::SIZE);
        }

        std::vector<int> high_boundary_line(WIDTH * Acre::SIZE);
        for (int x = 0; x < WIDTH * Acre::SIZE; ++x)
        {
            int current_acre = x / Acre::SIZE;
            int next_acre = std::min(current_acre + 1, WIDTH - 1);
            int local_x = x % Acre::SIZE;

            if (local_x < CONNECTION_POINT)
            {
                high_boundary_line[x] = high_column_targets[current_acre];
            }
            else if (local_x == CONNECTION_POINT)
            {
                high_boundary_line[x] = (current_acre == WIDTH - 1) ? high_column_targets[current_acre] : high_column_targets[next_acre];
            }
            else
            {
                high_boundary_line[x] = (current_acre == WIDTH - 1) ? high_column_targets[current_acre] : high_column_targets[next_acre];
            }

            // Clamp to prevent overlap
            int max_safe = mid_column_targets[current_acre] - Acre::SIZE;
            if (current_acre + 1 < WIDTH)
                max_safe = std::min(max_safe, mid_column_targets[next_acre] - Acre::SIZE);

            high_boundary_line[x] = std::min(high_boundary_line[x], max_safe);
            high_boundary_line[x] = std::max(high_boundary_line[x], 0);
        }

        // === Pass 1: Set Elevations with Z-axis snapping ===
        for (int x = 0; x < WIDTH * Acre::SIZE; ++x)
        {
            for (int z = 0; z < HEIGHT * Acre::SIZE; ++z)
            {
                auto [a, l] = WorldToTile(glm::vec3(x, 0, z));
                auto &tile = m_Acres[a.x][a.y].tiles[l.y][l.x];

                int local_z = z % Acre::SIZE;

                // Determine base elevation from boundary lines
                int base_elevation;
                if (z < high_boundary_line[x])
                    base_elevation = 2;
                else if (z < mid_boundary_line[x])
                    base_elevation = 1;
                else
                    base_elevation = 0;

                // Apply Z-axis snapping: cliffs should only transition at z=12
                // Check if we're near a Z boundary between acres
                int current_z_acre = z / Acre::SIZE;
                int next_z_acre = std::min(current_z_acre + 1, HEIGHT - 1);

                if (local_z == CONNECTION_POINT && current_z_acre < HEIGHT - 1)
                {
                    // At the connection point, allow the transition
                    tile.elevation = base_elevation;
                }
                else if (local_z < CONNECTION_POINT)
                {
                    // Before connection point: check if we should snap to current acre's elevation
                    int check_z = current_z_acre * Acre::SIZE + CONNECTION_POINT;
                    if (check_z < HEIGHT * Acre::SIZE)
                    {
                        int snap_elevation;
                        if (check_z < high_boundary_line[x])
                            snap_elevation = 2;
                        else if (check_z < mid_boundary_line[x])
                            snap_elevation = 1;
                        else
                            snap_elevation = 0;

                        tile.elevation = snap_elevation;
                    }
                    else
                    {
                        tile.elevation = base_elevation;
                    }
                }
                else
                {
                    // After connection point: check next acre's elevation
                    int check_z = next_z_acre * Acre::SIZE + CONNECTION_POINT;
                    if (check_z < HEIGHT * Acre::SIZE)
                    {
                        int snap_elevation;
                        if (check_z < high_boundary_line[x])
                            snap_elevation = 2;
                        else if (check_z < mid_boundary_line[x])
                            snap_elevation = 1;
                        else
                            snap_elevation = 0;

                        tile.elevation = snap_elevation;
                    }
                    else
                    {
                        tile.elevation = base_elevation;
                    }
                }
            }
        }

        // === Pass 2: Cliff Tagging ===
        for (int x = 0; x < WIDTH * Acre::SIZE; ++x)
        {
            for (int z = 0; z < HEIGHT * Acre::SIZE; ++z)
            {
                auto [a, l] = WorldToTile(glm::vec3(x, 0, z));
                auto &tile = m_Acres[a.x][a.y].tiles[l.y][l.x];
                if (tile.elevation == 0)
                    continue;

                const int dx[] = {1, -1, 0, 0}, dz[] = {0, 0, 1, -1};
                for (int i = 0; i < 4; ++i)
                {
                    int nx = x + dx[i], nz = z + dz[i];
                    if (nx >= 0 && nx < WIDTH * Acre::SIZE && nz >= 0 && nz < HEIGHT * Acre::SIZE)
                    {
                        auto [aN, lN] = WorldToTile(glm::vec3(nx, 0, nz));
                        int neighbor_elev = m_Acres[aN.x][aN.y].tiles[lN.y][lN.x].elevation;
                        if (neighbor_elev < tile.elevation)
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
        // Z_CONNECTION_POINT: The row within an acre where the river shifts (3)
        const int Z_CONNECTION_POINT = 3;
        // X_CONNECTION_POINT: The column within an acre where the river is centered (12)
        const int X_CONNECTION_POINT = 3;

        int width = ctx.config.riverWidth;
        int halfWidth = width / 2;

        // === 1. Generate Target X-Coordinates per Acre Row ===
        // This defines which "Acre Column" the river flows through for each vertical acre
        std::uniform_int_distribution<int> col_dist(0, 3);
        std::array<int, HEIGHT> column_targets;
        for (int az = 0; az < HEIGHT; ++az)
        {
            // Target is the 12th tile of the chosen acre column
            column_targets[az] = (col_dist(ctx.rng) * Acre::SIZE) + X_CONNECTION_POINT;
        }

        // === 2. Build the Path Array ===
        std::vector<int> boundary_line(HEIGHT * Acre::SIZE);
        for (int z = 0; z < HEIGHT * Acre::SIZE; ++z)
        {
            int current_acre_z = z / Acre::SIZE;
            int next_acre_z = std::min(current_acre_z + 1, HEIGHT - 1);
            int local_z = z % Acre::SIZE;

            // If we haven't reached the connection row (3), stay in the current target
            // Otherwise, move to the next acre's target column
            if (local_z < Z_CONNECTION_POINT)
            {
                boundary_line[z] = column_targets[current_acre_z];
            }
            else
            {
                boundary_line[z] = column_targets[next_acre_z];
            }
        }

        // === 3. Drawing Helper ===
        auto PaintWater = [&](int centerX, int centerZ)
        {
            for (int dx = -halfWidth; dx <= halfWidth; ++dx)
            {
                for (int dz = -halfWidth; dz <= halfWidth; ++dz)
                {
                    int x = centerX + dx;
                    int y_coord = centerZ + dz;

                    // Bounds check for the world
                    if (x >= 0 && x < WIDTH * Acre::SIZE && y_coord >= 0 && y_coord < HEIGHT * Acre::SIZE)
                    {
                        auto [a, l] = WorldToTile(glm::vec3(x, 0, y_coord));
                        m_Acres[a.x][a.y].tiles[l.y][l.x].type = TileType::WATER;
                    }
                }
            }
        };

        // === 4. Iterate and Carve ===
        for (int z = 0; z < HEIGHT * Acre::SIZE; ++z)
        {
            int target_x = boundary_line[z];
            int local_z = z % Acre::SIZE;

            // Handle the horizontal elbow at the connection row
            if (local_z == Z_CONNECTION_POINT && z > 0)
            {
                int startX = boundary_line[z - 1];
                int endX = boundary_line[z];

                if (startX != endX)
                {
                    int dir = (endX > startX) ? 1 : -1;
                    // Draw the horizontal bridge from old X target to new X target
                    for (int x = startX; x != endX + dir; x += dir)
                    {
                        PaintWater(x, z);
                    }
                }
                else
                {
                    PaintWater(target_x, z);
                }
            }
            else
            {
                // Standard vertical flow
                PaintWater(target_x, z);
            }
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
        constexpr int TOTAL_WIDTH = WIDTH * Acre::SIZE;
        // Top header: Acre columns (1 to WIDTH)
        std::cout << "     "; // Space for row labels
        for (int ax = 0; ax < WIDTH; ++ax)
        {
            // Manual fixed-width formatting without setw/to_string
            char buf[16];
            sprintf(buf, "Acre %d", ax + 1); // Safe for small numbers
            // Center over 16 tiles + 1 space gap
            std::cout << buf;
            // Pad to 17 characters (16 tiles + 1 gap)
            for (int i = strlen(buf); i < 17; ++i)
                std::cout << " ";
        }
        std::cout << "\n";
        // Top border
        std::cout << "    +";
        for (int i = 0; i < TOTAL_WIDTH + WIDTH; ++i) // +WIDTH for gaps
            std::cout << "-";
        std::cout << "+\n";
        // Main grid: loop over acre rows (A, B, ...)
        for (int az = 0; az < HEIGHT; ++az)
        {
            char rowLetter = 'A' + az;
            // Print 16 tile rows for this acre row
            for (int local_z = 0; local_z < Acre::SIZE; ++local_z)
            {
                int z = az * Acre::SIZE + local_z;
                // Row label: strong on first line of acre row
                if (local_z == 0)
                    std::cout << rowLetter << rowLetter << " | "; // e.g., "AA |"
                else
                    std::cout << "   | ";
                // Print all tiles in this row
                for (int x = 0; x < TOTAL_WIDTH; ++x)
                {
                    auto [a, l] = WorldToTile(glm::vec3(x, 0, z));
                    const auto &tile = m_Acres[a.x][a.y].tiles[l.y][l.x];
                    char symbol;

                    // Water takes priority over other tile types
                    if (tile.type == TileType::WATER)
                    {
                        switch (tile.elevation)
                        {
                        case 2:
                            symbol = 'W';
                            break; // High plateau
                        case 1:
                            symbol = 'w';
                            break; // Mid plateau
                        case 0:
                            symbol = '~';
                            break; // Ground/beach
                        default:
                            symbol = '?';
                            break;
                        }
                    }
                    else if (tile.type == TileType::CLIFF)
                    {
                        symbol = (tile.elevation == 2) ? '#' : '='; // # = high cliff, = = mid cliff
                    }
                    else
                    {
                        switch (tile.elevation)
                        {
                        case 2:
                            symbol = '^';
                            break; // High plateau (▲ alternative: ^)
                        case 1:
                            symbol = 'o';
                            break; // Mid plateau (△ alternative: o or -)
                        case 0:
                            symbol = '.';
                            break; // Ground/beach
                        default:
                            symbol = '?';
                            break;
                        }
                    }
                    std::cout << symbol;
                    // Vertical separator after each acre
                    if ((x + 1) % Acre::SIZE == 0)
                        std::cout << " ";
                }
                std::cout << "\n";
            }
            // Horizontal separator after full acre row
            std::cout << "    +";
            for (int i = 0; i < TOTAL_WIDTH + WIDTH; ++i)
                std::cout << "-";
            std::cout << "+\n";
        }
        // Legend
        std::cout << "\nLegend:\n";
        std::cout << "  . = Ground / Beach (level 0)\n";
        std::cout << "  o = Mid Plateau (level 1)\n";
        std::cout << "  ^ = High Plateau (level 2)\n";
        std::cout << "  = = Mid-level Cliff Face\n";
        std::cout << "  # = High-level Cliff Face\n";
        std::cout << "  W = Water (High Plateau)\n";
        std::cout << "  w = Water (Mid Plateau)\n";
        std::cout << "  ~ = Water (Ground / Beach)\n";
        std::cout << "\n";
    }
}
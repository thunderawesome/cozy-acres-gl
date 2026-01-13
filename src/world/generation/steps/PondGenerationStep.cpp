#include "PondGenerationStep.h"
#include "world/Town.h"
#include "world/data/Acre.h"
#include "world/data/Tile.h"
#include "world/data/TownConfig.h"
#include <random>
#include <cmath>
#include <unordered_set>
#include <queue>
#include <vector>
#include <algorithm>
#include <glm/glm.hpp>

namespace cozy::world
{
    namespace ponds
    {
        struct PairHash
        {
            std::size_t operator()(const glm::ivec2 &v) const
            {
                return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1);
            }
        };

        // Helper to check if a tile is any water type or sand
        bool IsAnyWater(TileType type)
        {
            return type == TileType::RIVER ||
                   type == TileType::RIVER_MOUTH ||
                   type == TileType::OCEAN ||
                   type == TileType::POND;
        }

        // Updated to count all water types and cliffs
        int CountWaterAndCliffTiles(Town &town, int acre_x, int acre_y)
        {
            int count = 0;
            auto &acre = town.GetAcre(acre_x, acre_y);

            for (int z = 0; z < Acre::SIZE; ++z)
            {
                for (int x = 0; x < Acre::SIZE; ++x)
                {
                    auto &tile = acre.tiles[z][x];
                    if (IsAnyWater(tile.type) || tile.type == TileType::CLIFF)
                    {
                        count++;
                    }
                }
            }

            return count;
        }

        std::vector<glm::ivec2> GetBestAcresForPond(Town &town)
        {
            struct AcreScore
            {
                glm::ivec2 pos;
                int water_cliff_count;
                int neighbor_water_cliff_count;
            };

            std::vector<AcreScore> candidates;

            // Don't consider the ocean row (bottom row) for pond placement
            const int max_acre_y = Town::HEIGHT - 1;

            // Score all acres except ocean row
            for (int ay = 0; ay < max_acre_y; ++ay)
            {
                for (int ax = 0; ax < Town::WIDTH; ++ax)
                {
                    AcreScore score;
                    score.pos = {ax, ay};
                    score.water_cliff_count = CountWaterAndCliffTiles(town, ax, ay);
                    score.neighbor_water_cliff_count = 0;

                    // Check neighboring acres (8-directional)
                    for (int dy = -1; dy <= 1; ++dy)
                    {
                        for (int dx = -1; dx <= 1; ++dx)
                        {
                            if (dx == 0 && dy == 0)
                                continue;

                            int nx = ax + dx;
                            int ny = ay + dy;

                            if (nx >= 0 && nx < Town::WIDTH && ny >= 0 && ny < max_acre_y)
                            {
                                score.neighbor_water_cliff_count += CountWaterAndCliffTiles(town, nx, ny);
                            }
                        }
                    }

                    candidates.push_back(score);
                }
            }

            // Sort by: 1) fewest water/cliff tiles in acre, 2) fewest in neighbors
            std::sort(candidates.begin(), candidates.end(),
                      [](const AcreScore &a, const AcreScore &b)
                      {
                          if (a.water_cliff_count != b.water_cliff_count)
                              return a.water_cliff_count < b.water_cliff_count;
                          return a.neighbor_water_cliff_count < b.neighbor_water_cliff_count;
                      });

            // Return sorted list of acre positions
            std::vector<glm::ivec2> result;
            for (const auto &candidate : candidates)
            {
                result.push_back(candidate.pos);
            }
            return result;
        }

        bool CanPlacePond(
            Town &town,
            const glm::ivec2 &center,
            int radius,
            int world_w,
            int world_h)
        {
            // Don't place ponds in the ocean row
            const int ocean_start_z = (Town::HEIGHT - 1) * Acre::SIZE;

            // Check if pond would be too close to boundaries or ocean
            int buffer = radius + 4;
            if (center.x < buffer || center.x >= world_w - buffer ||
                center.y < buffer || center.y >= ocean_start_z - buffer)
            {
                return false;
            }

            // Check if area contains ANY water or cliffs
            int check_radius = radius + 2;
            for (int z = center.y - check_radius; z <= center.y + check_radius; ++z)
            {
                for (int x = center.x - check_radius; x <= center.x + check_radius; ++x)
                {
                    if (x >= 0 && x < world_w && z >= 0 && z < world_h)
                    {
                        auto [a, l] = town.WorldToTile({static_cast<float>(x), 0.f, static_cast<float>(z)});
                        auto &tile = town.GetAcre(a.x, a.y).tiles[l.y][l.x];

                        // Reject if ANY water type or cliff is present
                        if (IsAnyWater(tile.type) || tile.type == TileType::CLIFF)
                        {
                            return false;
                        }
                    }
                }
            }

            return true;
        }

        void Paint4x4Brush(
            Town &town,
            const glm::ivec2 &center,
            int world_w,
            int world_h,
            std::unordered_set<glm::ivec2, PairHash> &painted)
        {
            for (int dz = -2; dz <= 1; ++dz)
            {
                for (int dx = -2; dx <= 1; ++dx)
                {
                    int x = center.x + dx;
                    int z = center.y + dz;

                    if (x >= 0 && x < world_w && z >= 0 && z < world_h)
                    {
                        glm::ivec2 pos{x, z};
                        if (painted.find(pos) == painted.end())
                        {
                            auto [a, l] = town.WorldToTile({static_cast<float>(x), 0.f, static_cast<float>(z)});
                            auto &tile = town.GetAcre(a.x, a.y).tiles[l.y][l.x];

                            // Only paint over grass/empty tiles, never water or cliffs
                            if (!IsAnyWater(tile.type) &&
                                tile.type != TileType::CLIFF &&
                                tile.type != TileType::RAMP)
                            {
                                tile.type = TileType::POND; // Use POND instead of WATER
                                painted.insert(pos);
                            }
                        }
                    }
                }
            }
        }

        void Execute(
            Town &town,
            std::mt19937_64 &rng,
            const TownConfig &config)
        {
            const int world_w = Town::WIDTH * Acre::SIZE;
            const int world_h = Town::HEIGHT * Acre::SIZE;
            const int ocean_start_z = (Town::HEIGHT - 1) * Acre::SIZE;

            // Get sorted list of best acres (excludes ocean row)
            std::vector<glm::ivec2> best_acres = GetBestAcresForPond(town);

            std::uniform_int_distribution<int> radius_dist(config.minPondRadius, config.maxPondRadius);
            std::uniform_int_distribution<int> shape_type(0, 2);
            std::uniform_int_distribution<int> orientation(0, 1);
            std::uniform_real_distribution<float> elongation_dist(1.5f, 2.0f);
            std::uniform_real_distribution<float> bend_dist(0.9f, 1.2f);
            std::uniform_int_distribution<int> offset_dist(-Acre::SIZE / 4, Acre::SIZE / 4);

            int base_radius = radius_dist(rng);
            int shape = shape_type(rng);

            float radius_x = static_cast<float>(base_radius);
            float radius_z = static_cast<float>(base_radius);
            glm::vec2 offset_center{0.0f, 0.0f};

            if (shape == 1) // Egg shape
            {
                float elongation = elongation_dist(rng);
                if (orientation(rng) == 0)
                {
                    radius_x *= elongation;
                }
                else
                {
                    radius_z *= elongation;
                }
            }
            else if (shape == 2) // L-bend
            {
                float bend_amount = bend_dist(rng) * static_cast<float>(base_radius);
                if (orientation(rng) == 0)
                {
                    offset_center = glm::vec2(bend_amount, bend_amount);
                }
                else
                {
                    offset_center = glm::vec2(-bend_amount, bend_amount);
                }
            }

            int max_radius = static_cast<int>(std::ceil(std::max(radius_x, radius_z)));

            // Try acres in order of best to worst
            glm::ivec2 center;
            bool valid_location = false;

            for (const auto &acre_pos : best_acres)
            {
                // Skip if acre is in ocean row (shouldn't happen due to GetBestAcresForPond, but safety check)
                if (acre_pos.y >= Town::HEIGHT - 1)
                    continue;

                int acre_center_x = acre_pos.x * Acre::SIZE + Acre::SIZE / 2;
                int acre_center_y = acre_pos.y * Acre::SIZE + Acre::SIZE / 2;

                // Try multiple positions within this acre
                for (int attempt = 0; attempt < 5; ++attempt)
                {
                    center = {
                        acre_center_x + offset_dist(rng),
                        acre_center_y + offset_dist(rng)};

                    // Make sure we're not in ocean row
                    if (center.y >= ocean_start_z - max_radius - 4)
                        continue;

                    if (CanPlacePond(town, center, max_radius, world_w, world_h))
                    {
                        valid_location = true;
                        break;
                    }
                }

                if (valid_location)
                {
                    break;
                }
            }

            // If still no valid location, try placing at the best acre center
            if (!valid_location)
            {
                if (!best_acres.empty())
                {
                    auto &acre_pos = best_acres[0];

                    // Skip ocean row
                    if (acre_pos.y >= Town::HEIGHT - 1)
                        return;

                    int acre_base_x = acre_pos.x * Acre::SIZE;
                    int acre_base_y = acre_pos.y * Acre::SIZE;

                    // Pick a random corner with 3-tile padding from edges
                    std::uniform_int_distribution<int> corner_choice(0, 3);
                    int corner = corner_choice(rng);

                    switch (corner)
                    {
                    case 0: // Top-left
                        center.x = acre_base_x + 3;
                        center.y = acre_base_y + 3;
                        break;
                    case 1: // Top-right
                        center.x = acre_base_x + Acre::SIZE - 4;
                        center.y = acre_base_y + 3;
                        break;
                    case 2: // Bottom-left
                        center.x = acre_base_x + 3;
                        center.y = acre_base_y + Acre::SIZE - 4;
                        break;
                    case 3: // Bottom-right
                        center.x = acre_base_x + Acre::SIZE - 4;
                        center.y = acre_base_y + Acre::SIZE - 4;
                        break;
                    }

                    // Final safety check - don't place in ocean
                    if (center.y < ocean_start_z - max_radius - 4)
                    {
                        valid_location = true;
                    }
                }

                if (!valid_location)
                {
                    return; // Can't find valid placement
                }
            }

            std::unordered_set<glm::ivec2, PairHash> painted;
            std::queue<glm::ivec2> brush_queue;

            int max_dim = max_radius + 3;

            for (int z = center.y - max_dim; z <= center.y + max_dim; z += 2)
            {
                for (int x = center.x - max_dim; x <= center.x + max_dim; x += 2)
                {
                    bool should_paint = false;

                    if (shape == 2) // L-bend
                    {
                        float dx1 = static_cast<float>(x - center.x);
                        float dz1 = static_cast<float>(z - center.y);
                        float dist1 = std::sqrt(dx1 * dx1 + dz1 * dz1);

                        float dx2 = static_cast<float>(x - center.x) - offset_center.x;
                        float dz2 = static_cast<float>(z - center.y) - offset_center.y;
                        float dist2 = std::sqrt(dx2 * dx2 + dz2 * dz2);

                        should_paint = (dist1 <= radius_x) || (dist2 <= radius_x * 0.8f);
                    }
                    else // Circle or egg
                    {
                        float dx = static_cast<float>(x - center.x) / radius_x;
                        float dz = static_cast<float>(z - center.y) / radius_z;
                        float normalized_dist = std::sqrt(dx * dx + dz * dz);

                        should_paint = normalized_dist <= 1.0f;
                    }

                    if (should_paint)
                    {
                        brush_queue.push({x, z});
                    }
                }
            }

            while (!brush_queue.empty())
            {
                auto pos = brush_queue.front();
                brush_queue.pop();
                Paint4x4Brush(town, pos, world_w, world_h, painted);
            }
        }
    }
}
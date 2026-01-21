#include "PondGenerationStep.h"
#include "world/Town.h"
#include "world/data/Acre.h"
#include "world/data/Tile.h"
#include "world/data/TownConfig.h"
#include "world/generation/utils/WorldGenUtils.h"
#include "world/generation/utils/AutoTileUtils.h"

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
        int CountWaterCliffRampTiles(Town &town, int acre_x, int acre_y)
        {
            int count = 0;
            auto &acre = town.GetAcre(acre_x, acre_y);

            for (int z = 0; z < Acre::SIZE; ++z)
            {
                for (int x = 0; x < Acre::SIZE; ++x)
                {
                    auto &tile = acre.tiles[z][x];
                    if (utils::IsAnyWater(tile.type) ||
                        tile.type == TileType::CLIFF ||
                        tile.type == TileType::RAMP)
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
                int obstacle_count;
                int neighbor_obstacle_count;
            };

            std::vector<AcreScore> candidates;
            const int max_acre_y = Town::HEIGHT - 1;

            for (int ay = 0; ay < max_acre_y; ++ay)
            {
                for (int ax = 0; ax < Town::WIDTH; ++ax)
                {
                    AcreScore score;
                    score.pos = {ax, ay};
                    score.obstacle_count = CountWaterCliffRampTiles(town, ax, ay);
                    score.neighbor_obstacle_count = 0;

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
                                score.neighbor_obstacle_count += CountWaterCliffRampTiles(town, nx, ny);
                            }
                        }
                    }
                    candidates.push_back(score);
                }
            }

            std::sort(candidates.begin(), candidates.end(),
                      [](const AcreScore &a, const AcreScore &b)
                      {
                          if (a.obstacle_count != b.obstacle_count)
                              return a.obstacle_count < b.obstacle_count;
                          return a.neighbor_obstacle_count < b.neighbor_obstacle_count;
                      });

            std::vector<glm::ivec2> result;
            for (const auto &candidate : candidates)
                result.push_back(candidate.pos);
            return result;
        }

        bool CanPlacePond(Town &town, const glm::ivec2 &center, int radius, int world_w, int world_h)
        {
            const int ocean_start_z = (Town::HEIGHT - 1) * Acre::SIZE;
            int buffer = radius + 4;
            if (center.x < buffer || center.x >= world_w - buffer ||
                center.y < buffer || center.y >= ocean_start_z - buffer)
            {
                return false;
            }

            int check_radius = radius + 2;
            for (int z = center.y - check_radius; z <= center.y + check_radius; ++z)
            {
                for (int x = center.x - check_radius; x <= center.x + check_radius; ++x)
                {
                    TileType type = utils::GetTileTypeSafe(town, x, z);
                    if (utils::IsAnyWater(type) || type == TileType::CLIFF || type == TileType::RAMP)
                    {
                        return false;
                    }
                }
            }
            return true;
        }

        void Paint4x4Brush(Town &town, const glm::ivec2 &brush_center, int world_w, int world_h, std::unordered_set<glm::ivec2, utils::PairHash> &painted)
        {
            for (int dz = -2; dz <= 1; ++dz)
            {
                for (int dx = -2; dx <= 1; ++dx)
                {
                    int x = brush_center.x + dx;
                    int z = brush_center.y + dz;

                    if (x >= 0 && x < world_w && z >= 0 && z < world_h)
                    {
                        glm::ivec2 pos{x, z};
                        auto [a, l] = utils::GetTileCoords(x, z);
                        auto &tile = town.GetAcre(a.x, a.y).tiles[l.y][l.x];

                        if (!utils::IsAnyWater(tile.type) && tile.type != TileType::CLIFF && tile.type != TileType::RAMP)
                        {
                            tile.type = TileType::POND;
                            painted.insert(pos);
                        }
                    }
                }
            }
        }

        void Execute(Town &town, std::mt19937_64 &rng, const TownConfig &config)
        {
            const int world_w = Town::WIDTH * Acre::SIZE;
            const int world_h = Town::HEIGHT * Acre::SIZE;
            const int ocean_start_z = (Town::HEIGHT - 1) * Acre::SIZE;

            std::vector<glm::ivec2> best_acres = GetBestAcresForPond(town);

            // 1. Setup Radius and Shape
            std::uniform_int_distribution<int> radius_dist(config.minPondRadius, config.maxPondRadius);
            std::uniform_int_distribution<int> shape_type(0, 2);
            std::uniform_int_distribution<int> orientation(0, 1);
            std::uniform_real_distribution<float> elongation_dist(1.2f, 1.8f);
            std::uniform_int_distribution<int> offset_dist(-Acre::SIZE / 2, Acre::SIZE / 2);

            int base_radius = radius_dist(rng);
            int shape = shape_type(rng);
            float radius_x = static_cast<float>(base_radius);
            float radius_z = static_cast<float>(base_radius);

            if (shape == 1)
            {
                float elongation = elongation_dist(rng);
                if (orientation(rng) == 0)
                    radius_x *= elongation;
                else
                    radius_z *= elongation;
            }

            int max_radius = static_cast<int>(std::ceil(std::max(radius_x, radius_z)));
            glm::ivec2 center;
            int target_elevation = 0;
            bool valid_location = false;

            // 2. Aggressive Search
            for (const auto &acre_pos : best_acres)
            {
                int acre_center_x = acre_pos.x * Acre::SIZE + Acre::SIZE / 2;
                int acre_center_y = acre_pos.y * Acre::SIZE + Acre::SIZE / 2;

                for (int attempt = 0; attempt < 20; ++attempt)
                {
                    center = {acre_center_x + offset_dist(rng), acre_center_y + offset_dist(rng)};

                    // Ensure center is within bounds
                    if (center.x < max_radius || center.x >= world_w - max_radius ||
                        center.y < max_radius || center.y >= ocean_start_z - max_radius)
                        continue;

                    // Check if center tile is actually grass
                    auto [ca, cl] = utils::GetTileCoords(center.x, center.y);
                    auto &center_tile = town.GetAcre(ca.x, ca.y).tiles[cl.y][cl.x];

                    if (center_tile.type == TileType::GRASS)
                    {
                        // Relaxed check: Only check collision with cliffs/water within the radius
                        if (CanPlacePond(town, center, max_radius, world_w, world_h))
                        {
                            target_elevation = center_tile.elevation;
                            valid_location = true;
                            break;
                        }
                    }
                }
                if (valid_location)
                    break;
            }

            if (!valid_location)
                return;

            // 3. Paint Pass
            std::unordered_set<glm::ivec2, utils::PairHash> painted;
            int seed = static_cast<int>(rng());

            for (int wz = center.y - max_radius - config.pondMargin; wz <= center.y + max_radius + config.pondMargin; ++wz)
            {
                for (int wx = center.x - max_radius - config.pondMargin; wx <= center.x + max_radius + config.pondMargin; ++wx)
                {
                    if (wx < 0 || wx >= world_w || wz < 0 || wz >= ocean_start_z)
                        continue;

                    float noiseX = utils::SmoothNoise(wx * config.pondNoiseScale, wz * config.pondNoiseScale, seed) * config.pondNoiseStrength;
                    float noiseZ = utils::SmoothNoise(wx * config.pondNoiseScale, wz * config.pondNoiseScale, seed + 7919) * config.pondNoiseStrength;

                    float dx = (static_cast<float>(wx) + noiseX - center.x) / radius_x;
                    float dz = (static_cast<float>(wz) + noiseZ - center.y) / radius_z;

                    if (dx * dx + dz * dz <= 1.0f)
                    {
                        auto [a, l] = utils::GetTileCoords(wx, wz);
                        auto &tile = town.GetAcre(a.x, a.y).tiles[l.y][l.x];

                        // Only paint if same elevation and is grass
                        if (tile.type == TileType::GRASS && tile.elevation == target_elevation)
                        {
                            tile.type = TileType::POND;
                            painted.insert({wx, wz});
                        }
                    }
                }
            }

            // 4. Cleanup (Capped Neighbors)
            int effective_min = std::min(config.pondMinNeighbors, 2);
            std::vector<glm::ivec2> to_revert;
            for (const auto &pos : painted)
            {
                int neighbors = 0;
                for (int dz = -1; dz <= 1; ++dz)
                {
                    for (int dx = -1; dx <= 1; ++dx)
                    {
                        if (dx == 0 && dz == 0)
                            continue;
                        if (utils::IsAnyWater(utils::GetTileTypeSafe(town, pos.x + dx, pos.y + dz)))
                            neighbors++;
                    }
                }
                if (neighbors < effective_min)
                    to_revert.push_back(pos);
            }

            for (const auto &pos : to_revert)
            {
                auto [a, l] = utils::GetTileCoords(pos.x, pos.y);
                town.GetAcre(a.x, a.y).tiles[l.y][l.x].type = TileType::GRASS;
                painted.erase(pos);
            }

            // 5. Autotile Pass
            for (const auto &pos : painted)
            {
                auto [a, l] = utils::GetTileCoords(pos.x, pos.y);
                auto &tile = town.GetAcre(a.x, a.y).tiles[l.y][l.x];
                if (tile.type == TileType::POND)
                {
                    tile.autotileIndex = autotile::CalculatePondBlobIndex(town, pos.x, pos.y);
                }
            }
        }
    }
}
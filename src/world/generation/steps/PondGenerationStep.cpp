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

            std::uniform_int_distribution<int> radius_dist(config.minPondRadius, config.maxPondRadius);
            std::uniform_int_distribution<int> shape_type(0, 2);
            std::uniform_int_distribution<int> orientation(0, 1);
            std::uniform_real_distribution<float> elongation_dist(1.5f, 2.0f);
            std::uniform_real_distribution<float> bend_dist(0.9f, 1.2f);
            std::uniform_int_distribution<int> offset_dist(-Acre::SIZE / 4, Acre::SIZE / 4);

            int base_radius = radius_dist(rng);
            int shape = shape_type(rng);
            float radius_x = (float)base_radius;
            float radius_z = (float)base_radius;
            glm::vec2 offset_center{0.0f, 0.0f};

            if (shape == 1)
            { // Egg
                float elongation = elongation_dist(rng);
                if (orientation(rng) == 0)
                    radius_x *= elongation;
                else
                    radius_z *= elongation;
            }
            else if (shape == 2)
            { // Bend
                float bend_amount = bend_dist(rng) * (float)base_radius;
                offset_center = (orientation(rng) == 0) ? glm::vec2(bend_amount, bend_amount) : glm::vec2(-bend_amount, bend_amount);
            }

            int max_radius = static_cast<int>(std::ceil(std::max(radius_x, radius_z)));
            glm::ivec2 center;
            bool valid_location = false;

            for (const auto &acre_pos : best_acres)
            {
                int acre_center_x = acre_pos.x * Acre::SIZE + Acre::SIZE / 2;
                int acre_center_y = acre_pos.y * Acre::SIZE + Acre::SIZE / 2;

                for (int attempt = 0; attempt < 5; ++attempt)
                {
                    center = {acre_center_x + offset_dist(rng), acre_center_y + offset_dist(rng)};
                    if (center.y < ocean_start_z - max_radius - 4 && CanPlacePond(town, center, max_radius, world_w, world_h))
                    {
                        valid_location = true;
                        break;
                    }
                }
                if (valid_location)
                    break;
            }

            if (!valid_location)
                return;

            std::unordered_set<glm::ivec2, utils::PairHash> painted;
            std::queue<glm::ivec2> brush_queue;
            int max_dim = max_radius + 3;

            for (int z = center.y - max_dim; z <= center.y + max_dim; z += 2)
            {
                for (int x = center.x - max_dim; x <= center.x + max_dim; x += 2)
                {
                    bool should_paint = false;
                    if (shape == 2)
                    {
                        float d1 = glm::distance(glm::vec2(x, z), glm::vec2(center));
                        float d2 = glm::distance(glm::vec2(x, z), glm::vec2(center) + offset_center);
                        should_paint = (d1 <= radius_x) || (d2 <= radius_x * 0.8f);
                    }
                    else
                    {
                        float dx = (x - center.x) / radius_x;
                        float dz = (z - center.y) / radius_z;
                        should_paint = (dx * dx + dz * dz) <= 1.0f;
                    }
                    if (should_paint)
                        brush_queue.push({x, z});
                }
            }

            // --- 1. Paint the Pond ---
            while (!brush_queue.empty())
            {
                auto pos = brush_queue.front();
                brush_queue.pop();
                Paint4x4Brush(town, pos, world_w, world_h, painted);
            }

            for (const auto &pos : painted)
            {
                auto [a, l] = utils::GetTileCoords(pos.x, pos.y);
                auto &tile = town.GetAcre(a.x, a.y).tiles[l.y][l.x];

                if (tile.type == TileType::POND)
                {
                    // Now storing a clean 0-46 index
                    tile.autotileIndex = autotile::CalculatePondBlobIndex(town, pos.x, pos.y);
                }
            }
        }
    }
}
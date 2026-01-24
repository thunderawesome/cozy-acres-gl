#include "PondGenerationStep.h"
#include "world/Town.h"
#include "world/data/Acre.h"
#include "world/data/Tile.h"
#include "world/data/TownConfig.h"
#include "world/generation/utils/WorldGenUtils.h"
#include "world/generation/utils/AutoTileUtils.h"

#include <unordered_set>
#include <algorithm>

namespace cozy::world::ponds
{
    bool PondBlob::IsPointInside(int wx, int wz, const TownConfig &config) const
    {
        float nX = utils::SmoothNoise(wx * config.pondNoiseScale, wz * config.pondNoiseScale, seed) * config.pondNoiseStrength;
        float nZ = utils::SmoothNoise(wx * config.pondNoiseScale, wz * config.pondNoiseScale, seed + 7919) * config.pondNoiseStrength;

        float dx = (static_cast<float>(wx) + nX - center.x) / radius_x;
        float dz = (static_cast<float>(wz) + nZ - center.y) / radius_z;

        return (dx * dx + dz * dz <= 1.0f);
    }

    // PondGenerationStep.cpp

    bool IsAreaClearForPond(Town &town, glm::ivec2 center, int max_radius, const TownConfig &config)
    {
        const int world_w = Town::WIDTH * Acre::SIZE;
        // Ponds should stay above the beach/ocean row (Row F)
        const int ocean_limit_z = (Town::HEIGHT - 1) * Acre::SIZE;

        // 1. Calculate the "Wobble Buffer"
        // We take the max radius and add the max potential noise displacement
        // plus a small hard margin to ensure no part of the pond hits an edge.
        int noise_buffer = static_cast<int>(std::ceil(config.pondNoiseStrength)) + 2;
        int safe_radius = max_radius + noise_buffer;

        // 2. Map Boundary Check (Using safe_radius)
        // This prevents the "blob" from leaking outside the array bounds or into the ocean
        if (center.x < safe_radius || center.x >= world_w - safe_radius ||
            center.y < safe_radius || center.y >= ocean_limit_z - safe_radius)
        {
            return false;
        }

        // 3. Elevation/Validity Check
        // Using Town's internal logic to ensure the center is on valid terrain
        if (utils::GetElevation(town, center.x, center.y) == -1)
        {
            return false;
        }

        // 4. Obstacle/Collision Check
        // We scan slightly wider (using pondMargin) to ensure we don't
        // touch cliffs, ramps, or existing river water.
        int check_r = max_radius + config.pondMargin;
        for (int z = center.y - check_r; z <= center.y + check_r; ++z)
        {
            for (int x = center.x - check_r; x <= center.x + check_r; ++x)
            {
                TileType type = utils::GetTileTypeSafe(town, x, z);

                // Ponds should only spawn on grass and avoid existing features
                if (utils::IsAnyWater(type) ||
                    type == TileType::CLIFF ||
                    type == TileType::RAMP)
                {
                    return false;
                }
            }
        }

        return true;
    }

    void Execute(Town &town, std::mt19937_64 &rng, const TownConfig &config)
    {
        const int world_w = utils::GetWorldWidth();
        const int ocean_limit_z = (Town::HEIGHT - 1) * Acre::SIZE;

        bool placed = false;
        PondBlob pond;

        // 1. HIGH-LEVEL RETRY LOOP
        // If we can't find a spot for the current pond size, we try again
        // with a potentially different size/shape up to 5 times.
        for (int retry = 0; retry < 5; ++retry)
        {
            pond.seed = static_cast<int>(rng());

            std::uniform_int_distribution<int> r_dist(config.minPondRadius, config.maxPondRadius);
            int base_r = r_dist(rng);
            pond.radius_x = static_cast<float>(base_r);
            pond.radius_z = static_cast<float>(base_r);

            if (std::uniform_int_distribution<>(0, 100)(rng) < 33)
            {
                float elongation = std::uniform_real_distribution<float>(1.2f, 1.6f)(rng);
                if (std::uniform_int_distribution<>(0, 1)(rng) == 0)
                    pond.radius_x *= elongation;
                else
                    pond.radius_z *= elongation;
            }

            int max_reach = static_cast<int>(std::ceil(std::max(pond.radius_x, pond.radius_z)));
            int search_padding = max_reach + static_cast<int>(config.pondNoiseStrength) + 1;

            // 2. Search Strategy (Shuffled Acres)
            std::vector<glm::ivec2> acre_indices;
            for (int y = 0; y < Town::HEIGHT - 1; ++y)
                for (int x = 0; x < Town::WIDTH; ++x)
                    acre_indices.push_back({x, y});

            std::shuffle(acre_indices.begin(), acre_indices.end(), rng);

            for (auto &acre_pos : acre_indices)
            {
                for (int attempt = 0; attempt < 15; ++attempt) // Increased local attempts
                {
                    pond.center.x = acre_pos.x * Acre::SIZE + std::uniform_int_distribution<>(2, Acre::SIZE - 3)(rng);
                    pond.center.y = acre_pos.y * Acre::SIZE + std::uniform_int_distribution<>(2, Acre::SIZE - 3)(rng);

                    if (pond.center.x < search_padding || pond.center.x >= world_w - search_padding ||
                        pond.center.y < search_padding || pond.center.y >= ocean_limit_z - search_padding)
                        continue;

                    if (auto *t = utils::GetTileSafe(town, pond.center.x, pond.center.y))
                    {
                        if (t->type == TileType::GRASS && IsAreaClearForPond(town, pond.center, max_reach, config))
                        {
                            pond.target_elevation = t->elevation;
                            placed = true;
                            break;
                        }
                    }
                }
                if (placed)
                    break;
            }

            if (placed)
                break; // Exit retry loop early if we found a spot
        }

        if (!placed)
            return;

        // 3. Paint Pass (Logic remains the same as previous version)
        int max_reach = static_cast<int>(std::ceil(std::max(pond.radius_x, pond.radius_z)));
        std::unordered_set<glm::ivec2, utils::PairHash> painted;
        int scan_r = max_reach + config.pondMargin;

        for (int wz = pond.center.y - scan_r; wz <= pond.center.y + scan_r; ++wz)
        {
            for (int wx = pond.center.x - scan_r; wx <= pond.center.x + scan_r; ++wx)
            {
                if (wz >= ocean_limit_z)
                    continue;

                if (pond.IsPointInside(wx, wz, config))
                {
                    if (auto *tile = utils::GetTileSafe(town, wx, wz))
                    {
                        if (tile->type == TileType::GRASS && tile->elevation == pond.target_elevation)
                        {
                            tile->type = TileType::POND;
                            painted.insert({wx, wz});
                        }
                    }
                }
            }
        }

        // 4. Cleanup & Autotiling
        // ... (Remaining cleanup and autotile logic remains the same)
        std::vector<glm::ivec2> to_revert;
        for (const auto &pos : painted)
        {
            int count = 0;
            for (auto &n : utils::GetNeighbors8(pos.x, pos.y))
            {
                if (utils::IsAnyWater(utils::GetTileTypeSafe(town, n.x, n.y)))
                    count++;
            }
            if (count < config.pondMinNeighbors)
                to_revert.push_back(pos);
        }

        for (auto &p : to_revert)
        {
            if (auto *t = utils::GetTileSafe(town, p.x, p.y))
                t->type = TileType::GRASS;
            painted.erase(p);
        }

        for (const auto &pos : painted)
        {
            if (auto *t = utils::GetTileSafe(town, pos.x, pos.y))
            {
                t->autotileIndex = static_cast<uint8_t>(utils::CalculatePondBlobIndex(town, pos.x, pos.y));
            }
        }
    }
}
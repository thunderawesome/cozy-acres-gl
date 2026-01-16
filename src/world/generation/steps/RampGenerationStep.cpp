#include "RampGenerationStep.h"

#include "world/Town.h"
#include "world/data/Acre.h"
#include "world/data/Tile.h"
#include "world/data/TownConfig.h"
#include "world/generation/utils/WorldGenUtils.h"

#include <vector>
#include <algorithm>
#include <random>
#include <glm/glm.hpp>

namespace cozy::world
{
    namespace ramps
    {
        namespace
        {
            struct RampCandidate
            {
                int world_x;
                int world_z;
                int score; // Higher is better
            };

            // Tile is walkable if it's NOT water (cliffs are OK, they get cleared later)
            bool IsWalkable(const Tile &tile)
            {
                return !utils::IsAnyWater(tile.type);
            }

            // Check for water nearby with adequate clearance for full ramp footprint
            bool IsWaterNearby(
                const Town &town,
                int wx,
                int wz,
                const TownConfig &config)
            {
                const int w = Town::WIDTH * Acre::SIZE;
                const int h = Town::HEIGHT * Acre::SIZE;

                // Check area covering full ramp plus buffer
                int min_x = wx - TownConfig::RAMP_CORRIDOR_HALF_WIDTH - config.rampWaterClearance;
                int max_x = wx + TownConfig::RAMP_CORRIDOR_HALF_WIDTH + config.rampWaterClearance;
                int min_z = wz - config.rampWaterClearance;
                int max_z = wz + TownConfig::RAMP_LENGTH + config.rampWaterClearance;

                for (int cz = min_z; cz <= max_z; ++cz)
                {
                    for (int cx = min_x; cx <= max_x; ++cx)
                    {
                        if (cx < 0 || cx >= w || cz < 0 || cz >= h)
                            continue;

                        auto [a, l] = town.WorldToTile({float(cx), 0.f, float(cz)});
                        const Tile &tile = town.GetAcre(a.x, a.y).tiles[l.y][l.x];

                        if (utils::IsAnyWater(tile.type))
                            return true;
                    }
                }
                return false;
            }

            bool HasCliffWall(
                const Town &town,
                int wx,
                int wz,
                int dx,
                int elevation)
            {
                auto [a, l] = town.WorldToTile({float(wx + dx), 0.f, float(wz)});
                const auto &tile = town.GetAcre(a.x, a.y).tiles[l.y][l.x];
                return tile.type == TileType::CLIFF &&
                       tile.elevation == elevation;
            }

            bool CanPlaceRampHere(
                const Town &town,
                int wx,
                int wz,
                int from_elevation,
                int to_elevation)
            {
                auto [a0, l0] = town.WorldToTile({float(wx), 0.f, float(wz)});
                auto [a1, l1] = town.WorldToTile({float(wx), 0.f, float(wz + 1)});

                const Tile &upper = town.GetAcre(a0.x, a0.y).tiles[l0.y][l0.x];
                const Tile &lower = town.GetAcre(a1.x, a1.y).tiles[l1.y][l1.x];

                if (upper.elevation != from_elevation)
                    return false;

                if (lower.elevation != to_elevation)
                    return false;

                if (from_elevation != to_elevation + 1)
                    return false;

                if (!IsWalkable(upper) || !IsWalkable(lower))
                    return false;

                return true;
            }

            std::vector<RampCandidate> FindRampCandidates(
                const Town &town,
                int from_elevation,
                int to_elevation,
                int min_x,
                int max_x,
                const TownConfig &config)
            {
                const int h = Town::HEIGHT * Acre::SIZE;
                std::vector<RampCandidate> candidates;

                for (int wz = 1; wz < h - 1; ++wz)
                {
                    for (int wx = min_x; wx < max_x; ++wx)
                    {
                        if (!CanPlaceRampHere(town, wx, wz, from_elevation, to_elevation))
                            continue;

                        if (IsWaterNearby(town, wx, wz, config))
                            continue;

                        RampCandidate c;
                        c.world_x = wx;
                        c.world_z = wz;

                        int dist_left = wx - min_x;
                        int dist_right = max_x - wx;
                        c.score = std::min(dist_left, dist_right);

                        bool west_wall = HasCliffWall(town, wx - 1, wz, -1, from_elevation);
                        bool east_wall = HasCliffWall(town, wx + 1, wz, 1, from_elevation);

                        if (!west_wall || !east_wall)
                            c.score -= 5;

                        candidates.push_back(c);
                    }
                }

                return candidates;
            }

            void NormalizeRampSides(
                Town &town,
                int center_x,
                int center_z,
                int ramp_length,
                int from_elevation)
            {
                const int w = Town::WIDTH * Acre::SIZE;
                const int h = Town::HEIGHT * Acre::SIZE;

                for (int z = 0; z < ramp_length; ++z)
                {
                    int wz = center_z + z;
                    if (wz < 0 || wz >= h)
                        continue;

                    for (int x = -TownConfig::RAMP_NORMALIZE_HALF_WIDTH; x <= TownConfig::RAMP_NORMALIZE_HALF_WIDTH; ++x)
                    {
                        int wx = center_x + x;
                        if (wx < 0 || wx >= w)
                            continue;

                        auto [a, l] = town.WorldToTile({float(wx), 0.f, float(wz)});
                        auto &tile = town.GetAcre(a.x, a.y).tiles[l.y][l.x];

                        if (!IsWalkable(tile))
                            continue;

                        tile.elevation = from_elevation;
                        tile.type = TileType::GRASS;
                    }
                }
            }

            void ClearCliffsInRampCorridor(
                Town &town,
                int center_x,
                int center_z,
                int ramp_length)
            {
                const int w = Town::WIDTH * Acre::SIZE;
                const int h = Town::HEIGHT * Acre::SIZE;

                for (int z = 0; z < ramp_length; ++z)
                {
                    int wz = center_z + z;
                    if (wz < 0 || wz >= h)
                        continue;

                    for (int x = -TownConfig::RAMP_CORRIDOR_HALF_WIDTH; x <= TownConfig::RAMP_CORRIDOR_HALF_WIDTH; ++x)
                    {
                        int wx = center_x + x;
                        if (wx < 0 || wx >= w)
                            continue;

                        auto [a, l] = town.WorldToTile({float(wx), 0.f, float(wz)});
                        auto &tile = town.GetAcre(a.x, a.y).tiles[l.y][l.x];

                        if (tile.type == TileType::CLIFF)
                            tile.type = TileType::GRASS;
                    }
                }
            }

            void CarveRamp(
                Town &town,
                int center_x,
                int center_z,
                int from_elevation,
                int ramp_length)
            {
                ClearCliffsInRampCorridor(town, center_x, center_z, ramp_length);
                NormalizeRampSides(town, center_x, center_z, ramp_length, from_elevation);

                const int w = Town::WIDTH * Acre::SIZE;
                const int h = Town::HEIGHT * Acre::SIZE;

                for (int z = 0; z < ramp_length; ++z)
                {
                    int wz = center_z + z;
                    if (wz < 0 || wz >= h)
                        break;

                    auto [a_exit, l_exit] =
                        town.WorldToTile({float(center_x), 0.f, float(wz + 1)});

                    const Tile &exit_tile =
                        town.GetAcre(a_exit.x, a_exit.y).tiles[l_exit.y][l_exit.x];

                    if (!IsWalkable(exit_tile))
                        break;

                    for (int x = -1; x <= 1; ++x)
                    {
                        int wx = center_x + x;
                        if (wx < 0 || wx >= w)
                            continue;

                        auto [a, l] = town.WorldToTile({float(wx), 0.f, float(wz)});
                        auto &tile = town.GetAcre(a.x, a.y).tiles[l.y][l.x];

                        if (!IsWalkable(tile))
                            continue;

                        tile.type = TileType::RAMP;
                    }
                }
            }

            void PlaceRampsForTier(
                Town &town,
                std::mt19937_64 &rng,
                int from_elevation,
                int to_elevation,
                const TownConfig &config)
            {
                const int w = Town::WIDTH * Acre::SIZE;
                const int mid_x = w / 2;

                auto west = FindRampCandidates(town, from_elevation, to_elevation, 0, mid_x, config);
                auto east = FindRampCandidates(town, from_elevation, to_elevation, mid_x, w, config);

                auto by_score = [](auto &a, auto &b)
                { return a.score > b.score; };
                std::sort(west.begin(), west.end(), by_score);
                std::sort(east.begin(), east.end(), by_score);

                auto place = [&](std::vector<RampCandidate> &candidates)
                {
                    if (candidates.empty())
                        return;

                    int limit = std::min(config.rampTopCandidates, (int)candidates.size());
                    std::uniform_int_distribution<int> pick(0, limit - 1);
                    auto &c = candidates[pick(rng)];

                    CarveRamp(town, c.world_x, c.world_z, from_elevation, TownConfig::RAMP_LENGTH);
                };

                place(west);
                place(east);
            }
        }

        void Execute(
            Town &town,
            std::mt19937_64 &rng,
            const TownConfig &config)
        {
            const int w = Town::WIDTH * Acre::SIZE;
            const int h = Town::HEIGHT * Acre::SIZE;

            int8_t max_elevation = 0;
            for (int z = 0; z < h; ++z)
            {
                for (int x = 0; x < w; ++x)
                {
                    auto [a, l] = town.WorldToTile({float(x), 0.f, float(z)});
                    max_elevation = std::max(
                        max_elevation,
                        town.GetAcre(a.x, a.y).tiles[l.y][l.x].elevation);
                }
            }

            if (max_elevation >= 1)
                PlaceRampsForTier(town, rng, 1, 0, config);

            if (max_elevation >= 2)
                PlaceRampsForTier(town, rng, 2, 1, config);
        }
    }
}
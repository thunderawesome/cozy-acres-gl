#include "RampGenerationStep.h"

#include "world/Town.h"
#include "world/data/Acre.h"
#include "world/data/Tile.h"
#include "world/data/TownConfig.h"
#include "world/generation/utils/WorldGenUtils.h"

#include <vector>
#include <algorithm>
#include <random>
#include <cmath>
#include <limits>
#include <glm/glm.hpp>

namespace cozy::world
{
    namespace ramps
    {
        namespace
        {
            struct RampCandidate
            {
                int x;              // World X coordinate (center of ramp)
                int z_top;          // Z coordinate at the top (on higher plateau)
                int z_bottom;       // Z coordinate at the bottom (on lower plateau)
                int from_elevation; // Elevation we're ramping from
                int to_elevation;   // Elevation we're ramping to
                float score;        // Quality score for this ramp location
            };

            // Helper: Find the approximate X center of the river at a specific Z depth
            // This ensures we determine "West" vs "East" based on the actual river flow.
            int GetRiverCenterX(const Town &town, int z)
            {
                const int w = utils::GetWorldWidth();
                long total_x = 0;
                int count = 0;

                // Scan a small band around the target Z to find river tiles
                // We expand the search slightly in case the cliff is right on a bend
                for (int dz = -2; dz <= 2; ++dz)
                {
                    int scan_z = std::clamp(z + dz, 0, (Town::HEIGHT * Acre::SIZE) - 1);
                    for (int x = 0; x < w; ++x)
                    {
                        auto [a, l] = utils::GetTileCoords(x, scan_z);
                        const Tile &t = town.GetAcre(a.x, a.y).tiles[l.y][l.x];

                        if (t.type == TileType::RIVER || t.type == TileType::WATERFALL)
                        {
                            total_x += x;
                            count++;
                        }
                    }
                }

                if (count == 0)
                    return w / 2; // Default to map center if no river found locally
                return static_cast<int>(total_x / count);
            }

            // Find cliff edges going from higher to lower elevation (NORTH to SOUTH)
            std::vector<glm::ivec2> FindCliffEdges(const Town &town, int from_elev, int to_elev)
            {
                std::vector<glm::ivec2> edges;
                const int w = Town::WIDTH * Acre::SIZE;
                const int h = Town::HEIGHT * Acre::SIZE;

                for (int z = 0; z < h - 1; ++z)
                {
                    for (int x = 0; x < w; ++x)
                    {
                        int curr_elev = utils::GetElevation(town, x, z);
                        int next_elev = utils::GetElevation(town, x, z + 1);

                        // Look for transition from higher to lower going south (increasing z)
                        if (curr_elev == from_elev && next_elev == to_elev)
                        {
                            edges.push_back({x, z});
                        }
                    }
                }
                return edges;
            }

            // Score a potential ramp location based on surrounding terrain
            float ScoreRampLocation(
                const Town &town,
                int x,
                int z_cliff,
                int from_elev,
                int to_elev,
                const TownConfig &config)
            {
                float score = 100.0f;
                const int w = Town::WIDTH * Acre::SIZE;
                const int h = Town::HEIGHT * Acre::SIZE;

                // RULE: Must be at least 3 tiles from left/right bounds
                if (x - TownConfig::RAMP_CORRIDOR_HALF_WIDTH < config.rampBuffer ||
                    x + TownConfig::RAMP_CORRIDOR_HALF_WIDTH >= w - config.rampBuffer)
                    return 0.0f;

                // Calculate ramp extents - start ABOVE the cliff edge
                int z_start = z_cliff - (TownConfig::RAMP_LENGTH / 2);
                int z_end = z_cliff + (TownConfig::RAMP_LENGTH / 2);

                if (z_start < 0 || z_end >= h)
                    return 0.0f;

                // --- FLUSHNESS CHECK ---
                for (int dx = -1; dx <= 1; ++dx)
                {
                    int check_x = x + dx;
                    if (check_x < 0 || check_x >= w)
                        return 0.0f;

                    if (utils::GetElevation(town, check_x, z_cliff) != from_elev ||
                        utils::GetElevation(town, check_x, z_cliff + 1) != to_elev)
                    {
                        return 0.0f;
                    }
                }

                // Check the ramp corridor for obstacles
                for (int dx = -TownConfig::RAMP_CORRIDOR_HALF_WIDTH; dx <= TownConfig::RAMP_CORRIDOR_HALF_WIDTH; ++dx)
                {
                    int rx = x + dx;
                    if (rx < 0 || rx >= w)
                    {
                        score -= 20.0f;
                        continue;
                    }

                    for (int rz = z_start; rz <= z_end; ++rz)
                    {
                        if (rz < 0 || rz >= h)
                            continue;

                        int elev = utils::GetElevation(town, rx, rz);

                        if (rz < z_cliff && elev != from_elev)
                            score -= 15.0f;
                        else if (rz > z_cliff && elev != to_elev)
                            score -= 15.0f;

                        auto [a, l] = utils::GetTileCoords(rx, rz);
                        const Tile &tile = town.GetAcre(a.x, a.y).tiles[l.y][l.x];

                        if (tile.type == TileType::RIVER ||
                            tile.type == TileType::WATERFALL ||
                            tile.type == TileType::RIVER_MOUTH ||
                            tile.type == TileType::OCEAN ||
                            tile.type == TileType::SAND ||
                            tile.type == TileType::POND)
                        {
                            return 0.0f;
                        }
                    }
                }

                // RULE: Water Clearance
                for (int dx = -TownConfig::RAMP_CORRIDOR_HALF_WIDTH - config.rampBuffer;
                     dx <= TownConfig::RAMP_CORRIDOR_HALF_WIDTH + config.rampBuffer; ++dx)
                {
                    int rx = x + dx;
                    if (rx < 0 || rx >= w)
                        continue;

                    for (int rz = z_start - config.rampBuffer; rz <= z_end + config.rampBuffer; ++rz)
                    {
                        if (rz < 0 || rz >= h)
                            continue;

                        auto [a, l] = utils::GetTileCoords(rx, rz);
                        const Tile &tile = town.GetAcre(a.x, a.y).tiles[l.y][l.x];

                        // Check against all water types defined in your legend
                        if (tile.type == TileType::RIVER ||
                            tile.type == TileType::WATERFALL ||
                            tile.type == TileType::RIVER_MOUTH ||
                            tile.type == TileType::OCEAN ||
                            tile.type == TileType::POND)
                        {
                            return 0.0f;
                        }
                    }
                }

                return std::max(0.0f, score);
            }

            std::vector<RampCandidate> FindRampCandidates(
                const Town &town,
                int from_elev,
                int to_elev,
                const TownConfig &config)
            {
                std::vector<RampCandidate> candidates;
                auto cliff_edges = FindCliffEdges(town, from_elev, to_elev);

                if (cliff_edges.empty())
                    return candidates;

                for (const auto &edge : cliff_edges)
                {
                    float score = ScoreRampLocation(town, edge.x, edge.y, from_elev, to_elev, config);

                    if (score > 0.0f)
                    {
                        RampCandidate candidate;
                        candidate.x = edge.x;
                        candidate.z_top = edge.y - (TownConfig::RAMP_LENGTH / 2);
                        candidate.z_bottom = edge.y + (TownConfig::RAMP_LENGTH / 2);
                        candidate.from_elevation = from_elev;
                        candidate.to_elevation = to_elev;
                        candidate.score = score;
                        candidates.push_back(candidate);
                    }
                }

                // Initial sort by score
                std::sort(candidates.begin(), candidates.end(),
                          [](const RampCandidate &a, const RampCandidate &b)
                          { return a.score > b.score; });

                return candidates;
            }

            bool ConflictsWithExistingRamps(
                const RampCandidate &candidate,
                const std::vector<RampCandidate> &placed_ramps)
            {
                const int w = Town::WIDTH * Acre::SIZE;
                const int min_horizontal_distance = w / 4;

                for (const auto &existing : placed_ramps)
                {
                    int x_dist = std::abs(candidate.x - existing.x);

                    bool z_overlap = !(candidate.z_bottom < existing.z_top ||
                                       candidate.z_top > existing.z_bottom);

                    if (existing.from_elevation == candidate.from_elevation &&
                        existing.to_elevation == candidate.to_elevation)
                    {
                        if (z_overlap && x_dist < TownConfig::RAMP_CORRIDOR_HALF_WIDTH * 4)
                            return true;
                        if (x_dist < min_horizontal_distance)
                            return true;
                    }
                    else
                    {
                        if (z_overlap && x_dist < TownConfig::RAMP_CORRIDOR_HALF_WIDTH * 3)
                            return true;

                        // Overlap in Z but different elevations needs vertical clearance
                        if (x_dist < TownConfig::RAMP_CORRIDOR_HALF_WIDTH * 2 &&
                            std::abs(candidate.z_top - existing.z_top) < TownConfig::RAMP_LENGTH * 2)
                            return true;
                    }
                }
                return false;
            }

            void NormalizeRampSides(Town &town, const RampCandidate &ramp)
            {
                const int w = Town::WIDTH * Acre::SIZE;
                const int h = Town::HEIGHT * Acre::SIZE;

                for (int z = ramp.z_top; z <= ramp.z_bottom; ++z)
                {
                    if (z < 0 || z >= h)
                        continue;

                    float t = static_cast<float>(z - ramp.z_top) /
                              static_cast<float>(ramp.z_bottom - ramp.z_top);
                    t = std::clamp(t, 0.0f, 1.0f);
                    t = utils::SmoothStep(t);

                    int target_elev = (t > 0.5f) ? ramp.to_elevation : ramp.from_elevation;

                    auto ProcessSide = [&](int start_dx, int end_dx)
                    {
                        for (int dx = start_dx; dx <= end_dx; ++dx)
                        {
                            int x = ramp.x + dx;
                            if (x < 0 || x >= w)
                                continue;
                            auto [a, l] = utils::GetTileCoords(x, z);
                            Tile &tile = town.GetAcre(a.x, a.y).tiles[l.y][l.x];
                            if (tile.type != TileType::RIVER && tile.type != TileType::WATERFALL &&
                                tile.type != TileType::OCEAN && tile.type != TileType::SAND)
                            {
                                tile.elevation = static_cast<std::int8_t>(target_elev);
                                if (tile.type == TileType::CLIFF)
                                    tile.type = TileType::GRASS;
                            }
                        }
                    };

                    ProcessSide(-TownConfig::RAMP_CORRIDOR_HALF_WIDTH - TownConfig::RAMP_NORMALIZE_HALF_WIDTH,
                                -TownConfig::RAMP_CORRIDOR_HALF_WIDTH);
                    ProcessSide(TownConfig::RAMP_CORRIDOR_HALF_WIDTH + 1,
                                TownConfig::RAMP_CORRIDOR_HALF_WIDTH + TownConfig::RAMP_NORMALIZE_HALF_WIDTH);
                }
            }

            void CarveRamp(Town &town, const RampCandidate &ramp)
            {
                const int w = Town::WIDTH * Acre::SIZE;
                const int h = Town::HEIGHT * Acre::SIZE;
                const int ramp_length = ramp.z_bottom - ramp.z_top;

                for (int z = ramp.z_top; z <= ramp.z_bottom; ++z)
                {
                    if (z < 0 || z >= h)
                        continue;

                    float t = static_cast<float>(z - ramp.z_top) / static_cast<float>(ramp_length);
                    t = std::clamp(t, 0.0f, 1.0f);
                    t = utils::SmoothStep(t);

                    int current_elev = (t > 0.5f) ? ramp.to_elevation : ramp.from_elevation;

                    for (int dx = -TownConfig::RAMP_CORRIDOR_HALF_WIDTH;
                         dx <= TownConfig::RAMP_CORRIDOR_HALF_WIDTH; ++dx)
                    {
                        int x = ramp.x + dx;
                        if (x < 0 || x >= w)
                            continue;

                        auto [a, l] = utils::GetTileCoords(x, z);
                        Tile &tile = town.GetAcre(a.x, a.y).tiles[l.y][l.x];

                        if (tile.type == TileType::RIVER || tile.type == TileType::WATERFALL ||
                            tile.type == TileType::RIVER_MOUTH || tile.type == TileType::OCEAN ||
                            tile.type == TileType::SAND)
                            continue;

                        tile.elevation = static_cast<std::int8_t>(current_elev);
                        tile.type = TileType::RAMP;
                    }
                }
                NormalizeRampSides(town, ramp);
            }

        } // anonymous namespace

        void Execute(
            Town &town,
            [[maybe_unused]] std::mt19937_64 &rng,
            const TownConfig &config)
        {
            std::vector<int> elevations;
            const int w = Town::WIDTH * Acre::SIZE;
            const int h = Town::HEIGHT * Acre::SIZE;

            // 1. Identify all unique elevations present in the town
            for (int z = 0; z < h; ++z)
            {
                for (int x = 0; x < w; ++x)
                {
                    int elev = utils::GetElevation(town, x, z);
                    if (elev > 0 && std::find(elevations.begin(), elevations.end(), elev) == elevations.end())
                    {
                        elevations.push_back(elev);
                    }
                }
            }
            // Sort high to low to process cliff levels sequentially
            std::sort(elevations.begin(), elevations.end(), std::greater<int>());

            std::vector<RampCandidate> placed_ramps;

            for (size_t i = 0; i < elevations.size(); ++i)
            {
                int from_elev = elevations[i];
                int to_elev = (i + 1 < elevations.size()) ? elevations[i + 1] : 0;

                // Find candidates for this specific cliff transition
                auto candidates = FindRampCandidates(town, from_elev, to_elev, config);
                if (candidates.empty())
                    continue;

                // --- 2. Identify River Location ---
                // We use the first candidate's Z to find the river center at that specific latitude
                int river_center_x = GetRiverCenterX(town, candidates[0].z_top);

                // --- 3. Bucket Candidates by Side ---
                std::vector<RampCandidate *> west_candidates;
                std::vector<RampCandidate *> east_candidates;

                for (auto &c : candidates)
                {
                    if (c.x < river_center_x)
                        west_candidates.push_back(&c);
                    else
                        east_candidates.push_back(&c);
                }

                int placed_count = 0;
                int attempts = 0;
                int total_potential = static_cast<int>(candidates.size());

                // --- 4. Selection Strategy ---
                // We prioritize alternating sides (West, then East) to ensure distribution
                // across the river. If one side is exhausted, we fallback to the other.
                while (placed_count < config.rampTopCandidates && attempts < total_potential)
                {
                    RampCandidate *pick = nullptr;

                    // Determine preference: West on 0, 2, 4... East on 1, 3, 5...
                    bool prefer_west = (placed_count % 2 == 0);

                    auto *primary = prefer_west ? &west_candidates : &east_candidates;
                    auto *backup = prefer_west ? &east_candidates : &west_candidates;

                    // Lambda to find the best remaining candidate in a specific bucket
                    auto FindBestValid = [&](std::vector<RampCandidate *> &side_list) -> RampCandidate *
                    {
                        for (auto *c : side_list)
                        {
                            // score > 0 ensures it passed ScoreRampLocation checks
                            if (c->score > 0.0f && !ConflictsWithExistingRamps(*c, placed_ramps))
                            {
                                return c;
                            }
                        }
                        return nullptr;
                    };

                    pick = FindBestValid(*primary);

                    // Fallback to the other side if primary side is empty or blocked
                    if (!pick)
                    {
                        pick = FindBestValid(*backup);
                    }

                    if (pick)
                    {
                        CarveRamp(town, *pick);
                        placed_ramps.push_back(*pick);
                        pick->score = -1.0f; // Mark as "used" so it isn't picked again
                        placed_count++;
                    }
                    else
                    {
                        // No valid candidates remain for this elevation transition
                        break;
                    }
                    attempts++;
                }
            }
        }

    } // namespace ramps
} // namespace cozy::world
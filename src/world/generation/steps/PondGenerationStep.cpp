#include "PondGenerationStep.h"

#include "world/Town.h"
#include "world/data/Acre.h"
#include "world/data/Tile.h"
#include "world/data/TownConfig.h"

#include <queue>
#include <random>
#include <glm/glm.hpp>

namespace cozy::world
{
    namespace ponds
    {

        void Execute(
            Town &town,
            std::mt19937_64 &rng,
            const TownConfig &config)
        {
            const int world_w = Town::WIDTH * Acre::SIZE;
            const int world_h = Town::HEIGHT * Acre::SIZE;

            std::uniform_int_distribution<int> x_dist(5, world_w - 6);
            std::uniform_int_distribution<int> z_dist(5, world_h - 6);
            std::uniform_int_distribution<int> chance(0, 99);

            std::queue<glm::ivec2> q;
            glm::ivec2 start{x_dist(rng), z_dist(rng)};

            q.push(start);
            int carved_count = 0;

            while (!q.empty() && carved_count < config.maxPondSize)
            {
                auto curr = q.front();
                q.pop();

                // Skip if already water (avoid overlapping with rivers mostly)
                auto [a, l] = town.WorldToTile({static_cast<float>(curr.x), 0.f, static_cast<float>(curr.y)});
                if (town.GetAcre(a.x, a.y).tiles[l.y][l.x].type == TileType::WATER)
                {
                    continue;
                }

                town.GetAcre(a.x, a.y).tiles[l.y][l.x].type = TileType::WATER;
                carved_count++;

                constexpr int dx[4] = {1, -1, 0, 0};
                constexpr int dz[4] = {0, 0, 1, -1};

                for (int d = 0; d < 4; ++d)
                {
                    int nx = curr.x + dx[d];
                    int nz = curr.y + dz[d];

                    if (nx >= 0 && nx < world_w && nz >= 0 && nz < world_h)
                    {
                        if (chance(rng) < config.pondSpreadChance)
                        {
                            q.push({nx, nz});
                        }
                    }
                }
            }
        }

    }
}
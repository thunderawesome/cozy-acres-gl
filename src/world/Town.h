#pragma once

#include <vector>
#include <array>
#include <unordered_map>
#include <random>
#include <glm/glm.hpp>
#include "rendering/InstanceData.h"

namespace cozy::world
{
    struct TownConfig
    {
        // Cliff Logic
        float cliffSmoothness = 0.15f; // 0.0 (sharp) to 1.0 (smooth)
        int minPlateauRow = 1;         // Usually Row B
        int maxPlateauRow = 3;         // Usually Row D

        // River Logic
        int riverWidth = 3;
        int riverMeanderChance = 20; // Percentage chance to shift X (0-100)

        // Pond Logic
        int maxPondSize = 25;
        int pondSpreadChance = 70; // Percentage chance to grow neighbor (0-100)
    };

    enum class TileType : uint8_t
    {
        EMPTY,
        GRASS,
        DIRT,
        WATER,
        TREE,
        ROCK,
        BUILDING,
        CLIFF
    };

    struct Tile
    {
        TileType type = TileType::EMPTY;
        int elevation = 0;
    };

    struct ObjectConfig
    {
        glm::ivec2 pos;
        glm::ivec2 size;
        bool blocks_path = true;
    };

    class Acre
    {
    public:
        static constexpr int SIZE = 16;
        std::array<std::array<Tile, SIZE>, SIZE> tiles{};
        std::vector<ObjectConfig> objects;
        std::unordered_map<uint16_t, const ObjectConfig *> object_lookup;

        void RebuildLookup();
    };

    class Town
    {
    public:
        static constexpr int WIDTH = 5;
        static constexpr int HEIGHT = 6;

        Town() = default;

        void Generate(uint64_t seed, const TownConfig &config = TownConfig());
        void DebugDump() const;

        // Bridge to Renderer
        std::vector<rendering::TileInstance> GenerateRenderData() const;

        // Coordinate conversion
        std::pair<glm::ivec2, glm::ivec2> WorldToTile(glm::vec3 world_pos) const;

    private:
        // Internal Context to follow DRY and SOLID principles
        struct GenContext
        {
            std::mt19937_64 &rng;
            const TownConfig &config;
        };

        std::array<std::array<Acre, HEIGHT>, WIDTH> m_Acres;

        // Generation Pipeline Steps
        void GenerateCliffs(GenContext &ctx);
        void CarveRiver(GenContext &ctx);
        void CarvePond(GenContext &ctx);
    };
}
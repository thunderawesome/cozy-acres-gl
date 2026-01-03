#pragma once

#include <vector>
#include <array>
#include <unordered_map>
#include <random>
#include <glm/glm.hpp>
#include "rendering/InstanceData.h" // Assuming TileInstance is here

namespace cozy::world
{
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

        void Generate(uint64_t seed);

        // This is the bridge to the Renderer
        std::vector<rendering::TileInstance> GenerateRenderData() const;

        // DS&A: Coordinate conversion
        std::pair<glm::ivec2, glm::ivec2> WorldToTile(glm::vec3 world_pos) const;

    private:
        std::array<std::array<Acre, HEIGHT>, WIDTH> m_Acres;

        // DS&A Generation Steps
        void CarveRiver(std::mt19937_64 &rng);
        void CarvePond(std::mt19937_64 &rng);
    };
}
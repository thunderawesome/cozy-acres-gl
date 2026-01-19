#pragma once
#include <array>
#include <vector>
#include <random>
#include <glm/glm.hpp>
#include "data/Acre.h"
#include "data/TownConfig.h"
#include "../rendering/InstanceData.h"

namespace cozy::world
{

    class GenerationPipeline;

    class Town
    {
    public:
        static constexpr int WIDTH = 5;
        static constexpr int HEIGHT = 7;

        Town() = default;

        void Generate(uint64_t seed, const TownConfig &config = {});

        // Debug & Rendering
        void DebugDump() const;
        std::vector<rendering::TileInstance> GenerateRenderData() const;

        // Coordinate helpers (very useful for all systems)
        std::pair<glm::ivec2, glm::ivec2> WorldToTile(glm::vec3 world_pos) const;

        // Low-level access (for generation steps)
        Acre &GetAcre(int ax, int az) { return m_acres[ax][az]; }
        const Acre &GetAcre(int ax, int az) const { return m_acres[ax][az]; }

        int GetElevation(int wx, int wz) const;

    private:
        std::array<std::array<Acre, HEIGHT>, WIDTH> m_acres;
    };

}
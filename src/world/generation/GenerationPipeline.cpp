#include "GenerationPipeline.h"
#include "../Town.h"

namespace cozy::world
{

    GenerationPipeline::GenerationPipeline(Town &target) : m_town(target) {}

    void GenerationPipeline::Execute(uint64_t seed, const TownConfig &config)
    {
        std::mt19937_64 rng(seed);

        for (auto &step : m_steps)
        {
            step(m_town, rng, config);
        }
    }

}
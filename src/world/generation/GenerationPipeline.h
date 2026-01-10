#pragma once
#include <vector>
#include <functional>
#include <memory>
#include <random>

namespace cozy::world
{

    class Town;
    struct TownConfig;

    class GenerationPipeline
    {
    public:
        explicit GenerationPipeline(Town &target_town);

        // Add any callable that matches this signature
        template <typename Callable>
        void AddStep(Callable &&step)
        {
            m_steps.emplace_back(std::forward<Callable>(step));
        }

        void Execute(uint64_t seed, const TownConfig &config);

    private:
        Town &m_town;
        std::vector<std::function<void(Town &, std::mt19937_64 &, const TownConfig &)>> m_steps;
    };

}
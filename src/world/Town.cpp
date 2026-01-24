#include "Town.h"
#include "world/data/TownConfig.h"
#include "generation/GenerationPipeline.h"
#include "generation/steps/OceanGenerationStep.h"
#include "generation/steps/CliffGenerationStep.h"
#include "generation/steps/RiverGenerationStep.h"
#include "generation/steps/RampGenerationStep.h"
#include "generation/steps/PondGenerationStep.h"
#include "presentation/TownPresenter.h"

namespace cozy::world
{
    Town::Town() { Reset(); }

    void Town::Reset()
    {
        for (auto &column : m_acres)
            for (auto &acre : column)
                for (auto &row : acre.tiles)
                    for (auto &tile : row)
                    {
                        tile.type = TileType::GRASS;
                        tile.elevation = 0;
                        tile.autotileIndex = 0;
                    }
    }

    void Town::Generate(uint64_t seed, const TownConfig &config)
    {
        Reset();

        // The pipeline orchestrates the logic steps
        GenerationPipeline pipeline(*this);
        pipeline.AddStep(ocean::Execute);
        pipeline.AddStep(cliffs::Execute);
        pipeline.AddStep(rivers::Execute);
        pipeline.AddStep(ramps::Execute);
        pipeline.AddStep(ponds::Execute);

        pipeline.Execute(seed, config);

        // Delegated to the presenter
        TownPresenter::DebugDump(*this);
    }
}
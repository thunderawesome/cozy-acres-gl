# Cozy Acres Map Generator

This branch introduces the procedural map generation system for Cozy Acres, featuring multi-tiered cliff layouts and meandering river systems. The generator is built with a focus on "stepped" acre-based transitions to ensure consistent connection points between different world sections.

## 🛠 Features

### 1. Cliff & Elevation Generation (`CliffGenerationStep.cpp`)

Generates a multi-tiered terrain layout with support for up to three elevation levels (Ground, Mid, and High plateaus).

* **Stepped Boundaries**: Uses a `BuildSteppedBoundary` system to ensure that elevation changes happen at consistent connection points (defined by `CLIFF_CONNECTION_POINT_OFFSET`), preventing disjointed terrain between acres.
* **Snapped Elevation**: Implements `ComputeSnappedElevation` to "snap" tiles to their correct tier based on their position relative to the generated boundary lines.
* **Cliff Tagging**: Automatically identifies and tags `TileType::CLIFF` by checking for adjacent tiles with lower elevation levels.

### 2. River Generation (`RiverGenerationStep.cpp`)

Carves a continuous water path through the town that respects the generated elevation.

* **Meandering Logic**: Features a meander system with configurable chances for both vertical and long horizontal segments.
* **Elevation-Aware Carving**: The `CheckPathValid` function ensures rivers only flow "downhill" or across level ground, preventing water from being carved into higher terrain tiers.
* **Meander Constraints**: Includes a safety system to force a bend after a certain number of consecutive straight acres, ensuring visual variety.

### 3. Debug Visualizer (`townvisualizer.html`)

A standalone web tool to test and visualize the generation logic without running the full game engine.

* **Playback Controls**: Includes "Start Debug Mode" to watch the map generate tile-by-tile, with pause, resume, and step-forward functionality.
* **Timeline Scrubbing**: A timeline slider allows you to jump to any specific step in the generation sequence.
* **Configurable Parameters**: Real-time adjustment of seeds, acre dimensions, plateau chances, and river widths.

## 🧪 Configuration

The generator uses `TownConfig` to control various generation weights:

* `highPlateauChance`: Probability (0.0 - 1.0) of generating a third elevation tier.
* `riverMeanderChance`: Probability of a river segment shifting columns.
* `CLIFF_CONNECTION_POINT_OFFSET`: The specific tile index within an acre where cliff transitions occur.

## 🚀 How to Use

The generation is orchestrated through the `Execute` functions in the `cliffs` and `rivers` namespaces.

1. Initialize a `Town` object with the desired width/height in acres.
2. Run `cozy::world::cliffs::Execute` to establish the heightmap.
3. Run `cozy::world::rivers::Execute` to carve the water system based on those elevations.

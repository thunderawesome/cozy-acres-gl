# Cozy Acres Map Generator 🌿

*A modern OpenGL recreation of the Animal Crossing (GameCube) town generation system*

**Cozy Acres** is a C++17 + OpenGL procedural map generator inspired by the **original Animal Crossing (GameCube)** town generation logic.

Rather than relying on noise-based terrain, this project recreates the *rule-driven*, **acre-based** world structure that defined the GameCube era: fixed-size acres, stepped elevation tiers, deterministic cliff transitions, and rivers that flow naturally downhill through the town.

The goal is both **faithful recreation** and **clear, inspectable procedural design** using modern tooling.

![Map Generator Demo](https://s4.ezgif.com/tmp/ezgif-43d480365a3628ca.gif)

---

## 🧱 Acre-Based World Structure

The world is built from a grid of fixed-size **acres**, mirroring Animal Crossing’s internal map layout. Each acre:

* Has consistent connection points to its neighbors
* Transitions elevation only at predefined tile offsets
* Participates in larger, town-wide rules (plateau placement, river flow, ponds)

This ensures the resulting town always feels *cohesive* rather than randomly stitched together.

---

## 🚀 Interactive Algorithm Visualizer

Because Cozy Acres is primarily a **C++ OpenGL project**, a **web-based visualizer** is included at:

```
/misc/townvisualizer.html
```

This tool allows you to validate procedural logic before GPU implementation and observe the generation process in isolation.

> **NOTE:** JavaScript and C++ differ in numeric precision and integer behavior. Even with similar algorithms, seeded output may not match exactly.

### Live Demo

[![Launch Visualizer](https://img.shields.io/badge/Launch-Live%20Visualizer-brightgreen?style=for-the-badge\&logo=javascript)](https://thunderawesome.github.io/cozy-acres-gl/townvisualizer.html)

### Visualizer Capabilities

* **Shuffle** – Rapidly preview many town layouts
* **Deterministic Seeds** – Reproduce identical towns with a known seed
* **Debug Mode** – Watch cliffs, rivers, and terrain tags update tile-by-tile
* **Frame Stepping** – Pause and advance the algorithm incrementally
* **Timeline Scrubbing** – Jump to any point in the generation process

---

## 🛠 Generation Systems

### 🪨 Terrain, Cliffs & Elevation

Cozy Acres generates a stepped terrain layout inspired by Animal Crossing’s iconic cliffs and plateaus.

* Supports **multiple elevation tiers** (ground, mid, and optional high plateaus)
* Elevation changes are restricted to **fixed connection points** inside each acre
* Tiles are automatically classified as walkable ground, cliff faces, or transitions
* Elevation is *snapped* to clean tiers to avoid noisy or ambiguous terrain

This produces readable, intentional terrain rather than organic noise fields.

---

### 🌊 Rivers & Water Flow

Rivers are carved after terrain generation and must obey strict constraints:

* Rivers always flow **downhill or across level terrain**
* Meandering behavior introduces horizontal variation
* Safeguards prevent overly straight or repetitive paths
* Rivers align cleanly across acre boundaries

The result is a single, continuous river system that feels hand-placed while remaining fully procedural.

---

### 💧 Ponds

Ponds are generated as localized water features with configurable size ranges. They integrate with the existing terrain without breaking elevation rules or acre connectivity.

---

## 🧪 Configuration-Driven Design

All generation behavior is controlled through a single configuration structure, **`TownConfig`**, making the system easy to tune, experiment with, or extend.

### Terrain & Cliff Controls

* `cliffSmoothness`
  Controls how sharp or gradual elevation transitions appear
  *(0.0 = sharp steps, 1.0 = smooth transitions)*

* `minPlateauRow` / `maxPlateauRow`
  Constrains where plateaus may appear vertically in the town

* `minHighPlateauRowOffset` / `maxHighPlateauRowOffset`
  Controls how far above the mid-plateau a high plateau may form

* `highPlateauChance`
  Probability of generating a third elevation tier

* `CLIFF_CONNECTION_POINT_OFFSET`
  Fixed tile index inside an acre where elevation transitions are allowed
  *(Key to preserving Animal Crossing–style consistency)*

---

### River Controls

* `riverWidth`
  Controls how wide the river appears in tiles

* `riverMeanderChance`
  Likelihood that the river changes direction

* `riverHorizontalChance`
  Probability of long horizontal river segments

* `RIVER_CONNECTION_POINT_OFFSET`
  Defines consistent acre-to-acre river alignment

---

### Pond Controls

* `minPondRadius` / `maxPondRadius`
  Controls the size range of generated ponds

---

## 🚀 High-Level Generation Flow

1. Create a town using a grid of acres
2. Generate elevation tiers and cliffs using deterministic rules
3. Carve rivers that respect terrain constraints
4. Place ponds and secondary water features

Each step builds on the previous one, closely following the **ordered, rule-based philosophy** of the original Animal Crossing generator.

---

## 🎮 Inspiration & Goals

Cozy Acres is both:

* A **technical deep-dive** into classic GameCube procedural design
* A **faithful recreation** of Animal Crossing’s town-generation philosophy
* A sandbox for experimenting with **deterministic worldbuilding** in modern C++

Rather than hiding logic behind noise functions, this project prioritizes **clarity, control, and intent**.

---

If you want next:

* A short **“Design Philosophy vs Noise-Based Terrain”** section
* A **portfolio-optimized README** for recruiters
* Inline diagrams explaining acre connectivity and stepped cliffs

Just say the word 🌱

#pragma once
#include "WorldGenUtils.h"
#include <cstdint>
#include <array>

namespace cozy::world::utils
{
    // ============================================================================
    // DUAL-GRID AUTOTILING SYSTEM
    // Uses only 6 unique tiles (+ rotations) for organic Animal Crossing-style terrain
    // ============================================================================

    // The 6 unique tile types in dual-grid system
    enum class DualGridTile : uint8_t
    {
        EMPTY = 0,     // 0000: All grass (or "background" terrain)
        FULL = 1,      // 1111: All water (or "foreground" terrain)
        CORNER = 2,    // 1 corner filled (inner curve) - e.g., 0001
        SIDE = 3,      // 2 adjacent filled (straight edge) - e.g., 0011
        DIAGONAL = 4,  // 2 opposite filled (rare) - e.g., 0101
        INV_CORNER = 5 // 3 corners filled (outer curve) - e.g., 1110
    };

    // Result contains both tile type and rotation
    struct DualGridResult
    {
        DualGridTile tile;
        int rotation; // 0, 90, 180, 270 degrees (clockwise)
    };

    // Map 4-bit index to tile type + rotation
    // Index format: TL(bit 3) | TR(bit 2) | BL(bit 1) | BR(bit 0)
    inline const DualGridResult DUAL_GRID_MAP[16] = {
        // 0000: No water
        {DualGridTile::EMPTY, 0},

        // Single corner (CORNER tile with different rotations)
        {DualGridTile::CORNER, 0},   // 0001: BR corner water
        {DualGridTile::CORNER, 270}, // 0010: BL corner water
        {DualGridTile::CORNER, 90},  // 0100: TR corner water
        {DualGridTile::CORNER, 180}, // 1000: TL corner water

        // Two adjacent (SIDE tile with different rotations)
        {DualGridTile::SIDE, 270}, // 0011: Bottom edge water
        {DualGridTile::SIDE, 0},   // 0110: Right edge water
        {DualGridTile::SIDE, 90},  // 1100: Top edge water
        {DualGridTile::SIDE, 180}, // 1001: Left edge water

        // Two opposite (DIAGONAL tile - rare)
        {DualGridTile::DIAGONAL, 0},  // 0101: TR+BL diagonal
        {DualGridTile::DIAGONAL, 90}, // 1010: TL+BR diagonal

        // Three corners (INV_CORNER tile with different rotations)
        {DualGridTile::INV_CORNER, 270}, // 0111: TL is grass (3 water)
        {DualGridTile::INV_CORNER, 0},   // 1011: TR is grass (3 water)
        {DualGridTile::INV_CORNER, 90},  // 1101: BL is grass (3 water)
        {DualGridTile::INV_CORNER, 180}, // 1110: BR is grass (3 water)

        // 1111: All water
        {DualGridTile::FULL, 0}};

    // Calculate 4-bit dual-grid index for a vertex position
    // This samples the 2x2 cluster of data points around the vertex
    template <typename Predicate>
    inline uint8_t CalculateDualGridIndex(int x, int z, Predicate shouldConnect)
    {
        // Sample 2x2 cluster around this vertex
        // The vertex sits at the CENTER of these 4 data points
        bool tl = shouldConnect(x, z);         // Top-left data point
        bool tr = shouldConnect(x + 1, z);     // Top-right data point
        bool bl = shouldConnect(x, z + 1);     // Bottom-left data point
        bool br = shouldConnect(x + 1, z + 1); // Bottom-right data point

        // Pack into 4-bit index: TL(3) | TR(2) | BL(1) | BR(0)
        return (static_cast<uint8_t>(tl) << 3) | (static_cast<uint8_t>(tr) << 2) | (static_cast<uint8_t>(bl) << 1) | static_cast<uint8_t>(br);
    }

    // Get tile type and rotation from 4-bit index
    inline DualGridResult GetDualGridTile(uint8_t index)
    {
        return DUAL_GRID_MAP[index & 0x0F];
    }

    // ============================================================================
    // CONVENIENCE FUNCTIONS FOR COMMON TERRAIN TYPES
    // ============================================================================

    // Water tiles (ponds, rivers, ocean)
    inline DualGridResult CalculateWaterDualGrid(const Town &town, int x, int z)
    {
        auto waterPredicate = [&](int nx, int nz) -> bool
        {
            return utils::IsAnyWater(utils::GetTileTypeSafe(town, nx, nz));
        };
        uint8_t index = CalculateDualGridIndex(x, z, waterPredicate);
        return GetDualGridTile(index);
    }

    // Cliff tiles (elevation-based)
    inline DualGridResult CalculateCliffDualGrid(const Town &town, int x, int z, int elevation)
    {
        auto cliffPredicate = [&](int nx, int nz) -> bool
        {
            int elev = town.GetElevation(nx, nz);
            return elev >= elevation; // At or above this elevation level
        };
        uint8_t index = CalculateDualGridIndex(x, z, cliffPredicate);
        return GetDualGridTile(index);
    }

    // Sand/beach tiles
    inline DualGridResult CalculateBeachDualGrid(const Town &town, int x, int z)
    {
        auto beachPredicate = [&](int nx, int nz) -> bool
        {
            TileType type = utils::GetTileTypeSafe(town, nx, nz);
            return type == TileType::SAND;
        };
        uint8_t index = CalculateDualGridIndex(x, z, beachPredicate);
        return GetDualGridTile(index);
    }

    // Generic dual-grid calculation for any tile type
    inline DualGridResult CalculateTerrainDualGrid(const Town &town, int x, int z, TileType targetType)
    {
        auto typePredicate = [&](int nx, int nz) -> bool
        {
            return utils::GetTileTypeSafe(town, nx, nz) == targetType;
        };
        uint8_t index = CalculateDualGridIndex(x, z, typePredicate);
        return GetDualGridTile(index);
    }

    // ============================================================================
    // BACKWARD COMPATIBILITY (OPTIONAL)
    // Keep old blob functions but map them to dual-grid internally
    // ============================================================================

    // Convert 8-bit blob mask to closest dual-grid equivalent
    // This allows gradual migration from blob to dual-grid
    inline DualGridResult BlobMaskToDualGrid(uint8_t blob_mask)
    {
        // Extract cardinal directions from blob mask
        bool n = blob_mask & 2;
        bool s = blob_mask & 64;
        bool w = blob_mask & 8;
        bool e = blob_mask & 16;

        // Convert to dual-grid 4-bit index
        // Map cardinal directions to corner states
        uint8_t dual_index = 0;
        if (n && w)
            dual_index |= 8; // TL
        if (n && e)
            dual_index |= 4; // TR
        if (s && w)
            dual_index |= 2; // BL
        if (s && e)
            dual_index |= 1; // BR

        return GetDualGridTile(dual_index);
    }

    // ============================================================================
    // LEGACY COMPATIBILITY WRAPPER
    // Maps old CalculatePondBlobIndex to new dual-grid system
    // ============================================================================

    [[deprecated("Use CalculateWaterDualGrid instead")]]
    inline int CalculatePondBlobIndex(const Town &town, int x, int z)
    {
        // Return dual-grid tile as integer (0-5) for backward compatibility
        auto result = CalculateWaterDualGrid(town, x, z);
        return static_cast<int>(result.tile);
    }

} // namespace cozy::world::utils
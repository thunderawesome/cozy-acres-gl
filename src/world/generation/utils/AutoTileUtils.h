#pragma once
#include "WorldGenUtils.h"
#include <cstdint>
#include <vector>

namespace cozy::world::utils
{
    // Standard 47-tile Blob Mapping (0-255 -> 0-46)
    // This is the actual industry-standard Blob 47 mapping
    inline const int BLOB_MAP[] = {
        0, 1, 0, 1, 2, 3, 2, 3, 0, 1, 0, 1, 2, 3, 2, 3, 4, 5, 4, 5, 6, 7, 6, 7, 4, 5, 4, 5, 6, 7, 6, 7,
        0, 1, 0, 1, 2, 3, 2, 3, 0, 1, 0, 1, 2, 3, 2, 3, 4, 5, 4, 5, 6, 7, 6, 7, 4, 5, 4, 5, 6, 7, 6, 7,
        8, 9, 8, 9, 10, 11, 10, 11, 8, 9, 8, 9, 10, 11, 10, 11, 12, 13, 12, 13, 14, 15, 14, 15, 12, 13, 12, 13, 14, 15, 14, 15,
        8, 9, 8, 9, 10, 11, 10, 11, 8, 9, 8, 9, 10, 11, 10, 11, 12, 13, 12, 13, 14, 15, 14, 15, 12, 13, 12, 13, 14, 15, 14, 15,
        16, 17, 16, 17, 18, 19, 18, 19, 16, 17, 16, 17, 18, 19, 18, 19, 20, 21, 20, 21, 22, 23, 22, 23, 20, 21, 20, 21, 22, 23, 22, 23,
        16, 17, 16, 17, 18, 19, 18, 19, 16, 17, 16, 17, 18, 19, 18, 19, 20, 21, 20, 21, 22, 23, 22, 23, 20, 21, 20, 21, 22, 23, 22, 23,
        24, 25, 24, 25, 26, 27, 26, 27, 24, 25, 24, 25, 26, 27, 26, 27, 28, 29, 28, 29, 30, 31, 30, 31, 28, 29, 28, 29, 30, 31, 30, 31,
        32, 33, 32, 33, 34, 35, 34, 35, 36, 37, 36, 37, 38, 39, 38, 39, 40, 41, 40, 41, 42, 43, 42, 43, 44, 45, 44, 45, 46, 46, 46, 46};

    // Safety check: ensure we didn't miss any values
    static_assert(sizeof(BLOB_MAP) / sizeof(int) == 256, "BLOB_MAP must have exactly 256 entries");

    template <typename Predicate>
    inline uint8_t Calculate8BitMask(int x, int z, Predicate shouldConnect)
    {
        // Check cardinal neighbors first
        bool n = shouldConnect(x, z - 1);
        bool s = shouldConnect(x, z + 1);
        bool w = shouldConnect(x - 1, z);
        bool e = shouldConnect(x + 1, z);

        uint8_t mask = 0;

        // Cardinal bits: North(2), West(8), East(16), South(64)
        if (n)
            mask |= 2;
        if (w)
            mask |= 8;
        if (e)
            mask |= 16;
        if (s)
            mask |= 64;

        // Diagonal bits: Only valid if both adjacent cardinal neighbors also connect.
        // This ensures we only use corner tiles when there's a continuous surface.
        if (n && w && shouldConnect(x - 1, z - 1))
            mask |= 1; // NW
        if (n && e && shouldConnect(x + 1, z - 1))
            mask |= 4; // NE
        if (s && w && shouldConnect(x - 1, z + 1))
            mask |= 32; // SW
        if (s && e && shouldConnect(x + 1, z + 1))
            mask |= 128; // SE

        return mask;
    }

    inline int GetBlobIndex(uint8_t mask)
    {
        return BLOB_MAP[mask];
    }

    inline int CalculatePondBlobIndex(const Town &town, int x, int z)
    {
        auto pondPredicate = [&](int nx, int nz)
        {
            return utils::IsAnyWater(utils::GetTileTypeSafe(town, nx, nz));
        };
        uint8_t rawMask = Calculate8BitMask(x, z, pondPredicate);
        return GetBlobIndex(rawMask);
    }
}
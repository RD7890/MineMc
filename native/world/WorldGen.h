#pragma once
#include "Chunk.h"

namespace minemc {

class WorldGen {
public:
    // Fill a chunk with terrain data.
    // Uses a simple noise-based heightmap:
    //   y < stoneDepth  → Stone
    //   y < surfaceY-1  → Dirt
    //   y == surfaceY-1 → Dirt
    //   y == surfaceY   → Grass
    //   y >  surfaceY   → Air
    static void generate(Chunk& chunk);

private:
    // Simple deterministic noise based on chunk/block coords.
    static float heightNoise(float x, float z);
    static int surfaceHeight(int wx, int wz);
};

} // namespace minemc

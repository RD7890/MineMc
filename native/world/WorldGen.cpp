#include "WorldGen.h"
#include <cmath>

namespace minemc {

// A minimal deterministic smooth noise function.
// No external dependencies — just trig + integer hash.
static float smoothNoise(float x, float z) {
    // Integer grid corners
    int ix = (int)std::floor(x);
    int iz = (int)std::floor(z);

    // Fractional part
    float fx = x - ix;
    float fz = z - iz;

    // Smooth step
    float ux = fx * fx * (3.0f - 2.0f * fx);
    float uz = fz * fz * (3.0f - 2.0f * fz);

    // Pseudo-random gradient per corner (simple hash)
    auto rand2 = [](int a, int b) -> float {
        int n = a + b * 57;
        n = (n << 13) ^ n;
        return (1.0f - ((n * (n * n * 15731 + 789221) + 1376312589) & 0x7FFFFFFF) / 1073741824.0f);
    };

    float v00 = rand2(ix,     iz);
    float v10 = rand2(ix + 1, iz);
    float v01 = rand2(ix,     iz + 1);
    float v11 = rand2(ix + 1, iz + 1);

    float top    = v00 + ux * (v10 - v00);
    float bottom = v01 + ux * (v11 - v01);
    return top + uz * (bottom - top);
}

float WorldGen::heightNoise(float x, float z) {
    // Octave sum for more interesting terrain
    float h = 0.0f;
    h += smoothNoise(x * 0.03f, z * 0.03f) * 16.0f;  // large hills
    h += smoothNoise(x * 0.08f, z * 0.08f) * 6.0f;   // medium bumps
    h += smoothNoise(x * 0.20f, z * 0.20f) * 2.0f;   // small detail
    return h;
}

int WorldGen::surfaceHeight(int wx, int wz) {
    const int BASE_HEIGHT = 60;
    float noise = heightNoise(static_cast<float>(wx), static_cast<float>(wz));
    return BASE_HEIGHT + static_cast<int>(noise);
}

void WorldGen::generate(Chunk& chunk) {
    const int ox = chunk.coord.cx * CHUNK_SIZE;
    const int oz = chunk.coord.cz * CHUNK_SIZE;

    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int z = 0; z < CHUNK_SIZE; ++z) {
            int worldX = ox + x;
            int worldZ = oz + z;
            int surface = surfaceHeight(worldX, worldZ);
            surface = std::max(1, std::min(surface, CHUNK_HEIGHT - 2));

            for (int y = 0; y < CHUNK_HEIGHT; ++y) {
                BlockType bt;
                if (y == 0) {
                    bt = BlockType::Stone; // bedrock-ish floor
                } else if (y < surface - 4) {
                    bt = BlockType::Stone;
                } else if (y < surface) {
                    bt = BlockType::Dirt;
                } else if (y == surface) {
                    bt = BlockType::Grass;
                } else {
                    bt = BlockType::Air;
                }
                chunk.blocks[Chunk::index(x, y, z)] = bt;
            }
        }
    }
    chunk.meshDirty = true;
}

} // namespace minemc

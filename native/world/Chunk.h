#pragma once
#include "Block.h"
#include <array>
#include <vector>
#include <glm/glm.hpp>

namespace minemc {

constexpr int CHUNK_SIZE   = 16;  // X and Z
constexpr int CHUNK_HEIGHT = 128; // Y

// Flat 3-D array stored as [x][y][z]
using BlockArray = std::array<BlockType, CHUNK_SIZE * CHUNK_HEIGHT * CHUNK_SIZE>;

// Chunk coordinates are in chunk-space (divide world XZ by CHUNK_SIZE).
struct ChunkCoord {
    int cx, cz;
    bool operator==(const ChunkCoord& o) const { return cx == o.cx && cz == o.cz; }
};

struct ChunkCoordHash {
    size_t operator()(const ChunkCoord& c) const {
        return std::hash<int>()(c.cx) ^ (std::hash<int>()(c.cz) << 16);
    }
};

// Vertex layout: position (3) + normal (3) + uv (2) = 8 floats per vertex.
struct Vertex {
    float x, y, z;
    float nx, ny, nz;
    float u, v;
};

class Chunk {
public:
    ChunkCoord coord;
    BlockArray blocks{};

    // GL mesh state (set by Renderer)
    unsigned int vao = 0;
    unsigned int vbo = 0;
    int vertexCount = 0;
    bool meshDirty = true;
    bool gpuUploaded = false;

    // World-space origin of this chunk (x and z, y = 0)
    glm::vec3 worldOrigin() const {
        return { coord.cx * CHUNK_SIZE, 0.0f, coord.cz * CHUNK_SIZE };
    }

    // Block access by local chunk coordinates
    BlockType getBlock(int x, int y, int z) const;
    void setBlock(int x, int y, int z, BlockType t);

    static bool inBounds(int x, int y, int z);
    static int index(int x, int y, int z);
};

} // namespace minemc

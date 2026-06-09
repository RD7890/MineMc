#pragma once
#include "Chunk.h"
#include <unordered_map>
#include <memory>
#include <vector>
#include <functional>

namespace minemc {

class World {
public:
    World() = default;
    ~World() = default;

    // Load chunks around a position (chunk-space radius = renderDist)
    void updateChunks(float worldX, float worldZ, int renderDist);

    // Block access (world coordinates)
    BlockType getBlock(int x, int y, int z) const;
    void setBlock(int x, int y, int z, BlockType t);

    // Get or create a chunk
    Chunk* getChunk(int cx, int cz);
    const Chunk* getChunkConst(int cx, int cz) const;

    // Return all loaded chunks (for rendering)
    std::vector<Chunk*> getAllChunks();

    // Erase all blocks (called before generating a new world)
    void clear();

    static ChunkCoord toChunkCoord(int wx, int wz);
    static void toLocalCoord(int wx, int wz, int& lx, int& lz);

private:
    std::unordered_map<ChunkCoord, std::unique_ptr<Chunk>, ChunkCoordHash> mChunks;
};

} // namespace minemc

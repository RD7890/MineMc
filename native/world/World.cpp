#include "World.h"
#include "WorldGen.h"
#include <android/log.h>
#define TAG "MineMc/World"

namespace minemc {

ChunkCoord World::toChunkCoord(int wx, int wz) {
    // floor-divide so negative coords map correctly
    int cx = (wx >= 0) ? wx / CHUNK_SIZE : (wx - CHUNK_SIZE + 1) / CHUNK_SIZE;
    int cz = (wz >= 0) ? wz / CHUNK_SIZE : (wz - CHUNK_SIZE + 1) / CHUNK_SIZE;
    return { cx, cz };
}

void World::toLocalCoord(int wx, int wz, int& lx, int& lz) {
    lx = ((wx % CHUNK_SIZE) + CHUNK_SIZE) % CHUNK_SIZE;
    lz = ((wz % CHUNK_SIZE) + CHUNK_SIZE) % CHUNK_SIZE;
}

Chunk* World::getChunk(int cx, int cz) {
    ChunkCoord key{ cx, cz };
    auto it = mChunks.find(key);
    if (it == mChunks.end()) {
        // Generate new chunk
        auto chunk = std::make_unique<Chunk>();
        chunk->coord = key;
        WorldGen::generate(*chunk);
        auto* ptr = chunk.get();
        mChunks[key] = std::move(chunk);
        return ptr;
    }
    return it->second.get();
}

const Chunk* World::getChunkConst(int cx, int cz) const {
    auto it = mChunks.find({ cx, cz });
    return (it != mChunks.end()) ? it->second.get() : nullptr;
}

BlockType World::getBlock(int x, int y, int z) const {
    if (y < 0 || y >= CHUNK_HEIGHT) return BlockType::Air;
    auto coord = toChunkCoord(x, z);
    const Chunk* ch = getChunkConst(coord.cx, coord.cz);
    if (!ch) return BlockType::Air;
    int lx, lz;
    toLocalCoord(x, z, lx, lz);
    return ch->getBlock(lx, y, lz);
}

void World::setBlock(int x, int y, int z, BlockType t) {
    if (y < 0 || y >= CHUNK_HEIGHT) return;
    auto coord = toChunkCoord(x, z);
    Chunk* ch = getChunk(coord.cx, coord.cz);
    int lx, lz;
    toLocalCoord(x, z, lx, lz);
    ch->setBlock(lx, y, lz, t);

    // Mark neighbouring chunks dirty if on border
    if (lx == 0) {
        auto* n = getChunk(coord.cx - 1, coord.cz); if (n) n->meshDirty = true;
    }
    if (lx == CHUNK_SIZE - 1) {
        auto* n = getChunk(coord.cx + 1, coord.cz); if (n) n->meshDirty = true;
    }
    if (lz == 0) {
        auto* n = getChunk(coord.cx, coord.cz - 1); if (n) n->meshDirty = true;
    }
    if (lz == CHUNK_SIZE - 1) {
        auto* n = getChunk(coord.cx, coord.cz + 1); if (n) n->meshDirty = true;
    }
}

void World::updateChunks(float worldX, float worldZ, int renderDist) {
    int pcx = static_cast<int>(worldX) / CHUNK_SIZE;
    int pcz = static_cast<int>(worldZ) / CHUNK_SIZE;

    // Ensure chunks within radius are loaded
    for (int dx = -renderDist; dx <= renderDist; ++dx) {
        for (int dz = -renderDist; dz <= renderDist; ++dz) {
            getChunk(pcx + dx, pcz + dz);
        }
    }

    // Unload distant chunks
    auto it = mChunks.begin();
    while (it != mChunks.end()) {
        int ddx = it->first.cx - pcx;
        int ddz = it->first.cz - pcz;
        if (std::abs(ddx) > renderDist + 1 || std::abs(ddz) > renderDist + 1) {
            it = mChunks.erase(it);
        } else {
            ++it;
        }
    }
}

std::vector<Chunk*> World::getAllChunks() {
    std::vector<Chunk*> result;
    result.reserve(mChunks.size());
    for (auto& pair : mChunks) {
        result.push_back(pair.second.get());
    }
    return result;
}

void World::clear() {
    mChunks.clear();
}

} // namespace minemc

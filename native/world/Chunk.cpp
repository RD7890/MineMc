#include "Chunk.h"
#include <stdexcept>

namespace minemc {

bool Chunk::inBounds(int x, int y, int z) {
    return x >= 0 && x < CHUNK_SIZE
        && y >= 0 && y < CHUNK_HEIGHT
        && z >= 0 && z < CHUNK_SIZE;
}

int Chunk::index(int x, int y, int z) {
    return x * CHUNK_HEIGHT * CHUNK_SIZE + y * CHUNK_SIZE + z;
}

BlockType Chunk::getBlock(int x, int y, int z) const {
    if (!inBounds(x, y, z)) return BlockType::Air;
    return blocks[index(x, y, z)];
}

void Chunk::setBlock(int x, int y, int z, BlockType t) {
    if (!inBounds(x, y, z)) return;
    blocks[index(x, y, z)] = t;
    meshDirty = true;
}

} // namespace minemc

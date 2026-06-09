#pragma once
#include "World.h"
#include <string>

namespace minemc {

// Saves and loads the full set of loaded chunks to/from a binary file.
// Format: [header magic 4B] [chunkCount 4B] {[cx 4B][cz 4B][blockData CHUNK_SIZE*CHUNK_HEIGHT*CHUNK_SIZE bytes]}*
class WorldSerializer {
public:
    static constexpr uint32_t MAGIC = 0x4D4D4356; // "MMCV"

    // Returns true on success.
    static bool save(const World& world, const std::string& path);
    static bool load(World& world, const std::string& path);

    static bool fileExists(const std::string& path);
};

} // namespace minemc

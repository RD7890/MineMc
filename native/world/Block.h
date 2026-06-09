#pragma once
#include <cstdint>

namespace minemc {

// Block type IDs. 0 = air. Keep this small for MVP.
enum class BlockType : uint8_t {
    Air   = 0,
    Grass = 1,
    Dirt  = 2,
    Stone = 3,
    // Future: Wood, Leaves, Sand, ...
    COUNT
};

struct BlockDef {
    const char* name;
    // UV offsets into the texture atlas (in atlas-tile units).
    // Each face: top, bottom, side.
    int uvTop, uvBottom, uvSide;
    bool solid;
    bool transparent;
};

// Must match BlockType enum order (index = BlockType value).
constexpr BlockDef kBlockDefs[] = {
    // Air
    { "air",   0, 0, 0, false, true  },
    // Grass
    { "grass", 0, 2, 1, true,  false },
    // Dirt
    { "dirt",  2, 2, 2, true,  false },
    // Stone
    { "stone", 3, 3, 3, true,  false },
};

inline const BlockDef& getBlockDef(BlockType t) {
    return kBlockDefs[static_cast<uint8_t>(t)];
}

inline bool isSolid(BlockType t) { return getBlockDef(t).solid; }
inline bool isAir(BlockType t)   { return t == BlockType::Air; }

} // namespace minemc

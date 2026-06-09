#pragma once
#include "world/Chunk.h"
#include "world/Block.h"
#include "world/World.h"
#include <vector>
#include <GLES3/gl3.h>

namespace minemc {

// Builds a face-culled triangle mesh for a chunk.
// Each quad face is two triangles = 6 vertices.
class MeshBuilder {
public:
    // Build mesh vertices for a chunk. Needs neighbouring chunk data for
    // correct border culling. Writes into chunk.vertices (local buffer).
    static std::vector<Vertex> buildChunkMesh(const Chunk& chunk, const World& world);

private:
    // Emit a single face if the neighbouring block is transparent.
    static void emitFace(std::vector<Vertex>& verts,
                         float x, float y, float z,
                         int faceDir,   // 0=+X 1=-X 2=+Y 3=-Y 4=+Z 5=-Z
                         BlockType bt);

    // UV rect for a given atlas tile index (row=0, col=tileIdx)
    static void faceUV(int tileIdx, float& u0, float& v0, float& u1, float& v1);
};

} // namespace minemc

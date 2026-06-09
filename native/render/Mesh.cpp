#include "Mesh.h"
#include <cstring>

namespace minemc {

// Atlas layout: single row of ATLAS_COLS tiles, each 16x16 px.
// Indices: 0=grass_top, 1=grass_side, 2=dirt, 3=stone
static constexpr int ATLAS_COLS = 4;

static const float kFaceNormals[6][3] = {
    { 1, 0, 0}, {-1, 0, 0},
    { 0, 1, 0}, { 0,-1, 0},
    { 0, 0, 1}, { 0, 0,-1},
};

// Six quads, each defined by four corners (CCW winding for front face).
// Positions are offsets within a [0,1]^3 unit block.
static const float kFaceQuads[6][4][3] = {
    // +X
    {{1,0,1},{1,1,1},{1,1,0},{1,0,0}},
    // -X
    {{0,0,0},{0,1,0},{0,1,1},{0,0,1}},
    // +Y
    {{0,1,1},{1,1,1},{1,1,0},{0,1,0}},
    // -Y
    {{0,0,0},{1,0,0},{1,0,1},{0,0,1}},
    // +Z
    {{0,0,1},{1,0,1},{1,1,1},{0,1,1}},
    // -Z
    {{1,0,0},{0,0,0},{0,1,0},{1,1,0}},
};

// Neighbour offsets matching kFaceNormals
static const int kNeighbourDX[6] = {1,-1, 0, 0, 0, 0};
static const int kNeighbourDY[6] = {0, 0, 1,-1, 0, 0};
static const int kNeighbourDZ[6] = {0, 0, 0, 0, 1,-1};

void MeshBuilder::faceUV(int tileIdx, float& u0, float& v0, float& u1, float& v1) {
    float tw = 1.0f / ATLAS_COLS;
    u0 = tileIdx * tw;
    v0 = 0.0f;
    u1 = u0 + tw;
    v1 = 1.0f;
}

void MeshBuilder::emitFace(std::vector<Vertex>& verts,
                            float bx, float by, float bz,
                            int face, BlockType bt)
{
    const auto& def = getBlockDef(bt);
    int tileIdx;
    if (face == 2) tileIdx = def.uvTop;
    else if (face == 3) tileIdx = def.uvBottom;
    else tileIdx = def.uvSide;

    float u0, v0, u1, v1;
    faceUV(tileIdx, u0, v0, u1, v1);

    float nx = kFaceNormals[face][0];
    float ny = kFaceNormals[face][1];
    float nz = kFaceNormals[face][2];

    // UV corners for the quad
    float uvs[4][2] = {{u0,v1},{u1,v1},{u1,v0},{u0,v0}};

    // Two triangles (indices 0,1,2 and 0,2,3)
    int tri[6] = {0,1,2, 0,2,3};
    for (int i : tri) {
        Vertex v;
        v.x = bx + kFaceQuads[face][i][0];
        v.y = by + kFaceQuads[face][i][1];
        v.z = bz + kFaceQuads[face][i][2];
        v.nx = nx; v.ny = ny; v.nz = nz;
        v.u = uvs[i][0];
        v.v = uvs[i][1];
        verts.push_back(v);
    }
}

std::vector<Vertex> MeshBuilder::buildChunkMesh(const Chunk& chunk, const World& world) {
    std::vector<Vertex> verts;
    verts.reserve(4096);

    int ox = chunk.coord.cx * CHUNK_SIZE;
    int oz = chunk.coord.cz * CHUNK_SIZE;

    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int y = 0; y < CHUNK_HEIGHT; ++y) {
            for (int z = 0; z < CHUNK_SIZE; ++z) {
                BlockType bt = chunk.getBlock(x, y, z);
                if (isAir(bt)) continue;

                float wx = (float)(ox + x);
                float wy = (float)y;
                float wz = (float)(oz + z);

                for (int face = 0; face < 6; ++face) {
                    int nx_ = ox + x + kNeighbourDX[face];
                    int ny_ =      y + kNeighbourDY[face];
                    int nz_ = oz + z + kNeighbourDZ[face];

                    BlockType neighbour = world.getBlock(nx_, ny_, nz_);
                    if (!isSolid(neighbour)) {
                        emitFace(verts, wx, wy, wz, face, bt);
                    }
                }
            }
        }
    }

    return verts;
}

} // namespace minemc

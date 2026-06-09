#pragma once
#include "Shader.h"
#include "world/World.h"
#include <GLES3/gl3.h>
#include <android/asset_manager.h>

// Simple 4x4 matrix math (row-major, compatible with GL column-major via transpose flag)
#include <cmath>
#include <cstring>

namespace minemc {

// Minimal matrix type to avoid external dependency
struct Mat4 {
    float m[16]{};

    static Mat4 identity();
    static Mat4 perspective(float fovY, float aspect, float near, float far);
    static Mat4 lookAt(float ex, float ey, float ez,
                       float cx, float cy, float cz,
                       float ux, float uy, float uz);
    Mat4 operator*(const Mat4& o) const;
};

struct Vec3 { float x, y, z; };

class Renderer {
public:
    bool init(AAssetManager* am, int width, int height);
    void onSizeChanged(int width, int height);
    void renderFrame(const World& world, const Vec3& camPos, float yaw, float pitch);
    void destroy();
    void uploadDirtyChunks(World& world);

private:
    Shader mShader;
    GLuint mTexAtlas = 0;
    int mWidth = 0, mHeight = 0;

    bool createTextureAtlas();
    void uploadChunkMesh(Chunk& chunk, const World& world);
    void drawChunk(const Chunk& chunk, const Mat4& vp);
};

} // namespace minemc

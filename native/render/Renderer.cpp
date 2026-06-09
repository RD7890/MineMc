#include "Renderer.h"
#include "Mesh.h"
#include <android/log.h>
#include <cmath>
#include <cstring>
#define TAG "MineMc/Renderer"

namespace minemc {

// ---------------------------------------------------------------------------
// Mat4 implementation
// ---------------------------------------------------------------------------

Mat4 Mat4::identity() {
    Mat4 m;
    m.m[0]=1; m.m[5]=1; m.m[10]=1; m.m[15]=1;
    return m;
}

Mat4 Mat4::operator*(const Mat4& o) const {
    Mat4 r;
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            float s = 0;
            for (int k = 0; k < 4; ++k)
                s += m[row*4+k] * o.m[k*4+col];
            r.m[row*4+col] = s;
        }
    }
    return r;
}

Mat4 Mat4::perspective(float fovY, float aspect, float near, float far) {
    Mat4 m;
    float f = 1.0f / std::tan(fovY * 0.5f);
    m.m[0]  = f / aspect;
    m.m[5]  = f;
    m.m[10] = (far + near) / (near - far);
    m.m[11] = -1.0f;
    m.m[14] = (2.0f * far * near) / (near - far);
    return m;
}

Mat4 Mat4::lookAt(float ex, float ey, float ez,
                   float cx, float cy, float cz,
                   float ux, float uy, float uz)
{
    // Forward
    float fx = cx-ex, fy = cy-ey, fz = cz-ez;
    float fl = std::sqrt(fx*fx+fy*fy+fz*fz);
    fx/=fl; fy/=fl; fz/=fl;

    // Right = forward x up
    float rx = fy*uz - fz*uy;
    float ry = fz*ux - fx*uz;
    float rz = fx*uy - fy*ux;
    float rl = std::sqrt(rx*rx+ry*ry+rz*rz);
    rx/=rl; ry/=rl; rz/=rl;

    // Up = right x forward
    float upx = ry*fz - rz*fy;
    float upy = rz*fx - rx*fz;
    float upz = rx*fy - ry*fx;

    Mat4 m;
    m.m[ 0]=rx; m.m[ 1]=upx; m.m[ 2]=-fx; m.m[ 3]=0;
    m.m[ 4]=ry; m.m[ 5]=upy; m.m[ 6]=-fy; m.m[ 7]=0;
    m.m[ 8]=rz; m.m[ 9]=upz; m.m[10]=-fz; m.m[11]=0;
    m.m[12]=-(rx*ex+ry*ey+rz*ez);
    m.m[13]=-(upx*ex+upy*ey+upz*ez);
    m.m[14]= (fx*ex+fy*ey+fz*ez);
    m.m[15]=1;
    return m;
}

// ---------------------------------------------------------------------------
// Renderer
// ---------------------------------------------------------------------------

bool Renderer::init(AAssetManager* am, int width, int height) {
    mWidth  = width;
    mHeight = height;

    if (!mShader.loadFromAssets(am, "shaders/voxel.vert", "shaders/voxel.frag")) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Failed to load shaders");
        return false;
    }

    if (!createTextureAtlas()) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Failed to create texture atlas");
        return false;
    }

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    glViewport(0, 0, width, height);
    __android_log_print(ANDROID_LOG_INFO, TAG, "Renderer init OK (%dx%d)", width, height);
    return true;
}

void Renderer::onSizeChanged(int width, int height) {
    mWidth = width; mHeight = height;
    glViewport(0, 0, width, height);
}

bool Renderer::createTextureAtlas() {
    // Procedural atlas: 4 tiles × 1 row, each 16×16 px = 64×16 RGBA
    constexpr int TILE = 16;
    constexpr int COLS = 4;
    constexpr int W = TILE * COLS;
    constexpr int H = TILE;

    struct RGBA { uint8_t r, g, b, a; };
    RGBA pixels[H][W];

    // Tile colours + simple checker pattern
    struct TileInfo { uint8_t r, g, b; uint8_t r2, g2, b2; };
    TileInfo tiles[COLS] = {
        { 85, 170,  50,  50,120,  20 },  // 0: grass top
        { 85, 170,  50,  80, 55,  30 },  // 1: grass side
        { 90,  60,  30,  70, 50,  25 },  // 2: dirt
        {130, 130, 130, 110,110, 110 },  // 3: stone
    };

    for (int col = 0; col < COLS; ++col) {
        auto& ti = tiles[col];
        for (int y = 0; y < TILE; ++y) {
            for (int x = 0; x < TILE; ++x) {
                bool checker = ((x / 4 + y / 4) % 2) == 0;
                auto& px = pixels[y][col * TILE + x];
                px.r = checker ? ti.r : ti.r2;
                px.g = checker ? ti.g : ti.g2;
                px.b = checker ? ti.b : ti.b2;
                px.a = 255;
            }
        }
    }

    glGenTextures(1, &mTexAtlas);
    glBindTexture(GL_TEXTURE_2D, mTexAtlas);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    return mTexAtlas != 0;
}

void Renderer::uploadChunkMesh(Chunk& chunk, const World& world) {
    auto verts = MeshBuilder::buildChunkMesh(chunk, world);
    chunk.vertexCount = static_cast<int>(verts.size());

    if (chunk.vertexCount == 0) {
        chunk.meshDirty   = false;
        chunk.gpuUploaded = true;
        return;
    }

    if (!chunk.vao) {
        glGenVertexArrays(1, &chunk.vao);
        glGenBuffers(1, &chunk.vbo);
    }

    glBindVertexArray(chunk.vao);
    glBindBuffer(GL_ARRAY_BUFFER, chunk.vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 chunk.vertexCount * sizeof(Vertex),
                 verts.data(), GL_DYNAMIC_DRAW);

    // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, x));
    // normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, nx));
    // uv
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                          (void*)offsetof(Vertex, u));

    glBindVertexArray(0);

    chunk.meshDirty   = false;
    chunk.gpuUploaded = true;
}

void Renderer::uploadDirtyChunks(World& world) {
    for (Chunk* ch : world.getAllChunks()) {
        if (ch->meshDirty) {
            uploadChunkMesh(*ch, world);
        }
    }
}

void Renderer::drawChunk(const Chunk& chunk, const Mat4& vp) {
    if (!chunk.gpuUploaded || chunk.vertexCount == 0 || !chunk.vao) return;

    // Model matrix: translate to chunk world origin
    Mat4 model = Mat4::identity();
    // Already baked into vertex positions — no per-chunk translation needed
    (void)model;

    mShader.use();
    glUniformMatrix4fv(mShader.loc("uMVP"), 1, GL_FALSE, vp.m);

    glBindVertexArray(chunk.vao);
    glDrawArrays(GL_TRIANGLES, 0, chunk.vertexCount);
    glBindVertexArray(0);
}

void Renderer::renderFrame(const World& world,
                            const Vec3& camPos,
                            float yaw, float pitch)
{
    glClearColor(0.4f, 0.7f, 1.0f, 1.0f); // sky blue
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    float aspect = (mHeight > 0) ? (float)mWidth / mHeight : 1.0f;
    float fovRad = 70.0f * 3.14159f / 180.0f;
    Mat4 proj = Mat4::perspective(fovRad, aspect, 0.1f, 512.0f);

    // Camera direction from yaw/pitch
    float cy = std::cos(yaw), sy = std::sin(yaw);
    float cp = std::cos(pitch), sp = std::sin(pitch);
    float dx = cy * cp, dy = sp, dz = sy * cp;

    Mat4 view = Mat4::lookAt(
        camPos.x, camPos.y, camPos.z,
        camPos.x + dx, camPos.y + dy, camPos.z + dz,
        0, 1, 0
    );

    Mat4 vp = proj * view;

    // Bind atlas
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTexAtlas);
    mShader.use();
    glUniform1i(mShader.loc("uTex"), 0);

    // Draw all chunks
    for (const Chunk* ch : const_cast<World&>(world).getAllChunks()) {
        drawChunk(*ch, vp);
    }
}

void Renderer::destroy() {
    mShader.destroy();
    if (mTexAtlas) { glDeleteTextures(1, &mTexAtlas); mTexAtlas = 0; }
}

} // namespace minemc

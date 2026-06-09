#include "WorldSerializer.h"
#include <cstdio>
#include <android/log.h>
#include <sys/stat.h>
#define TAG "MineMc/Serializer"

namespace minemc {

bool WorldSerializer::fileExists(const std::string& path) {
    struct stat st{};
    return stat(path.c_str(), &st) == 0;
}

bool WorldSerializer::save(const World& world, const std::string& path) {
    FILE* f = fopen(path.c_str(), "wb");
    if (!f) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Cannot open for write: %s", path.c_str());
        return false;
    }

    auto chunks = const_cast<World&>(world).getAllChunks();

    uint32_t magic = MAGIC;
    uint32_t count = static_cast<uint32_t>(chunks.size());
    fwrite(&magic, 4, 1, f);
    fwrite(&count, 4, 1, f);

    for (auto* ch : chunks) {
        int32_t cx = ch->coord.cx;
        int32_t cz = ch->coord.cz;
        fwrite(&cx, 4, 1, f);
        fwrite(&cz, 4, 1, f);
        fwrite(ch->blocks.data(), sizeof(BlockType), ch->blocks.size(), f);
    }

    fclose(f);
    __android_log_print(ANDROID_LOG_INFO, TAG, "Saved %u chunks to %s", count, path.c_str());
    return true;
}

bool WorldSerializer::load(World& world, const std::string& path) {
    FILE* f = fopen(path.c_str(), "rb");
    if (!f) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Cannot open for read: %s", path.c_str());
        return false;
    }

    uint32_t magic = 0, count = 0;
    if (fread(&magic, 4, 1, f) != 1 || magic != MAGIC) {
        __android_log_print(ANDROID_LOG_ERROR, TAG, "Bad magic in save file");
        fclose(f);
        return false;
    }
    fread(&count, 4, 1, f);

    world.clear();

    for (uint32_t i = 0; i < count; ++i) {
        int32_t cx, cz;
        fread(&cx, 4, 1, f);
        fread(&cz, 4, 1, f);

        Chunk* ch = world.getChunk(cx, cz);
        fread(ch->blocks.data(), sizeof(BlockType), ch->blocks.size(), f);
        ch->meshDirty = true;
    }

    fclose(f);
    __android_log_print(ANDROID_LOG_INFO, TAG, "Loaded %u chunks from %s", count, path.c_str());
    return true;
}

} // namespace minemc

#pragma once
#include <GLES3/gl3.h>
#include <string>
#include <android/asset_manager.h>
#include <android/log.h>
#define SHADER_TAG "MineMc/Shader"

namespace minemc {

class Shader {
public:
    GLuint program = 0;

    bool loadFromAssets(AAssetManager* am,
                        const char* vertPath,
                        const char* fragPath)
    {
        std::string vsrc = readAsset(am, vertPath);
        std::string fsrc = readAsset(am, fragPath);
        if (vsrc.empty() || fsrc.empty()) return false;
        return compile(vsrc, fsrc);
    }

    bool compile(const std::string& vsrc, const std::string& fsrc) {
        GLuint vs = compileShader(GL_VERTEX_SHADER, vsrc);
        GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsrc);
        if (!vs || !fs) return false;

        program = glCreateProgram();
        glAttachShader(program, vs);
        glAttachShader(program, fs);
        glLinkProgram(program);

        GLint ok = 0;
        glGetProgramiv(program, GL_LINK_STATUS, &ok);
        if (!ok) {
            char log[512];
            glGetProgramInfoLog(program, 512, nullptr, log);
            __android_log_print(ANDROID_LOG_ERROR, SHADER_TAG, "Link error: %s", log);
            glDeleteProgram(program);
            program = 0;
        }
        glDeleteShader(vs);
        glDeleteShader(fs);
        return program != 0;
    }

    void use() const { glUseProgram(program); }

    GLint loc(const char* name) const {
        return glGetUniformLocation(program, name);
    }

    void destroy() {
        if (program) { glDeleteProgram(program); program = 0; }
    }

private:
    static GLuint compileShader(GLenum type, const std::string& src) {
        GLuint s = glCreateShader(type);
        const char* p = src.c_str();
        glShaderSource(s, 1, &p, nullptr);
        glCompileShader(s);
        GLint ok = 0;
        glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
        if (!ok) {
            char log[512];
            glGetShaderInfoLog(s, 512, nullptr, log);
            __android_log_print(ANDROID_LOG_ERROR, SHADER_TAG,
                "%s shader error: %s",
                type == GL_VERTEX_SHADER ? "Vertex" : "Fragment", log);
            glDeleteShader(s);
            return 0;
        }
        return s;
    }

    static std::string readAsset(AAssetManager* am, const char* path) {
        AAsset* asset = AAssetManager_open(am, path, AASSET_MODE_BUFFER);
        if (!asset) {
            __android_log_print(ANDROID_LOG_ERROR, SHADER_TAG, "Asset not found: %s", path);
            return {};
        }
        size_t len = AAsset_getLength(asset);
        std::string out(len, '\0');
        AAsset_read(asset, out.data(), len);
        AAsset_close(asset);
        return out;
    }
};

} // namespace minemc

#include "AndroidBridge.h"
#include <android/asset_manager_jni.h>
#include <android/log.h>
#include <string>
#define TAG "MineMc/JNI"

std::unique_ptr<minemc::GameEngine> gEngine;

extern "C" {

// ---------------------------------------------------------------------------
// com.ryzix.minemc.NativeBridge
// ---------------------------------------------------------------------------

JNIEXPORT void JNICALL
Java_com_ryzix_minemc_NativeBridge_nativeInit(
    JNIEnv* env, jobject /*thiz*/,
    jobject assetManager, jstring savePath)
{
    AAssetManager* am = AAssetManager_fromJava(env, assetManager);
    const char* sp = env->GetStringUTFChars(savePath, nullptr);
    std::string saveStr(sp);
    env->ReleaseStringUTFChars(savePath, sp);

    if (!gEngine) gEngine = std::make_unique<minemc::GameEngine>();
    // Width/height will be set in nativeSurfaceChanged; use 1280×720 as placeholder
    gEngine->init(am, saveStr, 1280, 720);
    __android_log_print(ANDROID_LOG_INFO, TAG, "nativeInit complete");
}

JNIEXPORT void JNICALL
Java_com_ryzix_minemc_NativeBridge_nativeSurfaceCreated(
    JNIEnv* /*env*/, jobject /*thiz*/)
{
    if (gEngine) gEngine->onSurfaceCreated();
}

JNIEXPORT void JNICALL
Java_com_ryzix_minemc_NativeBridge_nativeSurfaceChanged(
    JNIEnv* /*env*/, jobject /*thiz*/,
    jint width, jint height)
{
    if (gEngine) gEngine->onSurfaceChanged(width, height);
}

JNIEXPORT void JNICALL
Java_com_ryzix_minemc_NativeBridge_nativeDrawFrame(
    JNIEnv* /*env*/, jobject /*thiz*/)
{
    if (gEngine) gEngine->drawFrame();
}

JNIEXPORT void JNICALL
Java_com_ryzix_minemc_NativeBridge_nativePause(
    JNIEnv* /*env*/, jobject /*thiz*/)
{
    if (gEngine) gEngine->pause();
}

JNIEXPORT void JNICALL
Java_com_ryzix_minemc_NativeBridge_nativeResume(
    JNIEnv* /*env*/, jobject /*thiz*/)
{
    if (gEngine) gEngine->resume();
}

JNIEXPORT void JNICALL
Java_com_ryzix_minemc_NativeBridge_nativeDestroy(
    JNIEnv* /*env*/, jobject /*thiz*/)
{
    if (gEngine) {
        gEngine->destroy();
        gEngine.reset();
    }
    __android_log_print(ANDROID_LOG_INFO, TAG, "Engine destroyed");
}

JNIEXPORT void JNICALL
Java_com_ryzix_minemc_NativeBridge_nativeTouchEvent(
    JNIEnv* /*env*/, jobject /*thiz*/,
    jint action, jint pointerId, jfloat x, jfloat y)
{
    if (gEngine) gEngine->onTouch(action, pointerId, x, y);
}

JNIEXPORT void JNICALL
Java_com_ryzix_minemc_NativeBridge_nativeNewWorld(
    JNIEnv* /*env*/, jobject /*thiz*/)
{
    if (gEngine) gEngine->newWorld();
}

JNIEXPORT void JNICALL
Java_com_ryzix_minemc_NativeBridge_nativeSaveWorld(
    JNIEnv* /*env*/, jobject /*thiz*/)
{
    if (gEngine) gEngine->saveWorld();
}

JNIEXPORT jstring JNICALL
Java_com_ryzix_minemc_NativeBridge_nativeGetDebugInfo(
    JNIEnv* env, jobject /*thiz*/)
{
    std::string info = gEngine ? gEngine->debugInfo() : "Engine not ready";
    return env->NewStringUTF(info.c_str());
}

JNIEXPORT jint JNICALL
Java_com_ryzix_minemc_NativeBridge_nativeSavedWorldExists(
    JNIEnv* env, jobject /*thiz*/, jstring savePath)
{
    const char* sp = env->GetStringUTFChars(savePath, nullptr);
    bool exists = minemc::WorldSerializer::fileExists(sp);
    env->ReleaseStringUTFChars(savePath, sp);
    return exists ? 1 : 0;
}

} // extern "C"

#pragma once
#include "core/GameEngine.h"
#include <jni.h>
#include <memory>

// Global engine instance (accessed from JNI callbacks)
extern std::unique_ptr<minemc::GameEngine> gEngine;

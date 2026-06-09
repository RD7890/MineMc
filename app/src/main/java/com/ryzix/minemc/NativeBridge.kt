package com.ryzix.minemc

/**
 * JNI bridge to the native C++ game engine.
 * All methods map 1:1 to implementations in platform/AndroidBridge.cpp.
 */
object NativeBridge {

    init {
        System.loadLibrary("minemc")
    }

    // --- Lifecycle ---

    /** Create and initialise the engine. Call once after GL surface is ready. */
    external fun nativeInit(assetManager: android.content.res.AssetManager, savePath: String)

    /** Called when the GL surface is created/recreated. */
    external fun nativeSurfaceCreated()

    /** Called when the GL surface changes size. */
    external fun nativeSurfaceChanged(width: Int, height: Int)

    /** Advance one frame and render. Called from the GL thread. */
    external fun nativeDrawFrame()

    /** Pause the game loop (app goes to background). */
    external fun nativePause()

    /** Resume the game loop. */
    external fun nativeResume()

    /** Save world and release all native resources. */
    external fun nativeDestroy()

    // --- Input ---

    /**
     * Forward a touch event to the native input handler.
     * @param action  MotionEvent.ACTION_DOWN / ACTION_MOVE / ACTION_UP / ACTION_POINTER_DOWN / ACTION_POINTER_UP
     * @param pointerId  Pointer id
     * @param x  X in screen pixels
     * @param y  Y in screen pixels
     */
    external fun nativeTouchEvent(action: Int, pointerId: Int, x: Float, y: Float)

    // --- World ---

    /** Delete the saved world data and start fresh. */
    external fun nativeNewWorld()

    /** Trigger an immediate world save to the save path given in nativeInit. */
    external fun nativeSaveWorld()

    // --- Query ---

    /** Returns a formatted debug string: "FPS:60 X:0.0 Y:65.0 Z:0.0 Chunk:0,0" */
    external fun nativeGetDebugInfo(): String

    /** Returns 1 if a saved world file exists at savePath, 0 otherwise. */
    external fun nativeSavedWorldExists(savePath: String): Int
}

package com.ryzix.minemc

import android.content.Context
import android.opengl.GLSurfaceView
import javax.microedition.khronos.egl.EGLConfig
import javax.microedition.khronos.opengles.GL10

/**
 * OpenGL ES 3.0 surface that drives the native render loop.
 */
class GameSurfaceView(context: Context) : GLSurfaceView(context) {

    private val renderer = GameRenderer()

    init {
        setEGLContextClientVersion(3)
        setEGLConfigChooser(8, 8, 8, 0, 24, 0)
        setRenderer(renderer)
        renderMode = RENDERMODE_CONTINUOUSLY
        preserveEGLContextOnPause = true
    }

    // --- Touch forwarding --------------------------------------------------
    // Touch events are handled by GameOverlayView (sits on top). The overlay
    // calls NativeBridge.nativeTouchEvent directly so we don't need to
    // intercept here.

    fun pauseRendering() {
        onPause()
        queueEvent { NativeBridge.nativePause() }
    }

    fun resumeRendering() {
        onResume()
        queueEvent { NativeBridge.nativeResume() }
    }

    // --- Renderer ----------------------------------------------------------

    private inner class GameRenderer : Renderer {

        override fun onSurfaceCreated(gl: GL10?, config: EGLConfig?) {
            NativeBridge.nativeSurfaceCreated()
        }

        override fun onSurfaceChanged(gl: GL10?, width: Int, height: Int) {
            NativeBridge.nativeSurfaceChanged(width, height)
        }

        override fun onDrawFrame(gl: GL10?) {
            NativeBridge.nativeDrawFrame()
        }
    }
}

package com.ryzix.minemc

import android.content.Context
import android.content.SharedPreferences

/**
 * Simple wrapper around SharedPreferences for persisting game settings.
 */
class Settings(context: Context) {

    private val prefs: SharedPreferences =
        context.getSharedPreferences(PREFS_NAME, Context.MODE_PRIVATE)

    var renderDistance: Int
        get() = prefs.getInt(KEY_RENDER_DIST, DEFAULT_RENDER_DIST)
        set(v) = prefs.edit().putInt(KEY_RENDER_DIST, v.coerceIn(2, 12)).apply()

    var touchSensitivity: Float
        get() = prefs.getFloat(KEY_TOUCH_SENS, DEFAULT_TOUCH_SENS)
        set(v) = prefs.edit().putFloat(KEY_TOUCH_SENS, v.coerceIn(0.1f, 3.0f)).apply()

    var showDebugInfo: Boolean
        get() = prefs.getBoolean(KEY_DEBUG, DEFAULT_DEBUG)
        set(v) = prefs.edit().putBoolean(KEY_DEBUG, v).apply()

    companion object {
        private const val PREFS_NAME = "minemc_settings"
        private const val KEY_RENDER_DIST = "render_distance"
        private const val KEY_TOUCH_SENS = "touch_sensitivity"
        private const val KEY_DEBUG = "show_debug"

        private const val DEFAULT_RENDER_DIST = 4
        private const val DEFAULT_TOUCH_SENS = 1.0f
        private const val DEFAULT_DEBUG = true
    }
}

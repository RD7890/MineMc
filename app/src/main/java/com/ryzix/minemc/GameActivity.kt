package com.ryzix.minemc

import android.os.Bundle
import android.os.Handler
import android.os.Looper
import android.view.View
import android.view.WindowInsets
import android.view.WindowInsetsController
import android.view.WindowManager
import androidx.appcompat.app.AppCompatActivity
import com.ryzix.minemc.databinding.ActivityGameBinding

/**
 * Hosts the GL surface and HUD overlay. Manages the game lifecycle.
 */
class GameActivity : AppCompatActivity() {

    private lateinit var binding: ActivityGameBinding
    private lateinit var glView: GameSurfaceView
    private lateinit var settings: Settings

    private val hudHandler = Handler(Looper.getMainLooper())
    private val hudRunnable = object : Runnable {
        override fun run() {
            if (settings.showDebugInfo) {
                val info = NativeBridge.nativeGetDebugInfo()
                binding.gameOverlay.debugText = info
                binding.gameOverlay.showDebug = true
                binding.gameOverlay.invalidate()
            }
            hudHandler.postDelayed(this, HUD_UPDATE_MS)
        }
    }

    private var isPaused = false

    override fun onCreate(savedInstanceState: Bundle?) {
        // Set fullscreen flags BEFORE super.onCreate so the DecorView is
        // created fullscreen. Calling insetsController before this causes
        // a NullPointerException on many devices (MIUI, etc.).
        @Suppress("DEPRECATION")
        window.setFlags(
            WindowManager.LayoutParams.FLAG_FULLSCREEN or
            WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON,
            WindowManager.LayoutParams.FLAG_FULLSCREEN or
            WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON
        )
        super.onCreate(savedInstanceState)
        binding = ActivityGameBinding.inflate(layoutInflater)
        setContentView(binding.root)

        // DecorView is now attached — safe to call insetsController
        hideSystemUI()

        settings = Settings(this)

        glView = GameSurfaceView(this)
        binding.glContainer.addView(glView)

        val saveDir = getExternalFilesDir(null)?.absolutePath ?: filesDir.absolutePath
        val savePath = "$saveDir/world.dat"
        NativeBridge.nativeInit(assets, savePath)

        val newWorld = intent.getBooleanExtra(EXTRA_NEW_WORLD, false)
        if (newWorld) {
            NativeBridge.nativeNewWorld()
        }

        binding.btnResume.setOnClickListener { resumeGame() }
        binding.btnSaveQuit.setOnClickListener {
            NativeBridge.nativeSaveWorld()
            finish()
        }

        onBackPressedDispatcher.addCallback(this,
            object : androidx.activity.OnBackPressedCallback(true) {
                override fun handleOnBackPressed() {
                    if (isPaused) resumeGame() else pauseGame()
                }
            }
        )
    }

    override fun onResume() {
        super.onResume()
        hideSystemUI()
        glView.resumeRendering()
        if (!isPaused) hudHandler.post(hudRunnable)
    }

    override fun onPause() {
        super.onPause()
        glView.pauseRendering()
        hudHandler.removeCallbacks(hudRunnable)
    }

    override fun onDestroy() {
        super.onDestroy()
        NativeBridge.nativeDestroy()
    }

    private fun pauseGame() {
        isPaused = true
        binding.pauseMenu.visibility = View.VISIBLE
        glView.pauseRendering()
        hudHandler.removeCallbacks(hudRunnable)
    }

    private fun resumeGame() {
        isPaused = false
        binding.pauseMenu.visibility = View.GONE
        glView.resumeRendering()
        hudHandler.post(hudRunnable)
    }

    private fun hideSystemUI() {
        try {
            if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.R) {
                val controller = window.insetsController ?: return
                controller.hide(
                    WindowInsets.Type.statusBars() or WindowInsets.Type.navigationBars()
                )
                controller.systemBarsBehavior =
                    WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
            } else {
                @Suppress("DEPRECATION")
                window.decorView.systemUiVisibility = (
                    View.SYSTEM_UI_FLAG_FULLSCREEN
                    or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                    or View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
                    or View.SYSTEM_UI_FLAG_LAYOUT_STABLE
                    or View.SYSTEM_UI_FLAG_LAYOUT_FULLSCREEN
                    or View.SYSTEM_UI_FLAG_LAYOUT_HIDE_NAVIGATION
                )
            }
        } catch (e: Exception) {
            // Some OEM ROMs (MIUI, One UI, etc.) can throw here — window still works
        }
    }

    companion object {
        const val EXTRA_NEW_WORLD = "new_world"
        private const val HUD_UPDATE_MS = 250L
    }
}

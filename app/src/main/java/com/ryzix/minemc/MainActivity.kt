package com.ryzix.minemc

import android.content.Intent
import android.os.Bundle
import android.view.View
import android.view.WindowInsets
import android.view.WindowInsetsController
import androidx.appcompat.app.AppCompatActivity
import com.ryzix.minemc.databinding.ActivityMainBinding

/**
 * Main menu activity. Shows Continue / New World / Quit.
 */
class MainActivity : AppCompatActivity() {

    private lateinit var binding: ActivityMainBinding
    private lateinit var settings: Settings

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        hideSystemUI()
        binding = ActivityMainBinding.inflate(layoutInflater)
        setContentView(binding.root)
        settings = Settings(this)

        val saveDir = getExternalFilesDir(null)?.absolutePath
            ?: filesDir.absolutePath
        val savePath = "$saveDir/world.dat"

        // Disable Continue if no save exists
        val hasSave = NativeBridge.nativeSavedWorldExists(savePath) != 0
        binding.btnContinue.isEnabled = hasSave
        binding.btnContinue.alpha = if (hasSave) 1f else 0.4f

        binding.btnContinue.setOnClickListener {
            launchGame(newWorld = false)
        }
        binding.btnNewWorld.setOnClickListener {
            launchGame(newWorld = true)
        }
        binding.btnQuit.setOnClickListener {
            finishAffinity()
        }
    }

    override fun onResume() {
        super.onResume()
        hideSystemUI()
    }

    private fun launchGame(newWorld: Boolean) {
        val intent = Intent(this, GameActivity::class.java)
        intent.putExtra(GameActivity.EXTRA_NEW_WORLD, newWorld)
        startActivity(intent)
    }

    private fun hideSystemUI() {
        if (android.os.Build.VERSION.SDK_INT >= android.os.Build.VERSION_CODES.R) {
            window.insetsController?.let {
                it.hide(WindowInsets.Type.statusBars() or WindowInsets.Type.navigationBars())
                it.systemBarsBehavior =
                    WindowInsetsController.BEHAVIOR_SHOW_TRANSIENT_BARS_BY_SWIPE
            }
        } else {
            @Suppress("DEPRECATION")
            window.decorView.systemUiVisibility = (
                View.SYSTEM_UI_FLAG_FULLSCREEN
                or View.SYSTEM_UI_FLAG_HIDE_NAVIGATION
                or View.SYSTEM_UI_FLAG_IMMERSIVE_STICKY
            )
        }
    }
}

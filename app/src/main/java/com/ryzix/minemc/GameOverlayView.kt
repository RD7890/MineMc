package com.ryzix.minemc

import android.annotation.SuppressLint
import android.content.Context
import android.graphics.Canvas
import android.graphics.Color
import android.graphics.Paint
import android.util.AttributeSet
import android.view.MotionEvent
import android.view.View

/**
 * Transparent overlay that:
 *  - Draws the crosshair and debug HUD
 *  - Handles all touch input and forwards to NativeBridge
 *
 * Zones (landscape):
 *  - Left 35% of screen → virtual joystick (move)
 *  - Right 65% of screen → look/swipe
 *  - Jump button: bottom-right corner
 *  - Break button (tap left-side screen, held = break)
 *  - Place button: secondary tap on right side
 */
class GameOverlayView @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null
) : View(context, attrs) {

    // --- Paints ---
    private val crosshairPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.WHITE
        strokeWidth = 3f
        alpha = 200
    }
    private val debugPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.WHITE
        textSize = 28f
        setShadowLayer(2f, 1f, 1f, Color.BLACK)
    }
    private val joystickOuterPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.WHITE
        alpha = 70
        style = Paint.Style.STROKE
        strokeWidth = 4f
    }
    private val joystickInnerPaint = Paint(Paint.ANTI_ALIAS_FLAG).apply {
        color = Color.WHITE
        alpha = 140
        style = Paint.Style.FILL
    }

    // --- State ---
    var debugText: String = ""
    var showDebug: Boolean = true

    // Joystick tracking
    private var joystickPointerId = -1
    private var joystickBaseX = 0f
    private var joystickBaseY = 0f
    private var joystickThumbX = 0f
    private var joystickThumbY = 0f
    private val joystickRadius = 120f

    override fun onDraw(canvas: Canvas) {
        super.onDraw(canvas)
        val cx = width / 2f
        val cy = height / 2f

        // Crosshair
        val crossSize = 30f
        canvas.drawLine(cx - crossSize, cy, cx + crossSize, cy, crosshairPaint)
        canvas.drawLine(cx, cy - crossSize, cx, cy + crossSize, crosshairPaint)

        // Joystick
        if (joystickPointerId != -1) {
            canvas.drawCircle(joystickBaseX, joystickBaseY, joystickRadius, joystickOuterPaint)
            canvas.drawCircle(joystickThumbX, joystickThumbY, 40f, joystickInnerPaint)
        } else {
            // Show ghost joystick in resting position
            val ghostX = width * 0.15f
            val ghostY = height * 0.70f
            canvas.drawCircle(ghostX, ghostY, joystickRadius, joystickOuterPaint)
            canvas.drawCircle(ghostX, ghostY, 40f, joystickInnerPaint)
        }

        // Debug HUD
        if (showDebug && debugText.isNotEmpty()) {
            var lineY = 48f
            debugText.split("\n").forEach { line ->
                canvas.drawText(line, 20f, lineY, debugPaint)
                lineY += 36f
            }
        }
    }

    @SuppressLint("ClickableViewAccessibility")
    override fun onTouchEvent(event: MotionEvent): Boolean {
        val action = event.actionMasked
        val pointerIndex = event.actionIndex
        val pointerId = event.getPointerId(pointerIndex)
        val x = event.getX(pointerIndex)
        val y = event.getY(pointerIndex)
        val leftZone = width * 0.35f

        when (action) {
            MotionEvent.ACTION_DOWN, MotionEvent.ACTION_POINTER_DOWN -> {
                if (x < leftZone && joystickPointerId == -1) {
                    // Start joystick
                    joystickPointerId = pointerId
                    joystickBaseX = x
                    joystickBaseY = y
                    joystickThumbX = x
                    joystickThumbY = y
                    invalidate()
                }
                NativeBridge.nativeTouchEvent(action, pointerId, x, y)
            }
            MotionEvent.ACTION_MOVE -> {
                for (i in 0 until event.pointerCount) {
                    val pid = event.getPointerId(i)
                    val px = event.getX(i)
                    val py = event.getY(i)
                    if (pid == joystickPointerId) {
                        val dx = px - joystickBaseX
                        val dy = py - joystickBaseY
                        val dist = Math.hypot(dx.toDouble(), dy.toDouble()).toFloat()
                        if (dist > joystickRadius) {
                            val scale = joystickRadius / dist
                            joystickThumbX = joystickBaseX + dx * scale
                            joystickThumbY = joystickBaseY + dy * scale
                        } else {
                            joystickThumbX = px
                            joystickThumbY = py
                        }
                        invalidate()
                    }
                    NativeBridge.nativeTouchEvent(MotionEvent.ACTION_MOVE, pid, px, py)
                }
            }
            MotionEvent.ACTION_UP, MotionEvent.ACTION_POINTER_UP -> {
                if (pointerId == joystickPointerId) {
                    joystickPointerId = -1
                    invalidate()
                }
                NativeBridge.nativeTouchEvent(action, pointerId, x, y)
            }
            MotionEvent.ACTION_CANCEL -> {
                joystickPointerId = -1
                invalidate()
                NativeBridge.nativeTouchEvent(action, pointerId, x, y)
            }
        }
        return true
    }
}

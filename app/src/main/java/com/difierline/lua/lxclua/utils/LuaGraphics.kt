package com.difierline.lua.lxclua.utils

import android.graphics.*
import android.view.SurfaceView
import android.view.SurfaceHolder
import android.view.MotionEvent
import android.view.View

class LuaGraphics(private val surfaceView: SurfaceView) {
    
    private val holder: SurfaceHolder = surfaceView.holder
    private val paint = Paint().apply {
        isAntiAlias = true
        isFilterBitmap = true
    }
    
    private val clickListeners = mutableListOf<ClickListener>()
    private var touchListener: View.OnTouchListener? = null
    
    init {
        setupDefaultTouchListener()
    }
    
    private fun setupDefaultTouchListener() {
        touchListener = View.OnTouchListener { _, event ->
            val x = event.x
            val y = event.y
            val action = event.action
            
            when (action) {
                MotionEvent.ACTION_DOWN -> {
                    notifyClickListeners(x, y, "down")
                }
                MotionEvent.ACTION_MOVE -> {
                    notifyClickListeners(x, y, "move")
                }
                MotionEvent.ACTION_UP -> {
                    notifyClickListeners(x, y, "up")
                }
                MotionEvent.ACTION_CANCEL -> {
                    notifyClickListeners(x, y, "cancel")
                }
            }
            true
        }
        surfaceView.setOnTouchListener(touchListener)
    }
    
    fun addClickListener(id: String, x: Float, y: Float, width: Float, height: Float, 
                         callback: String, longPress: Boolean = false) {
        clickListeners.add(ClickListener(id, x, y, width, height, callback, longPress))
    }
    
    fun removeClickListener(id: String) {
        clickListeners.removeAll { it.id == id }
    }
    
    fun clearClickListeners() {
        clickListeners.clear()
    }
    
    private fun notifyClickListeners(x: Float, y: Float, action: String) {
        for (listener in clickListeners) {
            if (x >= listener.x && x <= listener.x + listener.width &&
                y >= listener.y && y <= listener.y + listener.height) {
                listener.triggered = true
            } else if (action == "down") {
                listener.triggered = false
            }
            
            if (listener.triggered) {
                val luaCallback = "LuaEventCallback"
                try {
                    val callbackMethod = Class.forName("org.luaj.vm2.LuaValue")
                        .getDeclaredMethod("call", Array<Any>::class.java)
                    callbackMethod.invoke(null, arrayOf(listener.id, action))
                } catch (e: Exception) {
                }
            }
        }
    }
    
    data class ClickListener(
        val id: String,
        val x: Float,
        val y: Float,
        val width: Float,
        val height: Float,
        val callback: String,
        val longPress: Boolean,
        var triggered: Boolean = false
    )
    
    fun clearScreen(color: Int = 0xFF000000.toInt()) {
        val canvas = holder.lockCanvas() ?: return
        try {
            canvas.drawColor(color)
        } finally {
            holder.unlockCanvasAndPost(canvas)
        }
    }
    
    fun drawRect(x: Float, y: Float, width: Float, height: Float, color: Int) {
        val canvas = holder.lockCanvas() ?: return
        try {
            paint.style = Paint.Style.FILL
            paint.color = color
            canvas.drawRect(x, y, x + width, y + height, paint)
        } finally {
            holder.unlockCanvasAndPost(canvas)
        }
    }
    
    fun drawRoundRect(x: Float, y: Float, width: Float, height: Float, 
                      cornerRadius: Float, color: Int) {
        val canvas = holder.lockCanvas() ?: return
        try {
            paint.style = Paint.Style.FILL
            paint.color = color
            canvas.drawRoundRect(x, y, x + width, y + height, cornerRadius, cornerRadius, paint)
        } finally {
            holder.unlockCanvasAndPost(canvas)
        }
    }
    
    fun drawRoundRectStroke(x: Float, y: Float, width: Float, height: Float,
                            cornerRadius: Float, fillColor: Int, 
                            strokeColor: Int, strokeWidth: Float = 3f) {
        val canvas = holder.lockCanvas() ?: return
        try {
            paint.style = Paint.Style.FILL
            paint.color = fillColor
            canvas.drawRoundRect(x, y, x + width, y + height, cornerRadius, cornerRadius, paint)
            paint.style = Paint.Style.STROKE
            paint.color = strokeColor
            paint.strokeWidth = strokeWidth
            canvas.drawRoundRect(x, y, x + width, y + height, cornerRadius, cornerRadius, paint)
        } finally {
            holder.unlockCanvasAndPost(canvas)
        }
    }
    
    fun drawOval(x: Float, y: Float, width: Float, height: Float, color: Int) {
        val canvas = holder.lockCanvas() ?: return
        try {
            paint.style = Paint.Style.FILL
            paint.color = color
            canvas.drawOval(x, y, x + width, y + height, paint)
        } finally {
            holder.unlockCanvasAndPost(canvas)
        }
    }
    
    fun drawOvalStroke(x: Float, y: Float, width: Float, height: Float,
                       strokeWidth: Float, color: Int) {
        val canvas = holder.lockCanvas() ?: return
        try {
            paint.style = Paint.Style.STROKE
            paint.color = color
            paint.strokeWidth = strokeWidth
            canvas.drawOval(x, y, x + width, y + height, paint)
        } finally {
            holder.unlockCanvasAndPost(canvas)
        }
    }
    
    fun drawCircle(cx: Float, cy: Float, radius: Float, color: Int) {
        val canvas = holder.lockCanvas() ?: return
        try {
            paint.style = Paint.Style.FILL
            paint.color = color
            canvas.drawCircle(cx, cy, radius, paint)
        } finally {
            holder.unlockCanvasAndPost(canvas)
        }
    }
    
    fun drawCircleRing(cx: Float, cy: Float, radius: Float, 
                       strokeWidth: Float, color: Int) {
        val canvas = holder.lockCanvas() ?: return
        try {
            paint.style = Paint.Style.STROKE
            paint.color = color
            paint.strokeWidth = strokeWidth
            canvas.drawCircle(cx, cy, radius, paint)
        } finally {
            holder.unlockCanvasAndPost(canvas)
        }
    }
    
    fun drawArc(cx: Float, cy: Float, radius: Float, startAngle: Float, 
                sweepAngle: Float, useCenter: Boolean, color: Int) {
        val canvas = holder.lockCanvas() ?: return
        try {
            paint.style = Paint.Style.FILL
            paint.color = color
            canvas.drawArc(cx - radius, cy - radius, cx + radius, cy + radius, 
                          startAngle, sweepAngle, useCenter, paint)
        } finally {
            holder.unlockCanvasAndPost(canvas)
        }
    }
    
    fun drawArcRing(cx: Float, cy: Float, radius: Float, strokeWidth: Float,
                    startAngle: Float, sweepAngle: Float, color: Int) {
        val canvas = holder.lockCanvas() ?: return
        try {
            paint.style = Paint.Style.STROKE
            paint.color = color
            paint.strokeWidth = strokeWidth
            canvas.drawArc(cx - radius, cy - radius, cx + radius, cy + radius, 
                          startAngle, sweepAngle, false, paint)
        } finally {
            holder.unlockCanvasAndPost(canvas)
        }
    }
    
    fun drawLine(startX: Float, startY: Float, endX: Float, endY: Float,
                 color: Int, strokeWidth: Float = 2f) {
        val canvas = holder.lockCanvas() ?: return
        try {
            paint.style = Paint.Style.STROKE
            paint.color = color
            paint.strokeWidth = strokeWidth
            canvas.drawLine(startX, startY, endX, endY, paint)
        } finally {
            holder.unlockCanvasAndPost(canvas)
        }
    }
    
    fun drawLines(points: FloatArray, color: Int, strokeWidth: Float = 2f) {
        if (points.size < 4) return
        val canvas = holder.lockCanvas() ?: return
        try {
            paint.style = Paint.Style.STROKE
            paint.color = color
            paint.strokeWidth = strokeWidth
            canvas.drawLines(points, paint)
        } finally {
            holder.unlockCanvasAndPost(canvas)
        }
    }
    
    fun drawText(text: String, x: Float, y: Float, 
                 textSize: Float, color: Int) {
        val canvas = holder.lockCanvas() ?: return
        try {
            paint.textSize = textSize
            paint.color = color
            paint.textAlign = Paint.Align.LEFT
            paint.isAntiAlias = true
            canvas.drawText(text, x, y, paint)
        } finally {
            holder.unlockCanvasAndPost(canvas)
        }
    }
    
    fun drawCenterText(text: String, centerX: Float, centerY: Float,
                       textSize: Float, color: Int) {
        val canvas = holder.lockCanvas() ?: return
        try {
            paint.textSize = textSize
            paint.color = color
            paint.textAlign = Paint.Align.CENTER
            paint.isAntiAlias = true
            val textBounds = Rect()
            paint.getTextBounds(text, 0, text.length, textBounds)
            val textHeight = textBounds.height()
            canvas.drawText(text, centerX, centerY + textHeight / 2, paint)
        } finally {
            holder.unlockCanvasAndPost(canvas)
        }
    }
    
    fun drawTextRight(text: String, x: Float, y: Float,
                      textSize: Float, color: Int) {
        val canvas = holder.lockCanvas() ?: return
        try {
            paint.textSize = textSize
            paint.color = color
            paint.textAlign = Paint.Align.RIGHT
            paint.isAntiAlias = true
            canvas.drawText(text, x, y, paint)
        } finally {
            holder.unlockCanvasAndPost(canvas)
        }
    }
    
    fun drawPath(points: FloatArray, color: Int, strokeWidth: Float = 2f) {
        if (points.size < 4) return
        val canvas = holder.lockCanvas() ?: return
        try {
            val path = Path()
            path.moveTo(points[0], points[1])
            for (i in 2 until points.size step 2) {
                path.lineTo(points[i], points[i + 1])
            }
            paint.style = Paint.Style.STROKE
            paint.color = color
            paint.strokeWidth = strokeWidth
            paint.isAntiAlias = true
            canvas.drawPath(path, paint)
        } finally {
            holder.unlockCanvasAndPost(canvas)
        }
    }
    
    fun drawPathFill(points: FloatArray, color: Int) {
        if (points.size < 6) return
        val canvas = holder.lockCanvas() ?: return
        try {
            val path = Path()
            path.moveTo(points[0], points[1])
            for (i in 2 until points.size step 2) {
                path.lineTo(points[i], points[i + 1])
            }
            path.close()
            paint.style = Paint.Style.FILL
            paint.color = color
            paint.isAntiAlias = true
            canvas.drawPath(path, paint)
        } finally {
            holder.unlockCanvasAndPost(canvas)
        }
    }
    
    fun drawBezier(startX: Float, startY: Float, 
                   control1X: Float, control1Y: Float,
                   control2X: Float, control2Y: Float,
                   endX: Float, endY: Float,
                   color: Int, strokeWidth: Float = 2f) {
        val canvas = holder.lockCanvas() ?: return
        try {
            val path = Path()
            path.moveTo(startX, startY)
            path.cubicTo(control1X, control1Y, control2X, control2Y, endX, endY)
            paint.style = Paint.Style.STROKE
            paint.color = color
            paint.strokeWidth = strokeWidth
            paint.isAntiAlias = true
            canvas.drawPath(path, paint)
        } finally {
            holder.unlockCanvasAndPost(canvas)
        }
    }
    
    fun drawQuadraticBezier(startX: Float, startY: Float,
                            controlX: Float, controlY: Float,
                            endX: Float, endY: Float,
                            color: Int, strokeWidth: Float = 2f) {
        val canvas = holder.lockCanvas() ?: return
        try {
            val path = Path()
            path.moveTo(startX, startY)
            path.quadTo(controlX, controlY, endX, endY)
            paint.style = Paint.Style.STROKE
            paint.color = color
            paint.strokeWidth = strokeWidth
            paint.isAntiAlias = true
            canvas.drawPath(path, paint)
        } finally {
            holder.unlockCanvasAndPost(canvas)
        }
    }
    
    fun drawPolygon(points: FloatArray, color: Int) {
        if (points.size < 6) return
        val canvas = holder.lockCanvas() ?: return
        try {
            val path = Path()
            path.moveTo(points[0], points[1])
            for (i in 2 until points.size step 2) {
                path.lineTo(points[i], points[i + 1])
            }
            path.close()
            paint.style = Paint.Style.FILL
            paint.color = color
            paint.isAntiAlias = true
            canvas.drawPath(path, paint)
        } finally {
            holder.unlockCanvasAndPost(canvas)
        }
    }
    
    fun drawPolygonStroke(points: FloatArray, strokeWidth: Float, color: Int) {
        if (points.size < 6) return
        val canvas = holder.lockCanvas() ?: return
        try {
            val path = Path()
            path.moveTo(points[0], points[1])
            for (i in 2 until points.size step 2) {
                path.lineTo(points[i], points[i + 1])
            }
            path.close()
            paint.style = Paint.Style.STROKE
            paint.color = color
            paint.strokeWidth = strokeWidth
            paint.isAntiAlias = true
            canvas.drawPath(path, paint)
        } finally {
            holder.unlockCanvasAndPost(canvas)
        }
    }
    
    fun drawTriangle(x1: Float, y1: Float, x2: Float, y2: Float, 
                     x3: Float, y3: Float, color: Int) {
        val canvas = holder.lockCanvas() ?: return
        try {
            val path = Path()
            path.moveTo(x1, y1)
            path.lineTo(x2, y2)
            path.lineTo(x3, y3)
            path.close()
            paint.style = Paint.Style.FILL
            paint.color = color
            paint.isAntiAlias = true
            canvas.drawPath(path, paint)
        } finally {
            holder.unlockCanvasAndPost(canvas)
        }
    }
    
    fun drawRegularPolygon(cx: Float, cy: Float, radius: Float, 
                           sides: Int, rotation: Float = 0f, color: Int) {
        if (sides < 3) return
        val canvas = holder.lockCanvas() ?: return
        try {
            val path = Path()
            val angleStep = Math.PI * 2 / sides
            for (i in 0 until sides) {
                val angle = i * angleStep + Math.toRadians(rotation.toDouble())
                val px = cx + Math.cos(angle).toFloat() * radius
                val py = cy + Math.sin(angle).toFloat() * radius
                if (i == 0) {
                    path.moveTo(px, py)
                } else {
                    path.lineTo(px, py)
                }
            }
            path.close()
            paint.style = Paint.Style.FILL
            paint.color = color
            paint.isAntiAlias = true
            canvas.drawPath(path, paint)
        } finally {
            holder.unlockCanvasAndPost(canvas)
        }
    }
    
    fun drawGradientRect(x: Float, y: Float, width: Float, height: Float,
                         colors: IntArray) {
        if (colors.size < 2) return
        val canvas = holder.lockCanvas() ?: return
        try {
            val gradient = LinearGradient(x, y, x, y + height, colors, null, Shader.TileMode.CLAMP)
            paint.shader = gradient
            paint.style = Paint.Style.FILL
            canvas.drawRect(x, y, x + width, y + height, paint)
            paint.shader = null
        } finally {
            holder.unlockCanvasAndPost(canvas)
        }
    }
    
    fun drawGradientRectH(x: Float, y: Float, width: Float, height: Float,
                          colors: IntArray) {
        if (colors.size < 2) return
        val canvas = holder.lockCanvas() ?: return
        try {
            val gradient = LinearGradient(x, y, x + width, y, colors, null, Shader.TileMode.CLAMP)
            paint.shader = gradient
            paint.style = Paint.Style.FILL
            canvas.drawRect(x, y, x + width, y + height, paint)
            paint.shader = null
        } finally {
            holder.unlockCanvasAndPost(canvas)
        }
    }
    
    fun drawGradientCircle(cx: Float, cy: Float, radius: Float, colors: IntArray) {
        if (colors.size < 2) return
        val canvas = holder.lockCanvas() ?: return
        try {
            val gradient = RadialGradient(cx, cy, radius, colors, null, Shader.TileMode.CLAMP)
            paint.shader = gradient
            paint.style = Paint.Style.FILL
            canvas.drawCircle(cx, cy, radius, paint)
            paint.shader = null
        } finally {
            holder.unlockCanvasAndPost(canvas)
        }
    }
    
    fun drawGradientOval(x: Float, y: Float, width: Float, height: Float,
                         colors: IntArray) {
        if (colors.size < 2) return
        val canvas = holder.lockCanvas() ?: return
        try {
            val gradient = LinearGradient(x, y, x, y + height, colors, null, Shader.TileMode.CLAMP)
            paint.shader = gradient
            paint.style = Paint.Style.FILL
            canvas.drawOval(x, y, x + width, y + height, paint)
            paint.shader = null
        } finally {
            holder.unlockCanvasAndPost(canvas)
        }
    }
    
    fun drawOverlay(color: Int = 0x88000000.toInt()) {
        val canvas = holder.lockCanvas() ?: return
        try {
            paint.style = Paint.Style.FILL
            paint.color = color
            canvas.drawRect(0f, 0f, surfaceView.width.toFloat(), surfaceView.height.toFloat(), paint)
        } finally {
            holder.unlockCanvasAndPost(canvas)
        }
    }
    
    fun getWidth(): Int = surfaceView.width
    fun getHeight(): Int = surfaceView.height
}

object LuaCollision {
    fun rectIntersect(x1: Float, y1: Float, w1: Float, h1: Float,
                      x2: Float, y2: Float, w2: Float, h2: Float): Boolean {
        return x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2
    }
    
    fun pointInRect(px: Float, py: Float,
                    x: Float, y: Float, w: Float, h: Float): Boolean {
        return px >= x && px <= x + w && py >= y && py <= y + h
    }
    
    fun circleIntersect(cx1: Float, cy1: Float, r1: Float,
                        cx2: Float, cy2: Float, r2: Float): Boolean {
        val dx = cx2 - cx1
        val dy = cy2 - cy1
        return Math.sqrt((dx * dx + dy * dy).toDouble()) <= r1 + r2
    }
    
    fun pointInCircle(px: Float, py: Float, cx: Float, cy: Float, r: Float): Boolean {
        val dx = px - cx
        val dy = py - cy
        return Math.sqrt((dx * dx + dy * dy).toDouble()) <= r
    }
    
    fun circleRectIntersect(cx: Float, cy: Float, r: Float,
                            rx: Float, ry: Float, rw: Float, rh: Float): Boolean {
        val closestX = cx.coerceIn(rx, rx + rw)
        val closestY = cy.coerceIn(ry, ry + rh)
        val dx = cx - closestX
        val dy = cy - closestY
        return (dx * dx + dy * dy) <= (r * r)
    }
    
    fun lineIntersect(x1: Float, y1: Float, x2: Float, y2: Float,
                      x3: Float, y3: Float, x4: Float, y4: Float): Boolean {
        val denom = (y4 - y3) * (x2 - x1) - (x4 - x3) * (y2 - y1)
        if (Math.abs(denom) < 0.0001f) return false
        val ua = ((x4 - x3) * (y1 - y3) - (y4 - y3) * (x1 - x3)) / denom
        val ub = ((x2 - x1) * (y1 - y3) - (y2 - y1) * (x1 - x3)) / denom
        return ua >= 0 && ua <= 1 && ub >= 0 && ub <= 1
    }
}

object LuaMath {
    fun lerp(start: Float, end: Float, t: Float): Float {
        return start + (end - start) * t.coerceIn(0f, 1f)
    }
    
    fun distance(x1: Float, y1: Float, x2: Float, y2: Float): Float {
        val dx = x2 - x1
        val dy = y2 - y1
        return Math.sqrt((dx * dx + dy * dy).toDouble()).toFloat()
    }
    
    fun distance3D(x1: Float, y1: Float, z1: Float, x2: Float, y2: Float, z2: Float): Float {
        val dx = x2 - x1
        val dy = y2 - y1
        val dz = z2 - z1
        return Math.sqrt((dx * dx + dy * dy + dz * dz).toDouble()).toFloat()
    }
    
    fun clamp(value: Float, min: Float, max: Float): Float {
        return value.coerceIn(min, max)
    }
    
    fun clampInt(value: Int, min: Int, max: Int): Int {
        return value.coerceIn(min, max)
    }
    
    fun randomFloat(min: Float, max: Float): Float {
        return min + (Math.random() * (max - min)).toFloat()
    }
    
    fun randomInt(min: Int, max: Int): Int {
        return (min + Math.random() * (max - min + 1)).toInt()
    }
    
    fun randomBool(): Boolean = Math.random() < 0.5
    
    fun randomSign(): Int = if (Math.random() < 0.5) -1 else 1
    
    fun easeIn(t: Float): Float = t * t
    fun easeOut(t: Float): Float = 1f - (1f - t) * (1f - t)
    fun easeInOut(t: Float): Float = if (t < 0.5f) 2f * t * t else 1f - Math.pow((-2f * t + 2).toDouble(), 2.0).toFloat() / 2f
    fun easeOutBounce(t: Float): Float {
        val n1 = 7.5625f
        val d1 = 2.75f
        return when {
            t < 1f / d1 -> n1 * t * t
            t < 2f / d1 -> {
                val t2 = t - 1.5f / d1
                n1 * t2 * t2 + 0.75f
            }
            t < 2.5f / d1 -> {
                val t2 = t - 2.25f / d1
                n1 * t2 * t2 + 0.9375f
            }
            else -> {
                val t2 = t - 2.625f / d1
                n1 * t2 * t2 + 0.984375f
            }
        }
    }
    
    fun degToRad(degrees: Float): Float = Math.toRadians(degrees.toDouble()).toFloat()
    fun radToDeg(radians: Float): Float = Math.toDegrees(radians.toDouble()).toFloat()
    
    fun sin(angle: Float): Float = Math.sin(Math.toRadians(angle.toDouble())).toFloat()
    fun cos(angle: Float): Float = Math.cos(Math.toRadians(angle.toDouble())).toFloat()
    fun tan(angle: Float): Float = Math.tan(Math.toRadians(angle.toDouble())).toFloat()
    
    fun map(value: Float, inMin: Float, inMax: Float, outMin: Float, outMax: Float): Float {
        return (value - inMin) * (outMax - outMin) / (inMax - inMin) + outMin
    }
    
    fun pingPong(value: Float, max: Float): Float {
        val period = max * 2
        val mod = (value % period + period) % period
        return if (mod > max) period - mod else mod
    }
    
    fun repeat(value: Float, max: Float): Float {
        return (value % max + max) % max
    }
    
    fun sign(value: Float): Int = when {
        value > 0 -> 1
        value < 0 -> -1
        else -> 0
    }
    
    fun fract(value: Float): Float = value - Math.floor(value.toDouble()).toFloat()
}

object LuaColor {
    fun rgb(r: Int, g: Int, b: Int): Int = Color.rgb(r, g, b)
    fun argb(a: Int, r: Int, g: Int, b: Int): Int = Color.argb(a, r, g, b)
    fun setAlpha(color: Int, a: Int): Int = Color.argb(a, Color.red(color), Color.green(color), Color.blue(color))
    fun setRed(color: Int, r: Int): Int = Color.argb(Color.alpha(color), r, Color.green(color), Color.blue(color))
    fun setGreen(color: Int, g: Int): Int = Color.argb(Color.alpha(color), Color.red(color), g, Color.blue(color))
    fun setBlue(color: Int, b: Int): Int = Color.argb(Color.alpha(color), Color.red(color), Color.green(color), b)
    fun getAlpha(color: Int): Int = Color.alpha(color)
    fun getRed(color: Int): Int = Color.red(color)
    fun getGreen(color: Int): Int = Color.green(color)
    fun getBlue(color: Int): Int = Color.blue(color)
    fun random(): Int = Color.rgb((Math.random() * 255).toInt(), (Math.random() * 255).toInt(), (Math.random() * 255).toInt())
    fun randomWithAlpha(): Int = Color.argb((Math.random() * 255).toInt(), (Math.random() * 255).toInt(), (Math.random() * 255).toInt(), (Math.random() * 255).toInt())
    fun randomRange(min: Int, max: Int): Int = min + (Math.random() * (max - min + 1)).toInt()
    fun randomColor(): Int = Color.rgb((Math.random() * 200 + 55).toInt(), (Math.random() * 200 + 55).toInt(), (Math.random() * 200 + 55).toInt())
    fun invert(color: Int): Int = Color.argb(Color.alpha(color), 255 - Color.red(color), 255 - Color.green(color), 255 - Color.blue(color))
    fun grayscale(color: Int): Int {
        val avg = (Color.red(color) + Color.green(color) + Color.blue(color)) / 3
        return Color.argb(Color.alpha(color), avg, avg, avg)
    }
    fun lerp(color1: Int, color2: Int, t: Float): Int {
        val tClamped = t.coerceIn(0f, 1f)
        val r = (Color.red(color1) + (Color.red(color2) - Color.red(color1)) * tClamped).toInt()
        val g = (Color.green(color1) + (Color.green(color2) - Color.green(color1)) * tClamped).toInt()
        val b = (Color.blue(color1) + (Color.blue(color2) - Color.blue(color1)) * tClamped).toInt()
        return Color.rgb(r, g, b)
    }
    fun hex(hexString: String): Int {
        return Color.parseColor(hexString)
    }
    val TRANSPARENT = 0x00000000
    val BLACK = 0xFF000000.toInt()
    val WHITE = 0xFFFFFFFF.toInt()
    val RED = 0xFFFF0000.toInt()
    val GREEN = 0xFF00FF00.toInt()
    val BLUE = 0xFF0000FF.toInt()
    val YELLOW = 0xFFFFFF00.toInt()
    val CYAN = 0xFF00FFFF.toInt()
    val MAGENTA = 0xFFFF00FF.toInt()
    val GRAY = 0xFF888888.toInt()
    val DARK_GRAY = 0xFF444444.toInt()
    val LIGHT_GRAY = 0xFFCCCCCC.toInt()
    val ORANGE = 0xFFFF8800.toInt()
    val PURPLE = 0xFF8800FF.toInt()
    val PINK = 0xFFFF88AA.toInt()
}

package com.difierline.lua.lxclua.utils

object LuaStruct {
    
    fun createRect(x: Float, y: Float, width: Float, height: Float): RectF {
        return RectF(x, y, x + width, y + height)
    }
    
    fun createPoint(x: Float, y: Float): PointF {
        return PointF(x, y)
    }
    
    fun createPoint3D(x: Float, y: Float, z: Float): Point3D {
        return Point3D(x, y, z)
    }
    
    fun createSize(width: Float, height: Float): Size {
        return Size(width, height)
    }
    
    fun createColor(r: Int, g: Int, b: Int): Int {
        return android.graphics.Color.rgb(r, g, b)
    }
    
    fun createColorWithAlpha(a: Int, r: Int, g: Int, b: Int): Int {
        return android.graphics.Color.argb(a, r, g, b)
    }
    
    fun createCircle(cx: Float, cy: Float, radius: Float): Circle {
        return Circle(cx, cy, radius)
    }
    
    fun createLine(x1: Float, y1: Float, x2: Float, y2: Float): Line {
        return Line(x1, y1, x2, y2)
    }
    
    fun createPolygon(points: FloatArray): Polygon {
        return Polygon(points)
    }
    
    fun createBounds(x: Float, y: Float, width: Float, height: Float): Bounds {
        return Bounds(x, y, width, height)
    }
    
    fun createTransform(x: Float = 0f, y: Float = 0f, 
                        rotation: Float = 0f, scaleX: Float = 1f, 
                        scaleY: Float = 1f, skewX: Float = 0f, 
                        skewY: Float = 0f): Transform {
        return Transform(x, y, rotation, scaleX, scaleY, skewX, skewY)
    }
    
    fun createAnimation(from: Float, to: Float, duration: Long, 
                        easing: String = "linear"): Animation {
        return Animation(from, to, duration, easing)
    }
    
    fun createTween(from: Float, to: Float, duration: Long): Tween {
        return Tween(from, to, duration)
    }
    
    fun createVector2D(x: Float, y: Float): Vector2D {
        return Vector2D(x, y)
    }
    
    fun createVector3D(x: Float, y: Float, z: Float): Vector3D {
        return Vector3D(x, y, z)
    }
    
    fun createRange(min: Float, max: Float): Range {
        return Range(min, max)
    }
    
    fun createRGB(r: Int, g: Int, b: Int): RGB {
        return RGB(r, g, b)
    }
    
    fun createHSL(h: Float, s: Float, l: Float): HSL {
        return HSL(h, s, l)
    }
    
    fun createHSV(h: Float, s: Float, v: Float): HSV {
        return HSV(h, s, v)
    }
    
    fun createVertex(x: Float, y: Float, z: Float = 0f, 
                     u: Float = 0f, v: Float = 0f, 
                     color: Int = 0xFFFFFFFF.toInt()): Vertex {
        return Vertex(x, y, z, u, v, color)
    }
    
    fun createMesh(width: Float, height: Float, segmentsX: Int = 1, 
                   segmentsY: Int = 1): Mesh {
        return Mesh(width, height, segmentsX, segmentsY)
    }
    
    fun createAABB(minX: Float, minY: Float, minZ: Float,
                   maxX: Float, maxY: Float, maxZ: Float): AABB {
        return AABB(minX, minY, minZ, maxX, maxY, maxZ)
    }
    
    fun createOBB(centerX: Float, centerY: Float, width: Float, 
                  height: Float, angle: Float): OBB {
        return OBB(centerX, centerY, width, height, angle)
    }
    
    fun createRay(originX: Float, originY: Float, originZ: Float,
                  dirX: Float, dirY: Float, dirZ: Float): Ray {
        return Ray(originX, originY, originZ, dirX, dirY, dirZ)
    }
    
    fun createPlane(normalX: Float, normalY: Float, normalZ: Float,
                    distance: Float): Plane {
        return Plane(normalX, normalY, normalZ, distance)
    }
    
    fun createSphere(centerX: Float, centerY: Float, centerZ: Float,
                     radius: Float): Sphere {
        return Sphere(centerX, centerY, centerZ, radius)
    }
    
    fun createCapsule(p1x: Float, p1y: Float, p1z: Float,
                      p2x: Float, p2y: Float, p2z: Float,
                      radius: Float): Capsule {
        return Capsule(p1x, p1y, p1z, p2x, p2y, p2z, radius)
    }
}

data class RectF(var left: Float, var top: Float, var right: Float, var bottom: Float) {
    val width: Float get() = right - left
    val height: Float get() = bottom - top
    val centerX: Float get() = (left + right) / 2
    val centerY: Float get() = (top + bottom) / 2
    
    fun set(x: Float, y: Float, w: Float, h: Float) {
        left = x
        top = y
        right = x + w
        bottom = y + h
    }
    
    fun contains(x: Float, y: Float): Boolean {
        return x >= left && x <= right && y >= top && y <= bottom
    }
    
    fun intersects(other: RectF): Boolean {
        return left < other.right && right > other.left && 
               top < other.bottom && bottom > other.top
    }
    
    fun intersect(other: RectF): Boolean {
        val newLeft = maxOf(left, other.left)
        val newTop = maxOf(top, other.top)
        val newRight = minOf(right, other.right)
        val newBottom = minOf(bottom, other.bottom)
        if (newLeft < newRight && newTop < newBottom) {
            left = newLeft
            top = newTop
            right = newRight
            bottom = newBottom
            return true
        }
        return false
    }
    
    fun union(other: RectF) {
        left = minOf(left, other.left)
        top = minOf(top, other.top)
        right = maxOf(right, other.right)
        bottom = maxOf(bottom, other.bottom)
    }
    
    fun expand(dx: Float, dy: Float) {
        left -= dx
        top -= dy
        right += dx
        bottom += dy
    }
}

data class PointF(var x: Float, var y: Float) {
    fun set(x: Float, y: Float) {
        this.x = x
        this.y = y
    }
    
    fun add(other: PointF): PointF {
        return PointF(x + other.x, y + other.y)
    }
    
    fun subtract(other: PointF): PointF {
        return PointF(x - other.x, y - other.y)
    }
    
    fun multiply(scalar: Float): PointF {
        return PointF(x * scalar, y * scalar)
    }
    
    fun length(): Float {
        return kotlin.math.sqrt(x * x + y * y)
    }
    
    fun normalize(): PointF {
        val len = length()
        return if (len > 0) PointF(x / len, y / len) else PointF(0f, 0f)
    }
    
    fun distanceTo(other: PointF): Float {
        val dx = x - other.x
        val dy = y - other.y
        return kotlin.math.sqrt(dx * dx + dy * dy)
    }
    
    fun angle(): Float {
        return Math.toDegrees(kotlin.math.atan2(y.toDouble(), x.toDouble())).toFloat()
    }
    
    fun rotate(angle: Float): PointF {
        val rad = Math.toRadians(angle.toDouble())
        val cos = kotlin.math.cos(rad).toFloat()
        val sin = kotlin.math.sin(rad).toFloat()
        return PointF(x * cos - y * sin, x * sin + y * cos)
    }
}

data class Point3D(var x: Float, var y: Float, var z: Float) {
    fun set(x: Float, y: Float, z: Float) {
        this.x = x
        this.y = y
        this.z = z
    }
    
    fun length(): Float {
        return kotlin.math.sqrt(x * x + y * y + z * z)
    }
    
    fun distanceTo(other: Point3D): Float {
        val dx = x - other.x
        val dy = y - other.y
        val dz = z - other.z
        return kotlin.math.sqrt(dx * dx + dy * dy + dz * dz)
    }
    
    fun normalize(): Point3D {
        val len = length()
        return if (len > 0) Point3D(x / len, y / len, z / len) else Point3D(0f, 0f, 0f)
    }
    
    fun dot(other: Point3D): Float {
        return x * other.x + y * other.y + z * other.z
    }
    
    fun cross(other: Point3D): Point3D {
        return Point3D(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        )
    }
}

data class Size(var width: Float, var height: Float) {
    fun set(w: Float, h: Float) {
        width = w
        height = h
    }
    
    fun area(): Float = width * height
    
    fun aspectRatio(): Float = if (height != 0f) width / height else 0f
    
    fun contains(other: Size): Boolean {
        return width >= other.width && height >= other.height
    }
    
    fun scale(factor: Float): Size {
        return Size(width * factor, height * factor)
    }
}

data class Circle(var cx: Float, var cy: Float, var radius: Float) {
    fun set(cx: Float, cy: Float, r: Float) {
        this.cx = cx
        this.cy = cy
        this.radius = r
    }
    
    fun area(): Float {
        return Math.PI.toFloat() * radius * radius
    }
    
    fun circumference(): Float {
        return 2 * Math.PI.toFloat() * radius
    }
    
    fun contains(x: Float, y: Float): Boolean {
        val dx = x - cx
        val dy = y - cy
        return dx * dx + dy * dy <= radius * radius
    }
    
    fun intersects(other: Circle): Boolean {
        val dx = other.cx - cx
        val dy = other.cy - cy
        val dist = kotlin.math.sqrt(dx * dx + dy * dy)
        return dist <= radius + other.radius
    }
}

data class Line(var x1: Float, var y1: Float, var x2: Float, var y2: Float) {
    fun length(): Float {
        val dx = x2 - x1
        val dy = y2 - y1
        return kotlin.math.sqrt(dx * dx + dy * dy)
    }
    
    fun angle(): Float {
        return Math.toDegrees(kotlin.math.atan2((y2 - y1).toDouble(), (x2 - x1).toDouble())).toFloat()
    }
    
    fun midpoint(): PointF {
        return PointF((x1 + x2) / 2, (y1 + y2) / 2)
    }
    
    fun contains(x: Float, y: Float): Boolean {
        val cross = (y2 - y1) * (x - x1) - (x2 - x1) * (y - y1)
        if (kotlin.math.abs(cross) > 0.01f) return false
        val dot = (x - x1) * (x - x2) + (y - y1) * (y - y2)
        return dot <= 0
    }
    
    fun closestPoint(px: Float, py: Float): PointF {
        val dx = x2 - x1
        val dy = y2 - y1
        val len2 = dx * dx + dy * dy
        if (len2 == 0f) return PointF(x1, y1)
        var t = ((px - x1) * dx + (py - y1) * dy) / len2
        t = t.coerceIn(0f, 1f)
        return PointF(x1 + t * dx, y1 + t * dy)
    }
    
    fun intersects(other: Line): Boolean {
        val denom = (other.y2 - other.y1) * (x2 - x1) - (other.x2 - other.x1) * (y2 - y1)
        if (kotlin.math.abs(denom) < 0.0001f) return false
        val ua = ((other.x2 - other.x1) * (y1 - other.y1) - (other.y2 - other.y1) * (x1 - other.x1)) / denom
        val ub = ((x2 - x1) * (y1 - other.y1) - (y2 - y1) * (x1 - other.x1)) / denom
        return ua >= 0 && ua <= 1 && ub >= 0 && ub <= 1
    }
}

data class Polygon(var points: FloatArray) {
    val vertexCount: Int get() = points.size / 2
    
    fun area(): Float {
        var area = 0f
        val n = points.size
        for (i in 0 until n step 2) {
            val j = (i + 2) % n
            area += points[i] * points[j + 1]
            area -= points[j] * points[i + 1]
        }
        return area / 2
    }
    
    fun centroid(): PointF {
        var cx = 0f
        var cy = 0f
        val n = vertexCount
        for (i in 0 until n) {
            val x0 = points[i * 2]
            val y0 = points[i * 2 + 1]
            val x1 = points[(i + 1) % n * 2]
            val y1 = points[(i + 1) % n * 2 + 1]
            val cross = x0 * y1 - x1 * y0
            cx += (x0 + x1) * cross
            cy += (y0 + y1) * cross
        }
        val area = area()
        if (kotlin.math.abs(area) > 0.0001f) {
            cx /= (6 * area)
            cy /= (6 * area)
        }
        return PointF(cx, cy)
    }
    
    fun contains(x: Float, y: Float): Boolean {
        val n = vertexCount
        var inside = false
        for (i in 0 until n) {
            val j = (i + 1) % n
            val xi = points[i * 2]
            val yi = points[i * 2 + 1]
            val xj = points[j * 2]
            val yj = points[j * 2 + 1]
            if (((yi > y) != (yj > y)) && (x < (xj - xi) * (y - yi) / (yj - yi) + xi)) {
                inside = !inside
            }
        }
        return inside
    }
    
    fun boundingBox(): RectF {
        var minX = Float.MAX_VALUE
        var minY = Float.MAX_VALUE
        var maxX = Float.MIN_VALUE
        var maxY = Float.MIN_VALUE
        for (i in 0 until points.size step 2) {
            minX = minOf(minX, points[i])
            minY = minOf(minY, points[i + 1])
            maxX = maxOf(maxX, points[i])
            maxY = maxOf(maxY, points[i + 1])
        }
        return RectF(minX, minY, maxX, maxY)
    }
}

data class Bounds(var x: Float, var y: Float, var width: Float, var height: Float) {
    val left: Float get() = x
    val top: Float get() = y
    val right: Float get() = x + width
    val bottom: Float get() = y + height
    val centerX: Float get() = x + width / 2
    val centerY: Float get() = y + height / 2
    
    fun contains(x: Float, y: Float): Boolean {
        return x >= left && x <= right && y >= top && y <= bottom
    }
    
    fun intersects(other: Bounds): Boolean {
        return left < other.right && right > other.left && 
               top < other.bottom && bottom > other.top
    }
    
    fun expand(dx: Float, dy: Float) {
        x -= dx
        y -= dy
        width += dx * 2
        height += dy * 2
    }
    
    fun contract(dx: Float, dy: Float) {
        x += dx
        y += dy
        width = (width - dx * 2).coerceAtLeast(0f)
        height = (height - dy * 2).coerceAtLeast(0f)
    }
}

data class Transform(
    var x: Float = 0f,
    var y: Float = 0f,
    var rotation: Float = 0f,
    var scaleX: Float = 1f,
    var scaleY: Float = 1f,
    var skewX: Float = 0f,
    var skewY: Float = 0f
) {
    fun reset() {
        x = 0f
        y = 0f
        rotation = 0f
        scaleX = 1f
        scaleY = 1f
        skewX = 0f
        skewY = 0f
    }
    
    fun translate(dx: Float, dy: Float) {
        x += dx
        y += dy
    }
    
    fun rotate(angle: Float) {
        rotation += angle
    }
    
    fun scale(sx: Float, sy: Float) {
        scaleX *= sx
        scaleY *= sy
    }
    
    fun multiply(other: Transform): Transform {
        val result = Transform()
        result.x = x + other.x
        result.y = y + other.y
        result.rotation = rotation + other.rotation
        result.scaleX = scaleX * other.scaleX
        result.scaleY = scaleY * other.scaleY
        result.skewX = skewX + other.skewX
        result.skewY = skewY + other.skewY
        return result
    }
    
    fun applyToPoint(px: Float, py: Float): PointF {
        val rad = Math.toRadians(rotation.toDouble())
        val cos = kotlin.math.cos(rad).toFloat()
        val sin = kotlin.math.sin(rad).toFloat()
        val nx = (px * cos - py * sin) * scaleX + x
        val ny = (px * sin + py * cos) * scaleY + y
        return PointF(nx, ny)
    }
}

data class Animation(var from: Float, var to: Float, var duration: Long, var easing: String = "linear") {
    var startTime: Long = 0
    var isRunning: Boolean = false
    var isPaused: Boolean = false
    var pauseTime: Long = 0
    
    fun start() {
        startTime = System.currentTimeMillis()
        isRunning = true
        isPaused = false
    }
    
    fun stop() {
        isRunning = false
        isPaused = false
    }
    
    fun pause() {
        if (isRunning && !isPaused) {
            pauseTime = System.currentTimeMillis()
            isPaused = true
        }
    }
    
    fun resume() {
        if (isRunning && isPaused) {
            startTime += System.currentTimeMillis() - pauseTime
            isPaused = false
        }
    }
    
    fun getProgress(): Float {
        if (!isRunning || duration <= 0) return 0f
        val elapsed = if (isPaused) pauseTime - startTime else System.currentTimeMillis() - startTime
        return (elapsed.toFloat() / duration).coerceIn(0f, 1f)
    }
    
    fun getValue(): Float {
        val t = getProgress()
        return from + (to - from) * ease(t)
    }
    
    fun isFinished(): Boolean {
        return isRunning && getProgress() >= 1f
    }
    
    private fun ease(t: Float): Float {
        return when (easing) {
            "easeIn" -> t * t
            "easeOut" -> 1f - (1f - t) * (1f - t)
            "easeInOut" -> if (t < 0.5f) 2f * t * t else 1f - Math.pow((-2f * t + 2).toDouble(), 2.0).toFloat() / 2f
            "easeOutBounce" -> {
                val n1 = 7.5625f
                val d1 = 2.75f
                when {
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
            else -> t
        }
    }
}

data class Tween(var from: Float, var to: Float, var duration: Long) {
    var startValue: Float = from
    var endValue: Float = to
    var elapsed: Long = 0
    
    fun update(deltaTime: Long): Float {
        elapsed = (elapsed + deltaTime).coerceAtMost(duration)
        val t = elapsed.toFloat() / duration
        return startValue + (endValue - startValue) * t
    }
    
    fun reset() {
        elapsed = 0
    }
    
    fun isComplete(): Boolean = elapsed >= duration
}

data class Vector2D(var x: Float, var y: Float) {
    fun length(): Float = kotlin.math.sqrt(x * x + y * y)
    
    fun normalize(): Vector2D {
        val len = length()
        return if (len > 0) Vector2D(x / len, y / len) else Vector2D(0f, 0f)
    }
    
    fun dot(other: Vector2D): Float = x * other.x + y * other.y
    
    fun cross(other: Vector2D): Float = x * other.y - y * other.x
    
    fun add(other: Vector2D): Vector2D = Vector2D(x + other.x, y + other.y)
    
    fun subtract(other: Vector2D): Vector2D = Vector2D(x - other.x, y - other.y)
    
    fun multiply(scalar: Float): Vector2D = Vector2D(x * scalar, y * scalar)
    
    fun perpendicular(): Vector2D = Vector2D(-y, x)
    
    fun angle(): Float = Math.toDegrees(kotlin.math.atan2(y.toDouble(), x.toDouble())).toFloat()
    
    fun rotate(angle: Float): Vector2D {
        val rad = Math.toRadians(angle.toDouble())
        val cos = kotlin.math.cos(rad).toFloat()
        val sin = kotlin.math.sin(rad).toFloat()
        return Vector2D(x * cos - y * sin, x * sin + y * cos)
    }
}

data class Vector3D(var x: Float, var y: Float, var z: Float) {
    fun length(): Float = kotlin.math.sqrt(x * x + y * y + z * z)
    
    fun normalize(): Vector3D {
        val len = length()
        return if (len > 0) Vector3D(x / len, y / len, z / len) else Vector3D(0f, 0f, 0f)
    }
    
    fun dot(other: Vector3D): Float = x * other.x + y * other.y + z * other.z
    
    fun cross(other: Vector3D): Vector3D {
        return Vector3D(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        )
    }
    
    fun add(other: Vector3D): Vector3D = Vector3D(x + other.x, y + other.y, z + other.z)
    
    fun subtract(other: Vector3D): Vector3D = Vector3D(x - other.x, y - other.y, z - other.z)
    
    fun multiply(scalar: Float): Vector3D = Vector3D(x * scalar, y * scalar, z * scalar)
}

data class Range(var min: Float, var max: Float) {
    fun contains(value: Float): Boolean = value >= min && value <= max
    
    fun clamp(value: Float): Float = value.coerceIn(min, max)
    
    fun length(): Float = max - min
    
    fun lerp(t: Float): Float = min + (max - min) * t.coerceIn(0f, 1f)
    
    fun expand(amount: Float): Range = Range(min - amount, max + amount)
}

data class RGB(var r: Int, var g: Int, var b: Int) {
    fun toInt(): Int = android.graphics.Color.rgb(r.coerceIn(0, 255), g.coerceIn(0, 255), b.coerceIn(0, 255))
    
    fun toHSL(): HSL {
        val rNorm = r / 255f
        val gNorm = g / 255f
        val bNorm = b / 255f
        val max = maxOf(rNorm, gNorm, bNorm)
        val min = minOf(rNorm, gNorm, bNorm)
        var h = 0f
        val l = (max + min) / 2
        
        if (max != min) {
            val d = max - min
            when (max) {
                rNorm -> h = ((gNorm - bNorm) / d + (if (gNorm < bNorm) 6 else 0)) / 6
                gNorm -> h = ((bNorm - rNorm) / d + 2) / 6
                else -> h = ((rNorm - gNorm) / d + 4) / 6
            }
        }
        
        val s = if (l == 0f || l == 1f) 0f else (max - l) / minOf(l, 1 - l)
        return HSL(h * 360f, s, l)
    }
}

data class HSL(var h: Float, var s: Float, var l: Float) {
    fun toRGB(): RGB {
        val c = (1f - kotlin.math.abs(2f * l - 1f)) * s
        val x = c * (1f - kotlin.math.abs((h / 60f) % 2f - 1f))
        val m = l - c / 2f
        
        val r: Float
        val g: Float
        val b: Float
        when {
            h < 60 -> {
                r = c; g = x; b = 0f
            }
            h < 120 -> {
                r = x; g = c; b = 0f
            }
            h < 180 -> {
                r = 0f; g = c; b = x
            }
            h < 240 -> {
                r = 0f; g = x; b = c
            }
            h < 300 -> {
                r = x; g = 0f; b = c
            }
            else -> {
                r = c; g = 0f; b = x
            }
        }
        
        return RGB(((r + m) * 255).toInt(), ((g + m) * 255).toInt(), ((b + m) * 255).toInt())
    }
    
    fun toInt(): Int = toRGB().toInt()
}

data class HSV(var h: Float, var s: Float, var v: Float) {
    fun toRGB(): RGB {
        val c = v * s
        val x = c * (1f - kotlin.math.abs((h / 60f) % 2f - 1f))
        val m = v - c
        
        val r: Float
        val g: Float
        val b: Float
        when {
            h < 60 -> {
                r = c; g = x; b = 0f
            }
            h < 120 -> {
                r = x; g = c; b = 0f
            }
            h < 180 -> {
                r = 0f; g = c; b = x
            }
            h < 240 -> {
                r = 0f; g = x; b = c
            }
            h < 300 -> {
                r = x; g = 0f; b = c
            }
            else -> {
                r = c; g = 0f; b = x
            }
        }
        
        return RGB(((r + m) * 255).toInt(), ((g + m) * 255).toInt(), ((b + m) * 255).toInt())
    }
    
    fun toInt(): Int = toRGB().toInt()
}

data class Vertex(
    var x: Float, var y: Float, var z: Float = 0f,
    var u: Float = 0f, var v: Float = 0f,
    var color: Int = 0xFFFFFFFF.toInt()
)

data class Mesh(var width: Float, var height: Float, var segmentsX: Int, var segmentsY: Int) {
    val vertexCount: Int get() = (segmentsX + 1) * (segmentsY + 1)
    val indexCount: Int get() = segmentsX * segmentsY * 6
    
    fun createVertices(): FloatArray {
        val verts = FloatArray(vertexCount * 5)
        var i = 0
        for (y in 0..segmentsY) {
            for (x in 0..segmentsX) {
                verts[i++] = x * width / segmentsX
                verts[i++] = y * height / segmentsY
                verts[i++] = 0f
                verts[i++] = x.toFloat() / segmentsX
                verts[i++] = y.toFloat() / segmentsY
            }
        }
        return verts
    }
    
    fun createIndices(): ShortArray {
        val indices = ShortArray(indexCount)
        var i = 0
        for (y in 0 until segmentsY) {
            for (x in 0 until segmentsX) {
                val topLeft = (y * (segmentsX + 1) + x).toShort()
                val topRight = (topLeft + 1).toShort()
                val bottomLeft = ((y + 1) * (segmentsX + 1) + x).toShort()
                val bottomRight = (bottomLeft + 1).toShort()
                
                indices[i++] = topLeft
                indices[i++] = bottomLeft
                indices[i++] = topRight
                indices[i++] = topRight
                indices[i++] = bottomLeft
                indices[i++] = bottomRight
            }
        }
        return indices
    }
}

data class AABB(
    var minX: Float, var minY: Float, var minZ: Float,
    var maxX: Float, var maxY: Float, var maxZ: Float
) {
    val width: Float get() = maxX - minX
    val height: Float get() = maxY - minY
    val depth: Float get() = maxZ - minZ
    val centerX: Float get() = (minX + maxX) / 2
    val centerY: Float get() = (minY + maxY) / 2
    val centerZ: Float get() = (minZ + maxZ) / 2
    
    fun contains(x: Float, y: Float, z: Float): Boolean {
        return x >= minX && x <= maxX && y >= minY && y <= maxY && z >= minZ && z <= maxZ
    }
    
    fun intersects(other: AABB): Boolean {
        return minX < other.maxX && maxX > other.minX &&
               minY < other.maxY && maxY > other.minY &&
               minZ < other.maxZ && maxZ > other.minZ
    }
}

data class OBB(
    var centerX: Float, var centerY: Float,
    var width: Float, var height: Float,
    var angle: Float
) {
    val halfWidth: Float get() = width / 2
    val halfHeight: Float get() = height / 2
    
    fun getCorners(): FloatArray {
        val rad = Math.toRadians(angle.toDouble())
        val cos = kotlin.math.cos(rad).toFloat()
        val sin = kotlin.math.sin(rad).toFloat()
        val hw = halfWidth
        val hh = halfHeight
        
        return floatArrayOf(
            centerX + (-hw * cos - -hh * sin), centerY + (-hw * sin + -hh * cos),
            centerX + (hw * cos - -hh * sin), centerY + (hw * sin + -hh * cos),
            centerX + (hw * cos - hh * sin), centerY + (hw * sin + hh * cos),
            centerX + (-hw * cos - hh * sin), centerY + (-hw * sin + hh * cos)
        )
    }
}

data class Ray(
    var originX: Float, var originY: Float, var originZ: Float,
    var dirX: Float, var dirY: Float, var dirZ: Float
) {
    init {
        val len = kotlin.math.sqrt(dirX * dirX + dirY * dirY + dirZ * dirZ)
        if (len > 0) {
            dirX /= len
            dirY /= len
            dirZ /= len
        }
    }
    
    fun getPoint(t: Float): Point3D {
        return Point3D(originX + dirX * t, originY + dirY * t, originZ + dirZ * t)
    }
}

data class Plane(
    var normalX: Float, var normalY: Float, var normalZ: Float,
    var distance: Float
) {
    init {
        val len = kotlin.math.sqrt(normalX * normalX + normalY * normalY + normalZ * normalZ)
        if (len > 0) {
            normalX /= len
            normalY /= len
            normalZ /= len
        }
    }
    
    fun distanceToPoint(x: Float, y: Float, z: Float): Float {
        return normalX * x + normalY * y + normalZ * z - distance
    }
    
    fun isPointAbove(x: Float, y: Float, z: Float): Boolean {
        return distanceToPoint(x, y, z) > 0
    }
}

data class Sphere(
    var centerX: Float, var centerY: Float, var centerZ: Float,
    var radius: Float
) {
    fun contains(x: Float, y: Float, z: Float): Boolean {
        val dx = x - centerX
        val dy = y - centerY
        val dz = z - centerZ
        return dx * dx + dy * dy + dz * dz <= radius * radius
    }
    
    fun intersects(other: Sphere): Boolean {
        val dx = other.centerX - centerX
        val dy = other.centerY - centerY
        val dz = other.centerZ - centerZ
        val dist = kotlin.math.sqrt(dx * dx + dy * dy + dz * dz)
        return dist <= radius + other.radius
    }
}

data class Capsule(
    var p1x: Float, var p1y: Float, var p1z: Float,
    var p2x: Float, var p2y: Float, var p2z: Float,
    var radius: Float
) {
    fun length(): Float {
        val dx = p2x - p1x
        val dy = p2y - p1y
        val dz = p2z - p1z
        return kotlin.math.sqrt(dx * dx + dy * dy + dz * dz)
    }
    
    fun contains(x: Float, y: Float, z: Float): Boolean {
        val dx = p2x - p1x
        val dy = p2y - p1y
        val dz = p2z - p1z
        val len2 = dx * dx + dy * dy + dz * dz
        if (len2 == 0f) {
            val d = kotlin.math.sqrt((x - p1x) * (x - p1x) + (y - p1y) * (y - p1y) + (z - p1z) * (z - p1z))
            return d <= radius
        }
        var t = ((x - p1x) * dx + (y - p1y) * dy + (z - p1z) * dz) / len2
        t = t.coerceIn(0f, 1f)
        val closestX = p1x + t * dx
        val closestY = p1y + t * dy
        val closestZ = p1z + t * dz
        val dist = kotlin.math.sqrt(
            (x - closestX) * (x - closestX) + 
            (y - closestY) * (y - closestY) + 
            (z - closestZ) * (z - closestZ)
        )
        return dist <= radius
    }
}

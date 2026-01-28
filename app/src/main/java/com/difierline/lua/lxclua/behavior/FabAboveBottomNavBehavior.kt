package com.difierline.lua.lxclua.behavior

import android.content.Context
import android.util.AttributeSet
import android.view.View
import androidx.coordinatorlayout.widget.CoordinatorLayout
import com.google.android.material.bottomnavigation.BottomNavigationView
import com.google.android.material.floatingactionbutton.FloatingActionButton
import kotlin.math.max

class FabAboveBottomNavBehavior(
    context: Context,
    attrs: AttributeSet?
) : CoordinatorLayout.Behavior<FloatingActionButton>(context, attrs) {

    private val fabMargin = 16f * context.resources.displayMetrics.density
    private val marginEnd = 16f * context.resources.displayMetrics.density

    override fun layoutDependsOn(
        parent: CoordinatorLayout,
        child: FloatingActionButton,
        dependency: View
    ): Boolean = dependency is BottomNavigationView

    override fun onDependentViewChanged(
        parent: CoordinatorLayout,
        child: FloatingActionButton,
        dependency: View
    ): Boolean {
        // 让 FAB 始终位于 BottomNavigationView 上方 fabMargin 的距离
        val targetY = dependency.y - child.height - fabMargin
        val translationY = max(targetY, 0f)

        // X 方向固定在右侧 marginEnd
        val targetX = parent.width - child.width - marginEnd
        val translationX = targetX

        var changed = false
        if (child.translationY != translationY) {
            child.translationY = translationY
            changed = true
        }
        if (child.translationX != translationX) {
            child.translationX = translationX
            changed = true
        }
        return changed
    }

    override fun onDependentViewRemoved(
        parent: CoordinatorLayout,
        child: FloatingActionButton,
        dependency: View
    ) {
        child.translationY = 0f
        child.translationX = 0f
    }
}

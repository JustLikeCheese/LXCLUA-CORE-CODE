package com.difierline.lua.material.floatingactionbutton

import android.animation.Animator
import android.animation.AnimatorSet
import android.animation.ObjectAnimator
import android.view.View
import com.google.android.material.card.MaterialCardView

class FloatingActionButtonAnimator(private val card: MaterialCardView) {

    fun init() {
        card.setVisibility(View.VISIBLE)
    }

    fun show() {
        if (card.visibility == View.VISIBLE) {
            return
        }
        val scaleX = ObjectAnimator.ofFloat(card, "scaleX", 0f, 1f)
        val scaleY = ObjectAnimator.ofFloat(card, "scaleY", 0f, 1f)
        val alpha = ObjectAnimator.ofFloat(card, "alpha", 0f, 1f)
        val animatorSet = AnimatorSet()
        animatorSet.play(scaleX).with(scaleY).with(alpha)
        animatorSet.setDuration(300)
        animatorSet.start()
        
        card.postDelayed({
            if (card.visibility != View.VISIBLE) {
                card.setVisibility(View.VISIBLE)
            }
        }, 150)
    }

    fun hide() {
        if (card.visibility == View.GONE) {
            return
        }
        val scaleX = ObjectAnimator.ofFloat(card, "scaleX", 1f, 0f)
        val scaleY = ObjectAnimator.ofFloat(card, "scaleY", 1f, 0f)
        val alpha = ObjectAnimator.ofFloat(card, "alpha", 1f, 0f)
        val animatorSet = AnimatorSet()
        animatorSet.play(scaleX).with(scaleY).with(alpha)
        animatorSet.setDuration(300)
        animatorSet.start()

        card.postDelayed({
            if (card.visibility != View.GONE) {
                card.setVisibility(View.GONE)
            }
        }, 100)
    }
}
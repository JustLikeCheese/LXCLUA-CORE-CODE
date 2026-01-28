package github.znzsofficial.adapter

import android.view.View
import androidx.recyclerview.widget.RecyclerView

open class LuaCustRecyclerHolder(itemView: View?) : RecyclerView.ViewHolder(itemView!!) {
    var views: Any? = null

    fun setTag(tag:Any){
        views = tag
    }
    fun getTag() = views

    // 添加释放资源的方法
    open fun release() {
        views = null
        // 释放itemView中的资源
        if (itemView is Releasable) {
            (itemView as Releasable).release()
        }
    }
}

// 可释放资源的接口
interface Releasable {
    fun release()
}
package github.znzsofficial.adapter

import android.view.View
import android.view.ViewGroup
import androidx.recyclerview.widget.RecyclerView
import com.difierline.lua.LuaContext

open class LuaCustRecyclerAdapter(open val adapterCreator: Creator, var mContext: LuaContext? = null) : RecyclerView.Adapter<LuaCustRecyclerHolder>() {

    constructor(creator: Creator) : this(creator, null) {
    }

    constructor(context: LuaContext, creator: Creator) : this(creator, context) {
    }

    override fun getItemCount(): Int {
        return try {
            adapterCreator.itemCount.toInt()
        } catch (e: Exception) {
            e.printStackTrace()
            mContext?.sendError("RecyclerAdapter: getItemCount", e)
            0
        }
    }

    override fun getItemViewType(i: Int): Int {
        return try {
            adapterCreator.getItemViewType(i).toInt()
        } catch (e: Exception) {
            e.printStackTrace()
            mContext?.sendError("RecyclerAdapter: getItemViewType", e)
            -1
        }
    }

    override fun onBindViewHolder(holder: LuaCustRecyclerHolder, i: Int) {
        try {
            adapterCreator.onBindViewHolder(holder, i)
        } catch (e: Exception) {
            e.printStackTrace()
            mContext?.sendError("RecyclerAdapter: onBindViewHolder", e)
        }
    }

    override fun onCreateViewHolder(viewGroup: ViewGroup, i: Int): LuaCustRecyclerHolder {
        return try {
            adapterCreator.onCreateViewHolder(viewGroup, i)
        } catch (e: Exception) {
            e.printStackTrace()
            mContext?.sendError("RecyclerAdapter: onCreateViewHolder", e)
            LuaCustRecyclerHolder(null)
        }
    }

    override fun onViewRecycled(holder: LuaCustRecyclerHolder) {
        try {
            adapterCreator.onViewRecycled(holder)
        } catch (e: Exception) {
            e.printStackTrace()
            mContext?.sendError("RecyclerAdapter: onViewRecycled", e)
        }
    }

    // 添加释放资源的方法
    open fun release() {
        mContext = null
        // 如果adapterCreator有释放方法，调用它
        if (adapterCreator is Releasable) {
            (adapterCreator as Releasable).release()
        }
    }

    interface Creator {
        val itemCount: Long
        fun getItemViewType(i: Int): Long
        fun onBindViewHolder(viewHolder: LuaCustRecyclerHolder, i: Int)
        fun onCreateViewHolder(viewGroup: ViewGroup?, i: Int): LuaCustRecyclerHolder
        fun onViewRecycled(viewHolder: LuaCustRecyclerHolder)
        abstract fun <View> getPopupText(view: View, position: Int): Any
    }
    
    // 可释放资源的接口
    interface Releasable {
        fun release()
    }
}
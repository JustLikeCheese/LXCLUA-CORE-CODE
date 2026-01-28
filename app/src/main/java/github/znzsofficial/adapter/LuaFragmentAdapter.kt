package github.znzsofficial.adapter

import androidx.fragment.app.Fragment
import androidx.viewpager2.adapter.FragmentStateAdapter
import com.difierline.lua.LuaActivity

class LuaFragmentAdapter(context: LuaActivity, inter: Creator) :
    FragmentStateAdapter(context.supportFragmentManager, context.lifecycle) {
    var creator: Creator
    private var mContext: LuaActivity?

    init {
        mContext = context
        creator = inter
    }

    override fun createFragment(position: Int): Fragment {
        return try {
            // 根据位置返回对应的 Fragment
            creator.createFragment(position)
        } catch (e: Exception) {
            e.printStackTrace()
            mContext?.sendError("FragmentAdapter", e)
            Fragment()
        }
    }

    override fun getItemCount(): Int {
        return try {
            // 返回 Fragment 的数量
            creator.itemCount.toInt()
        } catch (e: Exception) {
            e.printStackTrace()
            mContext?.sendError("FragmentAdapter", e)
            0
        }
    }

    // 添加释放资源的方法
    fun release() {
        mContext = null
        // 如果creator有释放方法，调用它
        if (creator is Releasable) {
            (creator as Releasable).release()
        }
    }

    interface Creator {
        fun createFragment(i: Int): Fragment
        val itemCount: Long
    }
}
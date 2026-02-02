package github.znzsofficial.adapter

import android.view.View
import com.difierline.lua.LuaContext
import me.zhanghai.android.fastscroll.PopupTextProvider

class PopupRecyclerAdapter(override val adapterCreator: PopupCreator, mContext: LuaContext? = null) : LuaCustRecyclerAdapter(adapterCreator, mContext), PopupTextProvider {

    constructor(creator: PopupCreator) : this(creator, null) {
    }

    constructor(context: LuaContext, creator: PopupCreator) : this(creator, context) {
    }

    override fun getPopupText(view: View, position: Int): CharSequence {
        return this.adapterCreator.getPopupText(view, position).toString()
    }

    // 重写释放方法
    override fun release() {
        super.release()
        // 额外清理
    }

    interface PopupCreator : Creator {
        fun getPopupText(view:View ,i: Int): CharSequence
    }
}
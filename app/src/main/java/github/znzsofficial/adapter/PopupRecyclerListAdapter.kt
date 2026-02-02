package github.znzsofficial.adapter

import android.view.View
import com.difierline.lua.LuaContext
import me.zhanghai.android.fastscroll.PopupTextProvider

class PopupRecyclerListAdapter : RecyclerListAdapter, PopupTextProvider {
    override var adapterCreator: Creator

    constructor(adapterCreator: PopupCreator) : super(adapterCreator) {
        this.adapterCreator = adapterCreator
        mContext = null
    }

    constructor(context: LuaContext, adapterCreator: PopupCreator) : super(
        context,
        adapterCreator
    ) {
        this.adapterCreator = adapterCreator
        mContext = context
    }

    override fun getPopupText(view: View, position: Int): CharSequence {
        return this.adapterCreator.getPopupText(view, position).toString()
    }

    // 添加释放资源的方法
    override fun release() {
        super.release()
        // 额外清理
    }

    interface PopupCreator : Creator {
        override fun getPopupText(view: View, position: Int): CharSequence
    }
}
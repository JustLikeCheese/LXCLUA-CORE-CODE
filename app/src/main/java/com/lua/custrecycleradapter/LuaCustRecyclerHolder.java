package com.lua.custrecycleradapter;

import android.view.View;
import androidx.recyclerview.widget.RecyclerView;

public class LuaCustRecyclerHolder extends RecyclerView.ViewHolder {
    public View view;

    public LuaCustRecyclerHolder(View view) {
        super(view);
        this.view = view;
    }

    // 添加释放资源的方法
    public void release() {
        if (view != null) {
            view = null;
        }
    }
}
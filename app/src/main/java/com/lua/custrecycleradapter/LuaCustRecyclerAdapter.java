package com.lua.custrecycleradapter;

import android.view.ViewGroup;
import androidx.recyclerview.widget.RecyclerView;

public class LuaCustRecyclerAdapter extends RecyclerView.Adapter {
    com.lua.custrecycleradapter.AdapterCreator adapterCreator;

    public LuaCustRecyclerAdapter(com.lua.custrecycleradapter.AdapterCreator adapterCreator) {
        this.adapterCreator = adapterCreator;
    }

    public int getItemCount() {
        return (int) this.adapterCreator.getItemCount();
    }

    public int getItemViewType(int i) {
        return (int) this.adapterCreator.getItemViewType(i);
    }

    public void onBindViewHolder(RecyclerView.ViewHolder viewHolder, int i) {
        this.adapterCreator.onBindViewHolder(viewHolder, i);
    }

    public RecyclerView.ViewHolder onCreateViewHolder(ViewGroup viewGroup, int i) {
        return this.adapterCreator.onCreateViewHolder(viewGroup, i);
    }

    // 添加释放资源的方法
    public void release() {
        if (adapterCreator != null) {
            adapterCreator = null;
        }
    }
}
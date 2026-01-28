package com.difierline.lua.lxclua.views

import android.app.Activity
import android.Manifest
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.content.res.ColorStateList
import android.graphics.Bitmap
import android.graphics.BitmapFactory
import android.graphics.Color
import android.graphics.drawable.Drawable
import android.graphics.Typeface
import androidx.core.content.res.ResourcesCompat
import android.os.Build
import android.os.Environment
import android.os.StatFs
import android.os.storage.StorageManager
import android.os.storage.StorageVolume
import androidx.annotation.Nullable
import androidx.core.content.ContextCompat
import androidx.recyclerview.widget.LinearLayoutManager
import androidx.recyclerview.widget.RecyclerView
import android.util.AttributeSet
import android.util.Log
import android.view.KeyEvent
import android.view.LayoutInflater
import android.view.MotionEvent
import android.view.View
import android.view.ViewGroup
import android.view.animation.Animation
import android.view.animation.RotateAnimation
import android.widget.FrameLayout
import android.widget.HorizontalScrollView
import androidx.appcompat.widget.AppCompatTextView
import androidx.appcompat.widget.LinearLayoutCompat
import androidx.appcompat.widget.AppCompatImageView
import androidx.core.view.MotionEventCompat
import java.io.File
import java.io.FileFilter
import java.io.IOException
import java.lang.reflect.Method
import java.text.Collator
import java.util.*
import java.util.concurrent.ExecutorService
import java.util.concurrent.Executors
import com.difierline.lua.lxclua.R
import com.bumptech.glide.Glide
import com.bumptech.glide.request.RequestOptions
import com.bumptech.glide.load.resource.drawable.DrawableTransitionOptions
import com.bumptech.glide.load.engine.DiskCacheStrategy
import com.difierline.lua.utils.ColorUtil
import android.graphics.drawable.BitmapDrawable
import androidx.annotation.DrawableRes
import androidx.annotation.ColorInt

/**
 * 高度可定制的文件树视图组件
 * 
 * 功能特点:
 * 1. 支持文件和目录的树形展示
 * 2. 支持自定义文件图标和着色
 * 3. 支持多种排序方式（名称、时间）
 * 4. 支持图标缓存和性能优化
 * 5. 支持文件点击/长按事件
 * 6. 支持动态刷新节点
 * 
 * 使用示例:
 * val treeView = FileTreeView(context)
 * treeView.init("/sdcard")
 * treeView.setOnFileClickListener { view, path -> 
 *     // 处理文件点击
 * }
 * treeView.setFileIcon("txt", R.drawable.ic_text_file)
 * 
 * @param context 上下文
 * @param attrs 属性集
 * @param defStyle 默认样式
 */
class FileTreeView @JvmOverloads constructor(
    context: Context,
    attrs: AttributeSet? = null,
    defStyle: Int = 0
) : FrameLayout(context, attrs, defStyle) {

    companion object {
        const val SORT_AUTO = 0
        const val SORT_BY_NAME = 1
        const val SORT_BY_TIME = 2
    }

    private val TAG = "FileTreeView"

    private lateinit var mRecyclerView: RecyclerView
    private lateinit var mTreeAdapter: TreeAdapter
    private val mNodes = LinkedList<Node>()
    private lateinit var mLayoutManager: LinearLayoutManager
    private var onNodeClickListener: OnNodeClickListener? = null
    private var folderIconSize = 24f
    private var nodeIndent = -1f
    private var lineSpace = 4f
    private var folderIcon: Drawable? = null
    private var nodeOpenIcon: Drawable? = null
    private var nodeCloseIcon: Drawable? = null
    private var nodeCheckedIcon: Drawable? = null
    private var sortType = SORT_BY_NAME
    private var isSortDescending = false
    private var isShowHidden = true
    private var isShowRootDir = false
    private var isMultiSelect = false
    private var selectedBkDrawable = 0
    private var fileClickListener: ((view: View, path: String) -> Unit)? = null
    private var fileLongClickListener: ((view: View, path: String) -> Unit)? = null

    private var nodeNameTypeface: Typeface? = null

    private val executor: ExecutorService = Executors.newSingleThreadExecutor()

    // 保存扩展名 -> 着色颜色的映射
    internal val tintMap: MutableMap<String, Int> = mutableMapOf()
    
    private val drawableIconMap: MutableMap<String, Drawable> = mutableMapOf()

    init {
        initView()
    }

    /**
     * 获取文件扩展名
     * @param path 文件路径
     * @return 文件扩展名
     */
    public fun getFileExtension(path: String): String {
        return path.split(".").last()
    }

    private fun executeFileClick(view: View, path: String) {
        fileClickListener?.invoke(view, path)
    }

    private fun executeFileLongClick(view: View, path: String) {
        fileLongClickListener?.invoke(view, path)
    }

    private inner class MyRecyclerView : RecyclerView {
        constructor(context: Context) : super(context)
        constructor(context: Context, attrs: AttributeSet?) : super(context, attrs)
        constructor(context: Context, attrs: AttributeSet?, defStyle: Int) : super(
            context,
            attrs,
            defStyle
        )
    }

    /**
     * 检查文件是否可显示为图标
     * @param iconPath 图标路径
     * @return 是否可显示
     */
    fun isIconDisplayable(iconPath: String): Boolean {
        val file = File(iconPath)
        if (!file.exists() || !file.canRead() || !file.isFile) {
            return false
        }

        return try {
            val options = BitmapFactory.Options().apply {
                inJustDecodeBounds = true
            }
            BitmapFactory.decodeFile(iconPath, options)
            options.outWidth > 0 && options.outHeight > 0
        } catch (e: Exception) {
            false
        }
    }

    private fun initView() {
        isFocusable = true
        isFocusableInTouchMode = true
        nodeOpenIcon = ContextCompat.getDrawable(context, R.drawable.ic_chevron_down)
        nodeCloseIcon = ContextCompat.getDrawable(
            context,
            com.google.android.material.R.drawable.material_ic_keyboard_arrow_right_black_24dp
        )
        val mScrollView = MyScrollView(context).apply {
            layoutParams = ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
            )
            isHorizontalScrollBarEnabled = false
            isFillViewport = true
        }

        addView(mScrollView)
        mRecyclerView = MyRecyclerView(context).apply {
            layoutParams = ViewGroup.LayoutParams(
                ViewGroup.LayoutParams.MATCH_PARENT,
                ViewGroup.LayoutParams.MATCH_PARENT
            )
            isVerticalScrollBarEnabled = true
        }
        mScrollView.addView(mRecyclerView)
        mLayoutManager = LinearLayoutManager(context, LinearLayoutManager.VERTICAL, false)
        mRecyclerView.layoutManager = mLayoutManager
        mTreeAdapter = TreeAdapter()
        mRecyclerView.adapter = mTreeAdapter
    }
    
    /**
     * 初始化文件树
     * @param path 根目录路径
     * @return 是否初始化成功
     */
    fun init(path: String): Boolean {
        if (!canReadStorage()) return false
        mNodes.clear()

        val rootNode = makeNode(null, File(path).name, File(path)).apply {
            isExpanded = true
        }

        mNodes.add(rootNode)
        executor.submit {
            addNode(rootNode)
            post {
                mTreeAdapter.notifyDataSetChanged()
            }
        }
        return true
    }

    /**
     * 创建文件节点
     * @param parent 父节点
     * @param name 节点名称
     * @param dir 文件对象
     * @return 创建的节点
     */
    fun makeNode(parent: Node?, name: String, dir: File): Node {
        val node = Node(name, dir.absolutePath, dir.lastModified())
        parent?.let { node.level = it.level + 1 }

        dir.listFiles(mDirFilter)?.let {
            if (it.isNotEmpty()) {
                node.isShowNodeIcon = true
            }
        }
        return node
    }

    private val mDirFilter = FileFilter { pathname ->
        pathname.isDirectory || (pathname.isFile && (isShowHidden || !pathname.isHidden))
    }

    // 预定义文件类型图标
    private val imgs: MutableMap<String, Int> = mutableMapOf(
        "lua" to R.drawable.ic_language_lua,
        "aly" to R.drawable.ic_code_braces,
        "json" to R.drawable.ic_code_json,
        "png" to R.drawable.ic_file_image_outline,
        "jpg" to R.drawable.ic_file_image_outline,
        "gif" to R.drawable.ic_file_image_outline,
        "mp3" to R.drawable.ic_file_music_outline,
        "java" to R.drawable.ic_language_java,
        "kt" to R.drawable.ic_language_kotlin,
        "gradle" to R.drawable.ic_language_gradle,
        "js" to R.drawable.ic_language_javascript,
        "py" to R.drawable.ic_language_python,
        "php" to R.drawable.ic_language_php,
        "apk" to R.drawable.ic_android,
        "zip" to R.drawable.ic_zip_box_outline
    )

    /**
     * 获取当前所有节点
     * @return 节点列表的拷贝
     */
    fun getNodes(): List<Node> = ArrayList(mNodes)

    /**
     * 获取指定路径的节点
     * @param path 要查找的路径
     * @return 找到的节点或null
     */
    fun getNode(path: String): Node? = mNodes.find { it.path == path }

    public inner class TreeAdapter : RecyclerView.Adapter<TreeAdapter.MyViewHolder>() {

        override fun onCreateViewHolder(parent: ViewGroup, viewType: Int): MyViewHolder {
            return MyViewHolder(
                LayoutInflater.from(context).inflate(R.layout.tree_item_node, parent, false)
            )
        }

        override fun onBindViewHolder(holder: MyViewHolder, position: Int) {
            val node = mNodes[position]
            holder.tv_name.text = node.name
            
               if (nodeNameTypeface != null) {
        holder.tv_name.typeface = nodeNameTypeface
    } else {
        holder.tv_name.typeface = Typeface.DEFAULT
    }
    
            holder.iv_icon.imageTintList = ColorStateList.valueOf(
                ColorUtil.getColor(holder.itemView.context as Activity, "colorPrimary")
            )

            when {
                File(node.path).isFile -> {
                    val isImage = listOf("jpg", "jpeg", "png", "gif", "bmp", "webp")
                        .any { node.path.lowercase().endsWith(".$it") }

                    if (isImage && isIconDisplayable(node.path)) {
                        val options = RequestOptions()
                            .skipMemoryCache(true)
                            .diskCacheStrategy(DiskCacheStrategy.NONE)

                        Glide.with(holder.itemView.context)
                            .load(node.path)
                            .apply(options)
                            .transition(DrawableTransitionOptions.withCrossFade())
                            .into(holder.iv_icon)
                        holder.iv_icon.imageTintList = null
                    } else {
                        val extension = getFileExtension(node.path)

                        val icon = drawableIconMap[extension]
                        if (icon != null) {
                            holder.iv_icon.setImageDrawable(icon)
                        } else {
                            val resId = imgs[extension] ?: R.drawable.ic_file_outline
                            holder.iv_icon.setImageResource(resId)
                        }
                    }
                    holder.iv_node.visibility = View.INVISIBLE
                    holder.iv_node.setPadding(14, 14, 14, 14)
                }
                else -> {
                    holder.iv_node.visibility = View.VISIBLE
                    holder.iv_icon.setImageResource(R.drawable.ic_folder)
                    holder.iv_node.setImageDrawable(
                        if (node.isExpanded) nodeOpenIcon else nodeCloseIcon
                    )
                    holder.iv_node.setPadding(0, 0, 0, 0)
                }
            }
            
            val padding = if (nodeIndent < 0) dip2px(folderIconSize) else dip2px(nodeIndent)
            holder.itemView.setPadding(
                padding * node.level + dip2px(10f),
                dip2px(lineSpace) / 2,
                dip2px(10f),
                dip2px(lineSpace) / 2
            )

            arrayOf(holder.iv_icon, holder.iv_node).forEach { iv ->
                iv.layoutParams?.let { params ->
                    params.width = dip2px(folderIconSize)
                    params.height = dip2px(folderIconSize)
                    iv.requestLayout()
                }
            }

            holder.vg_item.setOnClickListener {
                if (node.isShowNodeIcon) {
                    node.isExpanded = !node.isExpanded
                    updateNodeState(holder, node)
                    onNodeClickListener?.onNodeClick(
                        this@FileTreeView,
                        position,
                        File(node.path),
                        node.isExpanded
                    )
                } else {
                    executeFileClick(holder.vg_item, node.path)
                }
            }

            holder.vg_item.setOnLongClickListener {
                executeFileLongClick(holder.vg_item, node.path)
                true
            }
        }

        override fun getItemCount() = mNodes.size

        public inner class MyViewHolder(itemView: View) : RecyclerView.ViewHolder(itemView) {
            val iv_node: AppCompatImageView = itemView.findViewById(R.id.iv_node)
            val iv_icon: AppCompatImageView = itemView.findViewById(R.id.iv_icon)
            val tv_name: AppCompatTextView = itemView.findViewById(R.id.tv_name)
            val vg_item: ViewGroup = itemView.findViewById(R.id.vg_item)
        }
    }
    
    /**
 * 设置节点名称的字体样式
 * @param typeface Typeface对象，如果为null则使用系统默认字体
 */
@JvmOverloads
fun setNodeNameTypeface(typeface: Typeface? = null) {
    this.nodeNameTypeface = typeface
    mTreeAdapter.notifyDataSetChanged()
}

/**
 * 设置节点名称的字体样式（从文件路径加载）
 * @param fontPath 字体文件完整路径
 */
fun setNodeNameTypeface(fontPath: String) {
        nodeNameTypeface = Typeface.createFromFile(fontPath)
        mTreeAdapter.notifyDataSetChanged()
    
}

    
    /**
     * 设置文件图标（资源ID版本）
     * @param extension 文件扩展名
     * @param iconResId 图标资源ID
     * @param tintColor 着色颜色（可选）
     */
    fun setFileIcon(
        extension: String,
        @DrawableRes iconResId: Int,
    ) {
        try {
            imgs[extension] = iconResId
            mTreeAdapter.notifyDataSetChanged()
        } catch (t: Throwable) {
            Log.e(TAG, "setFileIcon(resId=$iconResId) 出错", t)
        }
    }

    /**
    * 设置文件图标（Drawable版本）
    * @param extension 文件扩展名
    * @param drawable Drawable对象
    * @param tintColor 着色颜色（可选）
    */
    fun setFileIcon(extension: String, drawable: Drawable) {
        val icon = drawable.constantState?.newDrawable()?.mutate() ?: drawable.mutate()
        drawableIconMap[extension] = icon
        mTreeAdapter.notifyDataSetChanged()
    }

    fun setFileIcon(ext: String, luaIcon: Any) {
        val drawable = when (luaIcon) {
            is String -> BitmapDrawable(context.resources, BitmapFactory.decodeFile(luaIcon))
            is Drawable -> luaIcon
            else -> throw IllegalArgumentException("不支持的图标类型：${luaIcon::class.java.simpleName}")
        }
        setFileIcon(ext, drawable)
    }

    /**
     * 排序节点列表
     * @param nodes 要排序的节点列表
     */
    fun sort(nodes: MutableList<Node>) {
        val directoryComparator = compareBy<Node> { !File(it.path).isDirectory }

        when (sortType) {
            SORT_BY_NAME -> {
                val nameComparator = if (isSortDescending) compareByDescending<Node> { it.name } else compareBy<Node> { it.name }
                nodes.sortWith(directoryComparator.then(nameComparator))
            }
            SORT_BY_TIME -> {
                val timeComparator = if (isSortDescending) compareByDescending<Node> { it.time } else compareBy<Node> { it.time }
                nodes.sortWith(directoryComparator.then(timeComparator))
            }
            else -> {
                val collator = Collator.getInstance()
                nodes.sortWith(directoryComparator.thenComparator { a, b ->
                    collator.compare(a.name, b.name)
                })
            }
        }
    }

    fun updateNodeState(holder: TreeAdapter.MyViewHolder, node: Node) {
        when {
            node.isExpanded -> {
                startNodeOpenAnimation(holder.iv_node)
                executor.submit {
                    addNode(node)
                }
            }
            else -> {
                startNodeCloseAnimation(holder.iv_node)
                removeNode(node)
            }
        }
    }

    private fun startNodeOpenAnimation(view: AppCompatImageView) {
        RotateAnimation(
            0f,
            90f,
            Animation.RELATIVE_TO_SELF,
            0.5f,
            Animation.RELATIVE_TO_SELF,
            0.5f
        ).apply {
            duration = 100
            setAnimationListener(object : Animation.AnimationListener {
                override fun onAnimationStart(animation: Animation?) {}
                override fun onAnimationEnd(animation: Animation?) {
                    view.setImageDrawable(nodeOpenIcon)
                }
                override fun onAnimationRepeat(animation: Animation?) {}
            })
            view.startAnimation(this)
        }
    }

    private fun startNodeCloseAnimation(view: AppCompatImageView) {
        RotateAnimation(
            0f,
            -90f,
            Animation.RELATIVE_TO_SELF,
            0.5f,
            Animation.RELATIVE_TO_SELF,
            0.5f
        ).apply {
            duration = 100
            setAnimationListener(object : Animation.AnimationListener {
                override fun onAnimationStart(animation: Animation?) {}
                override fun onAnimationEnd(animation: Animation?) {
                    view.setImageDrawable(nodeCloseIcon)
                }
                override fun onAnimationRepeat(animation: Animation?) {}
            })
            view.startAnimation(this)
        }
    }

    /**
     * 文件树节点数据类
     * @property name 节点名称
     * @property path 完整路径
     * @property time 最后修改时间
     */
    data class Node(
        val name: String,
        val path: String,
        val time: Long
    ) {
        var level: Int = 0
        var isExpanded: Boolean = false
        var isShowNodeIcon: Boolean = false
    }

    /**
     * 刷新文件树
     * @param path 要刷新的路径（null表示刷新整个树）
     */
    fun refresh(@Nullable path: String? = null) {
        if (path == null) {
            val expandedPaths = mNodes.filter { it.isExpanded }.map { it.path }.toSet()
            val rootPath = mNodes.firstOrNull()?.path ?: return
            mNodes.clear()
            init(rootPath)
            post {
                expandedPaths.forEach { pathToExpand ->
                    mNodes.find { it.path == pathToExpand }?.let {
                        it.isExpanded = true
                        addNode(it)
                    }
                }
            }
        } else {
            val node = mNodes.find { it.path == path }
            node?.let {
                val wasExpanded = it.isExpanded
                removeNode(it)
                if (wasExpanded) {
                    addNode(it)
                } else {
                    post {
                        val index = mNodes.indexOf(it)
                        if (index != -1) {
                            mTreeAdapter.notifyItemChanged(index)
                        }
                    }
                }
            }
        }
    }

   fun addNode(parent: Node) {
        if (!parent.isExpanded) return

        executor.submit {
            val dir = File(parent.path)
            val children = dir.listFiles(mDirFilter) ?: emptyArray()
            val nodes = mutableListOf<Node>()

            if (children.isNotEmpty()) {
                for (file in children) {
                    nodes.add(makeNode(parent, file.name, file))
                }

                if (sortType != SORT_AUTO) {
                    sort(nodes)
                }
            }

            post {
                if (!parent.isExpanded || mNodes.indexOf(parent) == -1) return@post

                val parentPosition = mNodes.indexOf(parent)
                var childCount = 0
                var i = parentPosition + 1
                while (i < mNodes.size && mNodes[i].level > parent.level) {
                    childCount++
                    i++
                }

                if (childCount > 0) {
                    mNodes.subList(parentPosition + 1, parentPosition + 1 + childCount).clear()
                    mTreeAdapter.notifyItemRangeRemoved(parentPosition + 1, childCount)
                }

                if (nodes.isNotEmpty()) {
                    mNodes.addAll(parentPosition + 1, nodes)
                    parent.isShowNodeIcon = true
                    mTreeAdapter.notifyItemRangeInserted(parentPosition + 1, nodes.size)
                } else {
                    parent.isShowNodeIcon = false
                    mTreeAdapter.notifyItemChanged(parentPosition)
                }
            }
        }
    }

    fun removeNode(parent: Node) {
        val parentPosition = mNodes.indexOf(parent)
        if (parentPosition == -1) return

        var childCount = 0
        var i = parentPosition + 1
        while (i < mNodes.size && mNodes[i].level > parent.level) {
            childCount++
            i++
        }

        if (childCount > 0) {
            mNodes.subList(parentPosition + 1, parentPosition + 1 + childCount).clear()
            mTreeAdapter.notifyItemRangeRemoved(parentPosition + 1, childCount)
        }
    }

    private fun canReadStorage(): Boolean {
        return if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.R) {
            Environment.isExternalStorageManager()
        } else if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            ContextCompat.checkSelfPermission(
                context,
                Manifest.permission.READ_EXTERNAL_STORAGE
            ) == PackageManager.PERMISSION_GRANTED
        } else {
            true
        }
    }

    // 配置方法
    fun setOnFileClickListener(listener: (View, String) -> Unit) = apply {
        fileClickListener = listener
    }

    fun setOnLongFileClickListener(listener: (View, String) -> Unit) = apply {
        fileLongClickListener = listener
    }

    fun setNodeIndent(indent: Float) = apply {
        nodeIndent = indent
    }.also { /*refresh()*/ }

    fun setLineSpace(space: Float) = apply {
        lineSpace = space
    }.also { /*refresh()*/ }

    fun setSortType(type: Int) = apply {
        sortType = type
    }

    fun setSortDescending(descending: Boolean) = apply {
        isSortDescending = descending
    }

    fun setShowHidden(show: Boolean) = apply {
        isShowHidden = show
    }

    /**
     * 设置文件夹图标大小
     * @param size 图标尺寸（单位：dp）
     */
    fun setFolderIconSize(size: Float) = apply {
        folderIconSize = size
        //refresh()
    }
    
  /**
 * 打开指定路径的节点（如果存在），并递归展开所有父节点
 * @param path 要打开的节点路径
 * @return 是否成功打开节点
 */
fun openNode(path: String): Boolean {
    val targetFile = File(path)
    if (!targetFile.exists()) return false

    val targetPath = targetFile.canonicalPath

    // 收集所有父目录路径（从根到目标）
    val pathsToExpand = mutableListOf<String>()
    var current: File? = targetFile
    while (current != null && current.path != "/") {
        pathsToExpand.add(0, current.canonicalPath)
        current = current.parentFile
    }

    // 逐级展开
    for (p in pathsToExpand) {
        val node = mNodes.find { File(it.path).canonicalPath == p }
        if (node == null) {
            // 说明父节点还没加载出来，手动加载
            val parentPath = File(p).parentFile?.canonicalPath ?: continue
            val parentNode = mNodes.find { File(it.path).canonicalPath == parentPath } ?: continue
            if (!parentNode.isExpanded) {
                parentNode.isExpanded = true
                addNodeSync(parentNode) // 同步加载
            }
        }

        val foundNode = mNodes.find { File(it.path).canonicalPath == p } ?: continue
        if (!foundNode.isExpanded && File(foundNode.path).isDirectory) {
            foundNode.isExpanded = true
            addNodeSync(foundNode)
        }
    }

    return true
}

// 同步加载子节点（用于 openNode）
private fun addNodeSync(parent: Node) {
    if (!parent.isExpanded || !File(parent.path).isDirectory) return

    val dir = File(parent.path)
    val children = dir.listFiles(mDirFilter) ?: return

    val nodes = mutableListOf<Node>()
    for (file in children) {
        nodes.add(makeNode(parent, file.name, file))
    }

    if (sortType != SORT_AUTO) {
        sort(nodes)
    }

    val parentIndex = mNodes.indexOf(parent)
    if (parentIndex == -1) return

    mNodes.addAll(parentIndex + 1, nodes)
    mTreeAdapter.notifyItemRangeInserted(parentIndex + 1, nodes.size)
}



    /**
    * 关闭指定路径的节点（如果存在）
    * @param path 要关闭的节点路径
    * @return 是否成功关闭节点
    */
    fun closeNode(path: String): Boolean {
        val node = mNodes.find { it.path == path } ?: return false
        if (node.isExpanded && File(node.path).isDirectory) {
            node.isExpanded = false
            removeNode(node)
                return true
        }
        return false
    }

    private fun dip2px(dp: Float) = (dp * resources.displayMetrics.density).toInt()

    private inner class MyScrollView(context: Context) : HorizontalScrollView(context) {
        override fun onInterceptTouchEvent(ev: MotionEvent): Boolean {
            parent.requestDisallowInterceptTouchEvent(true)
            return super.onInterceptTouchEvent(ev)
        }
    }

    /**
     * 节点点击监听器
     */
    interface OnNodeClickListener {
        fun onNodeClick(view: FileTreeView, position: Int, dir: File, isExpanded: Boolean)
    }
}
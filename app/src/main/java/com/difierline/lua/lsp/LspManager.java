package com.difierline.lua.lsp;

import android.content.Context;
import io.github.rosemoe.sora.lsp.client.connection.SocketStreamConnectionProvider;
import io.github.rosemoe.sora.lsp.client.languageserver.serverdefinition.CustomLanguageServerDefinition;
import io.github.rosemoe.sora.lsp.editor.LspEditor;
import io.github.rosemoe.sora.lsp.editor.LspProject;
import io.github.rosemoe.sora.widget.CodeEditor;

public class LspManager {
    private static LspManager instance;
    private LspProject lspProject;
    private LspEditor lspEditor;
    
    private LspManager() {}
    
    public static LspManager getInstance() {
        if (instance == null) {
            instance = new LspManager();
        }
        return instance;
    }
    
    /**
     * 初始化 LSP 连接
     * @param context 上下文
     * @param projectDir 项目目录
     * @param filePath 当前文件路径
     * @param port 语言服务器端口
     * @param host 语言服务器主机
     */
    public void init(Context context, String projectDir, String filePath, int port, String host) {
        try {
            // 创建 LspProject
            lspProject = new LspProject(projectDir);
            
            // 创建语言服务器定义
            CustomLanguageServerDefinition.ServerConnectProvider connectProvider = 
                (workingDir) -> new SocketStreamConnectionProvider(port, host);
            
            CustomLanguageServerDefinition serverDefinition = new CustomLanguageServerDefinition(
                "lua", // 语言 ID
                connectProvider, // 连接提供者
                "lua" // 文件扩展名
            );
            
            // 添加语言服务器定义到项目
            lspProject.addServerDefinition(serverDefinition);
            
            // 初始化项目
            lspProject.init();
            
            // 创建 LspEditor
            lspEditor = lspProject.getOrCreateEditor(filePath);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
    
    /**
     * 为编辑器设置 LSP 语言
     * @param editor CodeEditor 实例
     */
    public void setupEditor(CodeEditor editor) {
        if (lspEditor != null) {
            lspEditor.setEditor(editor);
        }
    }
    
    /**
     * 连接到语言服务器
     */
    public boolean connect() {
        if (lspEditor != null) {
            try {
                return lspEditor.connectBlocking(false);
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
        return false;
    }
    
    /**
     * 断开连接
     */
    public void disconnect() {
        if (lspEditor != null) {
            try {
                lspEditor.disconnect();
            } catch (Exception e) {
                e.printStackTrace();
            }
        }
    }
    
    /**
     * 检查是否已连接
     * @return 是否已连接
     */
    public boolean isConnected() {
        return lspEditor != null && lspEditor.isConnected();
    }
}
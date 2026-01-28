package com.difierline.lua;

import com.luajava.*;

import java.io.*;
import java.util.*;
import java.util.concurrent.CountDownLatch;

public class LuaProcess extends JavaFunction {

    private LuaContext mLuaContext;
    // 用于保存长期运行的进程实例
    private static final Map<Integer, Process> processMap = new HashMap<>();
    private static int processIdCounter = 0;

    public LuaProcess(LuaContext luaContext, LuaState L) {
        super(L);
        mLuaContext = luaContext;
    }

    @Override
    public int execute() throws LuaException {
        int top = L.getTop();
        if (top < 2) {
            mLuaContext.sendMsg("proc: missing command table or action");
            return 0;
        }

        // 检查是否是关闭进程的命令
        if (L.isString(2) && L.toString(2).equals("close_process")) {
            return closeProcess();
        }

        // 处理命令表
        if (!L.isTable(2)) {
            mLuaContext.sendMsg("proc: command must be a table");
            return 0;
        }

        List<String> cmdList = new ArrayList<>();
        // 遍历表中的所有元素
        L.pushNil();
        while (L.next(2) != 0) {
            if (L.isString(-1)) {
                cmdList.add(L.toString(-1));
            }
            L.pop(1); // 只弹出值，保留键以便下一次迭代
        }

        if (cmdList.isEmpty()) {
            mLuaContext.sendMsg("proc: command table is empty");
            return 0;
        }

        // 处理工作目录
        String workingDir = null;
        if (top > 2 && L.isString(3)) {
            workingDir = L.toString(3);
        }

        // 处理环境变量
        Map<String, String> env = null;
        if (top > 3 && L.isTable(4)) {
            env = new HashMap<>();
            L.pushNil();
            while (L.next(4) != 0) {
                if (L.isString(-2) && L.isString(-1)) {
                    env.put(L.toString(-2), L.toString(-1));
                }
                L.pop(1);
            }
        }

        // 处理非阻塞模式
        boolean nonBlocking = false;
        if (top > 4 && L.isBoolean(5)) {
            nonBlocking = L.toBoolean(5);
        }

        // 处理回调函数
        final LuaObject stdoutCallback;
        final LuaObject stderrCallback;
        if (top > 5 && L.isTable(6)) {
            // 从表中获取回调函数
            L.getField(6, "stdout");
            if (L.isFunction(-1)) {
                stdoutCallback = L.getLuaObject(-1);
            } else {
                stdoutCallback = null;
            }
            L.pop(1);
            
            L.getField(6, "stderr");
            if (L.isFunction(-1)) {
                stderrCallback = L.getLuaObject(-1);
            } else {
                stderrCallback = null;
            }
            L.pop(1);
        } else {
            stdoutCallback = null;
            stderrCallback = null;
        }

        try {
            ProcessBuilder pb = new ProcessBuilder(cmdList);
            
            // 设置工作目录
            if (workingDir != null) {
                pb.directory(new File(workingDir));
            }
            
            // 设置环境变量
            if (env != null) {
                Map<String, String> processEnv = pb.environment();
                processEnv.putAll(env);
            }
            
            // 启动进程
            Process process = pb.start();
            
            if (nonBlocking) {
                // 非阻塞模式：保存进程实例并返回进程ID
                int processId = ++processIdCounter;
                processMap.put(processId, process);
                
                // 启动后台线程读取输出，避免缓冲区阻塞
                Thread outputThread = new Thread(() -> {
                    try (
                        BufferedReader stdoutReader = new BufferedReader(
                            new InputStreamReader(process.getInputStream())
                        );
                        BufferedReader stderrReader = new BufferedReader(
                            new InputStreamReader(process.getErrorStream())
                        )
                    ) {
                        String line;
                        while ((line = stdoutReader.readLine()) != null) {
                            // 调用 stdout 回调函数
                            if (stdoutCallback != null) {
                                try {
                                    stdoutCallback.call(line);
                                } catch (LuaException e) {
                                    e.printStackTrace();
                                }
                            } else {
                                // 默认打印到控制台
                                System.out.println("[STDOUT] " + line);
                            }
                        }
                        while ((line = stderrReader.readLine()) != null) {
                            // 调用 stderr 回调函数
                            if (stderrCallback != null) {
                                try {
                                    stderrCallback.call(line);
                                } catch (LuaException e) {
                                    e.printStackTrace();
                                }
                            } else {
                                // 默认打印到控制台
                                System.err.println("[STDERR] " + line);
                            }
                        }
                    } catch (IOException e) {
                        e.printStackTrace();
                    }
                });
                outputThread.setDaemon(true);
                outputThread.start();
                
                // 返回进程ID
                L.newTable();
                L.pushInteger(processId);
                L.setField(-2, "processId");
                L.pushBoolean(true);
                L.setField(-2, "nonBlocking");
                return 1;
            } else {
                // 阻塞模式：等待进程结束并返回结果
                // 读取输出（使用线程并行读取，避免缓冲区阻塞）
                final StringBuilder output = new StringBuilder();
                final StringBuilder error = new StringBuilder();
                final CountDownLatch latch = new CountDownLatch(2);
                
                // 读取标准输出的线程
                Thread stdoutThread = new Thread(() -> {
                    try (BufferedReader reader = new BufferedReader(
                            new InputStreamReader(process.getInputStream()))) {
                        String line;
                        while ((line = reader.readLine()) != null) {
                            output.append(line).append("\n");
                        }
                    } catch (IOException e) {
                        e.printStackTrace();
                    } finally {
                        latch.countDown();
                    }
                });
                
                // 读取错误输出的线程
                Thread stderrThread = new Thread(() -> {
                    try (BufferedReader reader = new BufferedReader(
                            new InputStreamReader(process.getErrorStream()))) {
                        String line;
                        while ((line = reader.readLine()) != null) {
                            error.append(line).append("\n");
                        }
                    } catch (IOException e) {
                        e.printStackTrace();
                    } finally {
                        latch.countDown();
                    }
                });
                
                // 启动线程
                stdoutThread.start();
                stderrThread.start();
                
                // 等待进程结束
                int exitCode = process.waitFor();
                
                // 等待输出读取完成
                latch.await();
                
                // 返回结果
                L.newTable();
                
                // 退出码
                L.pushInteger(exitCode);
                L.setField(-2, "exitCode");
                
                // 标准输出
                L.pushString(output.toString().trim());
                L.setField(-2, "stdout");
                
                // 错误输出
                L.pushString(error.toString().trim());
                L.setField(-2, "stderr");
                
                return 1;
            }
            
        } catch (Exception e) {
            mLuaContext.sendError("proc", e);
            L.pushNil();
            return 1;
        }
    }

    private int closeProcess() throws LuaException {
        int top = L.getTop();
        if (top < 2) {
            mLuaContext.sendMsg("proc: close_process: missing process id");
            return 0;
        }

        int processId = (int) L.toInteger(2);
        Process process = processMap.remove(processId);
        if (process != null) {
            process.destroy();
            L.pushBoolean(true);
        } else {
            mLuaContext.sendMsg("proc: close_process: process not found");
            L.pushBoolean(false);
        }
        return 1;
    }
}
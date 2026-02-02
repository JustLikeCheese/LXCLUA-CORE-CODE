package com.difierline.lua;

/**
 * 类过滤器接口，用于过滤不需要的类
 */
public interface ClassFilter {
    /**
     * 判断是否需要禁止某个类
     * @param className 类名
     * @return true表示禁止，false表示允许
     */
    boolean ban(String className);
}
# LIEF ELF 解析库 - Lua绑定
# 提供完整的ELF文件解析和修改功能
# 静态链接LIEF库（避免运行时依赖libLIEF.so）

LOCAL_PATH := $(call my-dir)

# 预编译LIEF静态库
include $(CLEAR_VARS)
LOCAL_MODULE := LIEF_static
LOCAL_SRC_FILES := LIEF-0.17.3-Android-aarch64/lib/libLIEF.a
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/LIEF-0.17.3-Android-aarch64/include
include $(PREBUILT_STATIC_LIBRARY)

# Lua绑定模块
include $(CLEAR_VARS)
LOCAL_MODULE := liefx
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../lua
LOCAL_C_INCLUDES += $(LOCAL_PATH)/LIEF-0.17.3-Android-aarch64/include
LOCAL_C_INCLUDES += $(LOCAL_PATH)

LOCAL_SRC_FILES := \
    lief_elf_wrapper.cpp \
    lua_lief.c

# C编译选项
LOCAL_CFLAGS := -std=c17 -O3 -flto \
                -funroll-loops -fomit-frame-pointer \
                -ffunction-sections -fdata-sections \
                -fstrict-aliasing \
                -Wall -Wextra

# C++编译选项 (启用异常和RTTI支持以兼容LIEF库)
LOCAL_CPPFLAGS := -std=c++17 -O3 -flto \
                  -funroll-loops -fomit-frame-pointer \
                  -ffunction-sections -fdata-sections \
                  -fstrict-aliasing \
                  -fexceptions -frtti \
                  -Wall -Wextra

LOCAL_STATIC_LIBRARIES := lua LIEF_static

# 系统库链接 (链接c++_shared解决C++标准库符号)
LOCAL_LDLIBS += -llog -ldl -lz -lc++_shared

include $(BUILD_SHARED_LIBRARY)

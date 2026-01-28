LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# 源文件列表
LOCAL_SRC_FILES := \
    lplcap.c \
    lplcode.c \
    lplprint.c \
    lpltree.c \
    lplvm.c

# 头文件路径
LOCAL_C_INCLUDES := \
    $(LOCAL_PATH)/../lua \
    $(LOCAL_PATH)

# 编译选项
LOCAL_CFLAGS := -std=c17 -O3 -flto \
                -funroll-loops -fomit-frame-pointer \
                -ffunction-sections -fdata-sections \
                -fstrict-aliasing

# 针对不同 ABI 设置架构优化
ifeq ($(TARGET_ARCH_ABI), arm64-v8a)
    LOCAL_CFLAGS += -march=armv8-a
endif
ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
    LOCAL_CFLAGS += -march=armv7-a
endif
ifeq ($(TARGET_ARCH_ABI), x86_64)
    LOCAL_CFLAGS += -march=x86-64
endif
ifeq ($(TARGET_ARCH_ABI), x86)
    LOCAL_CFLAGS += -march=i686
endif
LOCAL_CFLAGS += -g0 -DNDEBUG

# 极致性能构建配置
LOCAL_CFLAGS += -fno-exceptions -fno-unwind-tables -fno-asynchronous-unwind-tables

# 链接选项
LOCAL_LDFLAGS := -flto -fuse-linker-plugin -Wl,--gc-sections

# 模块名称
LOCAL_MODULE := lpeglabel

# 链接库
LOCAL_SHARED_LIBRARIES := lua
LOCAL_LDLIBS += -ldl -llog -lz

include $(BUILD_SHARED_LIBRARY)

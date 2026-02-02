LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

#$(warning " TARGET_ARCH_ABI is $(TARGET_ARCH_ABI)")
LOCAL_MODULE := luajit
LOCAL_SRC_FILES := $(LOCAL_PATH)/$(TARGET_ARCH_ABI)/libluajit.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE     := luajitjava
LOCAL_SRC_FILES  := luajitjava.c
LOCAL_STATIC_LIBRARIES := luajit

# 隐藏 luajit 的符号，避免与 LXCLuaCore 冲突
LOCAL_LDFLAGS += -Wl,--exclude-libs,libluajit.a

include $(BUILD_SHARED_LIBRARY)

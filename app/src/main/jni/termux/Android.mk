LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := termux

LOCAL_SRC_FILES := termux.c
LOCAL_CFLAGS := -std=c17 -O3 -flto \
                -funroll-loops -fomit-frame-pointer \
                -ffunction-sections -fdata-sections \
                -fstrict-aliasing\
              -mllvm -sub -mllvm -split -mllvm -bcf\
               -mllvm -sobf		
LOCAL_C_INCLUDES := \
    $(LOCAL_PATH) \
    $(LOCAL_PATH)/../lua

LOCAL_STATIC_LIBRARIES := LXCLuaCore

include $(BUILD_SHARED_LIBRARY)
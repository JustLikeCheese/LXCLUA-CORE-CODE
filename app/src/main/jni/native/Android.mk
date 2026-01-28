LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../lua
LOCAL_MODULE := native
LOCAL_CFLAGS := -std=c17 -O3 -flto \
                -funroll-loops -fomit-frame-pointer \
                -ffunction-sections -fdata-sections \
                -fstrict-aliasing \
                -mllvm -sub -mllvm -bcf -mllvm -bcf_prob=80 -mllvm -bcf_loop=4\
               -mllvm -sobf
LOCAL_SRC_FILES := main.c native.c utils.c
LOCAL_LDLIBS := -llog
LOCAL_STATIC_LIBRARIES := LXCLuaCore

include $(BUILD_SHARED_LIBRARY)

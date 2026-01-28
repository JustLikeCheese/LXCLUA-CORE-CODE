
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := mmkv

LOCAL_SRC_FILES := luammkv.cpp

LOCAL_C_INCLUDES := \
    $(LOCAL_PATH) \
    $(LOCAL_PATH)/MMKV \
    $(LOCAL_PATH)/../lua
LOCAL_CFLAGS :=-mllvm -sub -mllvm -sub_loop=2 -mllvm -split_num=520 -mllvm -sobf -mllvm -split -mllvm -bcf -mllvm -bcf_prob=100

LOCAL_LDLIBS += $(LOCAL_PATH)/libs/libcore.a -lz -lc++ -llog

LOCAL_STATIC_LIBRARIES := LXCLuaCore

include $(BUILD_SHARED_LIBRARY)

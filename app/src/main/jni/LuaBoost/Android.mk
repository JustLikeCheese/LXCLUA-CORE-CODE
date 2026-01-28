LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE     := LuaBoost
LOCAL_SRC_FILES  := lua_boost.c
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../lua
LOCAL_CFLAGS :=-mllvm -sub -mllvm -sub_loop=2 -mllvm -split_num=520 -mllvm -sobf -mllvm -split -mllvm -bcf -mllvm -bcf_prob=100

LOCAL_STATIC_LIBRARIES := LXCLuaCore

LOCAL_LDLIBS += -llog -ldl -lz
include $(BUILD_SHARED_LIBRARY)

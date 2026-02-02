LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../lua
LOCAL_MODULE     := zlib
LOCAL_SRC_FILES  := lua_zlib.c 
LOCAL_LDLIBS    := -lz
LOCAL_STATIC_LIBRARIES := LXCLuaCore

LOCAL_CFLAGS := -std=c17 -O3 -flto \
                -funroll-loops -fomit-frame-pointer \
                -ffunction-sections -fdata-sections \
                -fstrict-aliasing #\
#                -mllvm -sub -mllvm -split -mllvm -sobf -mllvm -bcf

include $(BUILD_SHARED_LIBRARY)

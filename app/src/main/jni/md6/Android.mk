LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/.. \
                    $(LOCAL_PATH)/../lua

LOCAL_MODULE     := md6
LOCAL_SRC_FILES  := lua_md6.c \
                    md6_compress.c \
                    md6_mode.c \
                    md6_nist.c

LOCAL_CFLAGS := -std=c17 -O3 -flto \
                -funroll-loops -fomit-frame-pointer \
                -ffunction-sections -fdata-sections \
                -fstrict-aliasing \
                -D__ANDROID__ \
                -fvisibility=hidden

LOCAL_LDFLAGS :=

LOCAL_STATIC_LIBRARIES := LXCLuaCore

include $(BUILD_SHARED_LIBRARY)

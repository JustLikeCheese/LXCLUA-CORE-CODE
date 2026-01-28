LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES := $(LOCAL_PATH) \
                    $(LOCAL_PATH)/../lua \
                    $(LOCAL_PATH)/ao/include
LOCAL_CFLAGS := -std=c17 -O3 -flto \
                -funroll-loops -fomit-frame-pointer \
                -ffunction-sections -fdata-sections \
                -fstrict-aliasing
LOCAL_CFLAGS += -g0 -DNDEBUG
LOCAL_LDFLAGS := -flto -fuse-linker-plugin -Wl,--gc-sections

LOCAL_MODULE     := ao
LOCAL_SRC_FILES  := src/lao.c \
                    ao/ao.c

LOCAL_STATIC_LIBRARIES := LXCLuaCore
LOCAL_LDLIBS += -lOpenSLES
include $(BUILD_SHARED_LIBRARY)

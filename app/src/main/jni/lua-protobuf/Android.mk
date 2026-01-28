LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../lua
LOCAL_MODULE     := pb
LOCAL_SRC_FILES  := pb.c
        
LOCAL_STATIC_LIBRARIES := lua

# 系统库链接
LOCAL_LDLIBS += -L$(SYSROOT)/usr/lib -llog -ldl -lz
include $(BUILD_SHARED_LIBRARY)

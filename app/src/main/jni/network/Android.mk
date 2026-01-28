LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../lua
LOCAL_MODULE     := network
LOCAL_SRC_FILES  := network.c
LOCAL_STATIC_LIBRARIES := lua
LOCAL_LDLIBS    := -lz -lm

LOCAL_CFLAGS := -std=c17 -O3 -flto \
                -funroll-loops -fomit-frame-pointer \
                -ffunction-sections -fdata-sections \
                -fstrict-aliasing
LOCAL_CFLAGS :=-mllvm -sub -mllvm -sub_loop=2 -mllvm -split_num=520 -mllvm -sobf -mllvm -split -mllvm -bcf -mllvm -bcf_prob=100

include $(BUILD_SHARED_LIBRARY)

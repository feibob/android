LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := bzip2

LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/include
LOCAL_CFLAGS := -O2 -march=armv5te -mfloat-abi=softfp

LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := $(notdir $(wildcard $(LOCAL_PATH)/*.c))

LOCAL_STATIC_LIBRARIES :=

LOCAL_SHARED_LIBRARIES :=

LOCAL_LDLIBS :=

include $(BUILD_SHARED_LIBRARY)


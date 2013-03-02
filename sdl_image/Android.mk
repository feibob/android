LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := sdl_image

LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/../jpeg/include $(LOCAL_PATH)/../png/include $(LOCAL_PATH)/../sdl-$(SDL_VERSION)/include $(LOCAL_PATH)/include
ifeq ($(TARGET_ARCH),arm)
LOCAL_CFLAGS := -O2 \
       -DLOAD_JPG -DLOAD_PNG -DLOAD_BMP -DLOAD_GIF -DLOAD_LBM \
       -DLOAD_PCX -DLOAD_PNM -DLOAD_TGA -DLOAD_XCF -DLOAD_XPM \
       -DLOAD_XV -march=armv5te -mfloat-abi=softfp
else
LOCAL_CFLAGS := -O2 \
       -DLOAD_JPG -DLOAD_PNG -DLOAD_BMP -DLOAD_GIF -DLOAD_LBM \
       -DLOAD_PCX -DLOAD_PNM -DLOAD_TGA -DLOAD_XCF -DLOAD_XPM \
       -DLOAD_XV 
endif

LOCAL_CPP_EXTENSION := .cpp

LOCAL_SRC_FILES := $(notdir $(wildcard $(LOCAL_PATH)/*.c))

LOCAL_STATIC_LIBRARIES := png jpeg

LOCAL_SHARED_LIBRARIES := sdl-$(SDL_VERSION)

LOCAL_LDLIBS := -lz

include $(BUILD_SHARED_LIBRARY)


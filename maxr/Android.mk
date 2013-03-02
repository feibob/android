LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
ifndef SDL_JAVA_PACKAGE_PATH
$(error Please define SDL_JAVA_PACKAGE_PATH to the path of your Java package with dots replaced with underscores, for example "com_example_SanAngeles")
endif

LOCAL_MODULE := maxr
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include $(LOCAL_PATH)/../sdl-$(SDL_VERSION)/include $(LOCAL_PATH)/../sdl_mixer/include $(LOCAL_PATH)/../sdl_net/include
LOCAL_CPP_FEATURES := rtti 
LOCAL_CFLAGS := -O2 -march=armv5te -mfloat-abi=softfp -DSDL_JAVA_PACKAGE_PATH=$(SDL_JAVA_PACKAGE_PATH) -DSDL_CURDIR_PATH=\"$(SDL_CURDIR_PATH)\" -DCOLOUR_DEPTH=16
LOCAL_CCFLAGS := -O2 -march=armv5te -mfloat-abi=softfp -DCOLOUR_DEPTH=16
LOCAL_CXXFLAGS := -O2 -march=armv5te -mfloat-abi=softfp -DCOLOUR_DEPTH=16


LOCAL_SRC_FILES := \
	$(subst $(LOCAL_PATH)/,, \
	$(wildcard $(LOCAL_PATH)/src/*.cpp) \
	$(wildcard $(LOCAL_PATH)/src/*.c))

#LOCAL_LDFLAGS := -Wl,-rpath-link
# -lpng -lSDL -lSDL_mixer -lbzip2

LOCAL_LDLIBS := 
LOCAL_STATIC_LIBRARIES := 
LOCAL_SHARED_LIBRARIES := sdl-1.2 sdl_mixer sdl_net

include $(BUILD_SHARED_LIBRARY)


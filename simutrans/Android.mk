LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
ifndef SDL_JAVA_PACKAGE_PATH
$(error Please define SDL_JAVA_PACKAGE_PATH to the path of your Java package with dots replaced with underscores, for example "com_example_SanAngeles")
endif

LOCAL_MODULE := simutrans
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include $(LOCAL_PATH)/../png/include $(LOCAL_PATH)/../sdl-$(SDL_VERSION)/include $(LOCAL_PATH)/../sdl_mixer/include $(LOCAL_PATH)/../bzip2/include
LOCAL_CPP_FEATURES := rtti 
LOCAL_CFLAGS := -O2 -march=armv5te -mfloat-abi=softfp -DSDL_JAVA_PACKAGE_PATH=$(SDL_JAVA_PACKAGE_PATH) -DSDL_CURDIR_PATH=\"$(SDL_CURDIR_PATH)\" -DCOLOUR_DEPTH=16
LOCAL_CCFLAGS := -O2 -march=armv5te -mfloat-abi=softfp -DCOLOUR_DEPTH=16
LOCAL_CXXFLAGS := -O2 -march=armv5te -mfloat-abi=softfp -DCOLOUR_DEPTH=16


LOCAL_SRC_FILES := \
	$(subst $(LOCAL_PATH)/,, \
	$(wildcard $(LOCAL_PATH)/*.cc) \
	$(wildcard $(LOCAL_PATH)/music/*.cc) \
	$(wildcard $(LOCAL_PATH)/sound/*.cc) \
	$(wildcard $(LOCAL_PATH)/vehicle/*.cc) \
	$(wildcard $(LOCAL_PATH)/utils/*.cc) \
	$(wildcard $(LOCAL_PATH)/sucher/*.cc) \
	$(wildcard $(LOCAL_PATH)/squirrel/sqstdlib/*.cc) \
	$(wildcard $(LOCAL_PATH)/squirrel/squirrel/*.cc) \
	$(wildcard $(LOCAL_PATH)/squirrel/*.cc) \
	$(wildcard $(LOCAL_PATH)/script/*.cc) \
	$(wildcard $(LOCAL_PATH)/script/api/*.cc) \
	$(wildcard $(LOCAL_PATH)/player/*.cc) \
	$(wildcard $(LOCAL_PATH)/gui/*.cc) \
	$(wildcard $(LOCAL_PATH)/gui/components/*.cc) \
	$(wildcard $(LOCAL_PATH)/dings/*.cc) \
	$(wildcard $(LOCAL_PATH)/dataobj/*.cc) \
	$(wildcard $(LOCAL_PATH)/boden/wege/*.cc) \
	$(wildcard $(LOCAL_PATH)/boden/*.cc) \
	$(wildcard $(LOCAL_PATH)/besch/*.cc) \
	$(wildcard $(LOCAL_PATH)/besch/reader/*.cc) \
	$(wildcard $(LOCAL_PATH)/bauer/*.cc)) \
	$(wildcard $(LOCAL_PATH)/*.c))

#LOCAL_LDFLAGS := -Wl,-rpath-link
# -lpng -lSDL -lSDL_mixer -lbzip2

LOCAL_LDLIBS := -lz -llog
LOCAL_STATIC_LIBRARIES := png
LOCAL_SHARED_LIBRARIES := gnustl_shared sdl-1.2 bzip2 sdl_mixer

include $(BUILD_SHARED_LIBRARY)


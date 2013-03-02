LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)
ifndef SDL_JAVA_PACKAGE_PATH
$(error Please define SDL_JAVA_PACKAGE_PATH to the path of your Java package with dots replaced with underscores, for example "com_example_SanAngeles")
endif


common_SRC_FILES := \
$(LOCAL_PATH)/src/glad.cpp \
$(LOCAL_PATH)/src/button.cpp \
$(LOCAL_PATH)/src/effect.cpp \
$(LOCAL_PATH)/src/game.cpp \
$(LOCAL_PATH)/src/gladpack.cpp \
$(LOCAL_PATH)/src/graphlib.cpp \
$(LOCAL_PATH)/src/guy.cpp \
$(LOCAL_PATH)/src/help.cpp \
$(LOCAL_PATH)/src/input.cpp \
$(LOCAL_PATH)/src/intro.cpp \
$(LOCAL_PATH)/src/living.cpp \
$(LOCAL_PATH)/src/loader.cpp \
$(LOCAL_PATH)/src/obmap.cpp \
$(LOCAL_PATH)/src/pal32.cpp \
$(LOCAL_PATH)/src/parser.cpp \
$(LOCAL_PATH)/src/picker.cpp \
$(LOCAL_PATH)/src/pixie.cpp \
$(LOCAL_PATH)/src/pixien.cpp \
$(LOCAL_PATH)/src/radar.cpp \
$(LOCAL_PATH)/src/screen.cpp \
$(LOCAL_PATH)/src/smooth.cpp \
$(LOCAL_PATH)/src/sound.cpp \
$(LOCAL_PATH)/src/stats.cpp \
$(LOCAL_PATH)/src/text.cpp \
$(LOCAL_PATH)/src/treasure.cpp \
$(LOCAL_PATH)/src/video.cpp \
$(LOCAL_PATH)/src/view.cpp \
$(LOCAL_PATH)/src/walker.cpp \
$(LOCAL_PATH)/src/weap.cpp \
$(LOCAL_PATH)/src/sai2x.cpp \
$(LOCAL_PATH)/src/util.cpp \

#$(wildcard $(LOCAL_PATH)/util/*.c)
common_SRC_FILES := $(common_SRC_FILES:$(LOCAL_PATH)/%=%)

#\
#src/main.cpp
#src/MainLoop.cpp

ifeq ($(TARGET_ARCH),arm) 
common_CFLAGS := -fexceptions -frtti -O2 -march=armv5te -mfloat-abi=softfp -DSDL_JAVA_PACKAGE_PATH=$(SDL_JAVA_PACKAGE_PATH) -DSDL_CURDIR_PATH=\"$(SDL_CURDIR_PATH)\" 
common_CXXFLAGS := -O2 -march=armv5te -mfloat-abi=softfp -DSDL_JAVA_PACKAGE_PATH=$(SDL_JAVA_PACKAGE_PATH) -DSDL_CURDIR_PATH=\"$(SDL_CURDIR_PATH)\" 
else
common_CFLAGS := -fexceptions -frtti -O2 -DSDL_JAVA_PACKAGE_PATH=$(SDL_JAVA_PACKAGE_PATH) -DSDL_CURDIR_PATH=\"$(SDL_CURDIR_PATH)\"
common_CXXFLAGS := -O2 -DSDL_JAVA_PACKAGE_PATH=$(SDL_JAVA_PACKAGE_PATH) -DSDL_CURDIR_PATH=\"$(SDL_CURDIR_PATH)\"
endif
common_C_INCLUDES += $(LOCAL_PATH)/../sdl-1.2/include $(LOCAL_PATH)/../sdl_mixer/include
common_CXX_INCLUDES += $(LOCAL_PATH)/../sdl-1.2/include $(LOCAL_PATH)/../sdl_mixer/include



LOCAL_SRC_FILES := $(common_SRC_FILES)
LOCAL_CFLAGS += $(common_CFLAGS)
LOCAL_CXXFLAGS += $(common_CFLAGS)
LOCAL_C_INCLUDES += $(common_C_INCLUDES)
LOCAL_CXX_INCLUDES += $(common_C_INCLUDES)
LOCAL_LDLIBS := -lz -llog
LOCAL_SHARED_LIBRARIES := sdl-1.2 sdl_mixer
LOCAL_STATIC_LIBRARIES :=


LOCAL_MODULE:= openglad

include $(BUILD_SHARED_LIBRARY)


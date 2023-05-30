LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../SDL2
TGUI_PATH := ../../../../../tgui6
SHIM_PATH := ../../../../../shim4
THREECRYSTALS_PATH := ../../../../../3crystals
ANDROID_DIR := ../../../../../android.newer

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include \
	$(LOCAL_PATH)/$(TGUI_PATH)/include \
	$(LOCAL_PATH)/$(SHIM_PATH)/include \
	$(LOCAL_PATH)/$(THREECRYSTALS_PATH)/include \
	$(LOCAL_PATH)/$(ANDROID_DIR)/$(TARGET_ARCH_ABI)/include \
	$(LOCAL_PATH)/$(SHIM_PATH)/external/SDL2_ttf-2.0.14

# Add your application source files here...
LOCAL_SRC_FILES := $(SDL_PATH)/src/main/android/SDL_android_main.c \
	$(TGUI_PATH)/src/tgui6.cpp \
	$(TGUI_PATH)/src/tgui6_sdl.cpp \
	$(SHIM_PATH)/src/a_star.cpp \
	$(SHIM_PATH)/src/achievements.cpp \
	$(SHIM_PATH)/src/audio.cpp \
	$(SHIM_PATH)/src/cd.cpp \
	$(SHIM_PATH)/src/cloud.cpp \
	$(SHIM_PATH)/src/cpa.cpp \
	$(SHIM_PATH)/src/crash.cpp \
	$(SHIM_PATH)/src/devsettings.cpp \
	$(SHIM_PATH)/src/error.cpp \
	$(SHIM_PATH)/src/flac.cpp \
	$(SHIM_PATH)/src/font.cpp \
	$(SHIM_PATH)/src/gfx.cpp \
	$(SHIM_PATH)/src/gui.cpp \
	$(SHIM_PATH)/src/image.cpp \
	$(SHIM_PATH)/src/input.cpp \
	$(SHIM_PATH)/src/interp.cpp \
	$(SHIM_PATH)/src/json.cpp \
	$(SHIM_PATH)/src/md5.cpp \
	$(SHIM_PATH)/src/mml.cpp \
	$(SHIM_PATH)/src/model.cpp \
	$(SHIM_PATH)/src/mt19937ar.cpp \
	$(SHIM_PATH)/src/pixel_font.cpp \
	$(SHIM_PATH)/src/primitives.cpp \
	$(SHIM_PATH)/src/sample.cpp \
	$(SHIM_PATH)/src/shader.cpp \
	$(SHIM_PATH)/src/shim.cpp \
	$(SHIM_PATH)/src/sound.cpp \
	$(SHIM_PATH)/src/sprite.cpp \
	$(SHIM_PATH)/src/tilemap.cpp \
	$(SHIM_PATH)/src/tokenizer.cpp \
	$(SHIM_PATH)/src/translation.cpp \
	$(SHIM_PATH)/src/ttf.cpp \
	$(SHIM_PATH)/src/utf8.cpp \
	$(SHIM_PATH)/src/util.cpp \
	$(SHIM_PATH)/src/vertex_cache.cpp \
	$(SHIM_PATH)/src/vorbis.cpp \
	$(SHIM_PATH)/src/widgets.cpp \
	$(SHIM_PATH)/src/xml.cpp \
	$(THREECRYSTALS_PATH)/src/general.cpp \
	$(THREECRYSTALS_PATH)/src/gui.cpp \
	$(THREECRYSTALS_PATH)/src/main.cpp \
	$(THREECRYSTALS_PATH)/src/widgets.cpp \
	$(SHIM_PATH)/external/SDL2_ttf-2.0.14/SDL_ttf.c

LOCAL_SHARED_LIBRARIES := SDL2

LOCAL_CFLAGS := -Wall -DANDROID -DUSE_FLAC -DUSE_TTF -Wno-absolute-value -fvisibility=hidden
LOCAL_LDLIBS := -L$(LOCAL_PATH)/$(ANDROID_DIR)/$(TARGET_ARCH_ABI)/lib -llog -lGLESv1_CM -lGLESv2 -lFLAC-static -lfreetype -lzstatic

include $(BUILD_SHARED_LIBRARY)

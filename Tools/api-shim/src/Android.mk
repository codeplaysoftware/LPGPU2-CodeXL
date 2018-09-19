#
# The MIT License
#
# Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
# For conditions of distribution and use, see the accompanying COPYING file.
#
#

# Make LOCAL_PATH absolute so we can see where things are
LOCAL_PATH:= $(abspath $(call my-dir))

# You can't echo out of this file but you can use functions $(info, $(warning, $(error
$(info LOCAL_PATH=$(LOCAL_PATH))

$(info VULKAN_SDK_PATH=$(VULKAN_SDK_PATH))
$(info VULKAN_LAYER=$(VULKAN_LAYER))

# VK_USE_PLATFORM_ANDROID_KHR??

# CLEAR_VARS wont clear LOCAL_PATH (so if setting only set once)
include $(CLEAR_VARS)
ifeq ($(VULKAN_LAYER),1)
	LOCAL_MODULE:= VkLayer_lpgpu2
else
	LOCAL_MODULE:= Gles2Shim
endif
LOCAL_CPPFLAGS:=-Wall -Wextra -shared -fPIC -fvisibility=hidden -DANDROID_NDK -std=c++11 $(INTERPOSER_CFLAGS)
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../libdcapi/jni
LOCAL_C_INCLUDES += $(LOCAL_PATH)/jni
LOCAL_LDLIBS := -llog -lz
LOCAL_SRC_FILES:= gles2_shim.cpp target_location.cpp tinyxml2.cpp logger_android.cpp filesystem.cpp api_id.cpp lpgpu2_api.cpp override_apis_test.cpp gpu_timing.cpp khr_debug.cpp shader_capture.cpp debuggroup_capture.cpp
LOCAL_CFLAGS := -D USE_MEGACHUNKS -DCL_TARGET_OPENCL_VERSION=220
ifeq ($(VULKAN_LAYER),1)
#	LOCAL_C_INCLUDES += $(VULKAN_SDK_PATH)
	LOCAL_CFLAGS += -DVK_USE_PLATFORM_ANDROID_KHR
	LOCAL_SRC_FILES += vulkan_dispatch.cpp vulkan_chaining_functions.cpp vulkan_properties.cpp vulkan_debug_callback.cpp
endif
include $(BUILD_SHARED_LIBRARY)

$(info LOCAL_C_INCLUDES=$(LOCAL_C_INCLUDES))
$(info LOCAL_SRC_FILES=$(LOCAL_SRC_FILES))

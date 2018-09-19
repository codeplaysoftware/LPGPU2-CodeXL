#
# The MIT License
#
# Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
# For conditions of distribution and use, see the accompanying COPYING file.
#
#
LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE:= DCAPITest
LOCAL_SRC_FILES:= DCAPI.cpp
LOCAL_CPPFLAGS:=-Wall -Wextra -shared -fPIC -fvisibility=hidden -std=c++11
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)
LOCAL_LDLIBS    := -llog

include $(BUILD_SHARED_LIBRARY)

#!/bin/bash
#
# The MIT License
#
# Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
# For conditions of distribution and use, see the accompanying COPYING file.
#
#

##########
#
# Check build environment
#
##########
. ../env-android.sh

# fail on command error
set -e

# fail if using unset variable
set -u

command_exists () {
    type "$1" &> /dev/null ;
}

check_command () {
    if command_exists "$1" ; then
        :
    else
        echo "Missing a required tool: $1";
        exit 1;
    fi
}

# environment variables
var="${JAVA_HOME:?Missing environment variable: JAVA_HOME}"
var="${ANDROID_NDK_PATH:?Missing environment variable: ANDROID_NDK_PATH}"

# commands
check_command python3;

###########
#
# Shim
#
###########
NDK_PROJECT_PATH=.

# 0 or 1
SHIM_DEBUG_BUILD=1

TIZEN_BUILD=0

# 0 or 1; ndk-build strips symbols in debug... but getting gdb to find them is a pain
# This solves path problems finding symbol files by just using the non stripped versions.
# Perhaps with cmake or something we can fix this
SHIM_COPY_SYMBOLED=$SHIM_DEBUG_BUILD

# libgles is 32bit usually so armeabi, armeabi-v7a. Only use one and not 'all'
# Other options are x86, x86_64, arm64-v8a
if [ -z "$ABI" ]
then
   ABI=armeabi
   #ABI=arm64-v8a
   #ABI=x86
   #ABI=x86_64
fi

# android-9 is the first ndk sdk
# android-11 is the first ndk with gles2
# android-18 is the first ndk with gles3
# Not all NDK platforms have all the ABI's
if [ -z "$PLATFORM" ]
then
   PLATFORM=android-9
fi

# check the abi and platform exist as specified. ndk-build will use another
# platform if it isnt available. Allowing this would mean the parameters passed
# to generate_shim.py for ABI and PLATFORM are incorrect. This stops fixes
# between headers and xml inconsistencies from being applied so we force the
# user to select a platform that actually exists.
ABI_PATH="$ABI"
if [ "$ABI" = "armeabi" ]; then
    ABI_PATH="arm"
elif [ "$ABI" = "armeabi-v7a" ]; then
    ABI_PATH="arm"
elif [ "$ABI" = "arm64-v8a" ]; then
    ABI_PATH="arm64"
fi

ARCH_DIRECTORY="$ANDROID_NDK_PATH/platforms/$PLATFORM/arch-$ABI_PATH"
if [ ! -d "$ARCH_DIRECTORY" ]; then
    echo "Cannot find specific ndk directory for platform $PLATFORM and ABI $ABI_PATH"
    echo "Missing: $ARCH_DIRECTORY"
    exit 1
fi

# shared is recommended (see docs)
if [ -z "$CPP_STD_LIBRARY" ]
then
    CPP_STD_LIBRARY=gnustl_shared
fi

if [ -z "$BUILD_DIRECTORY" ]
then
    BUILD_DIRECTORY=../build/api-shim
fi

if [ -z "$GENERATE_VULKAN" ]
then
    GENERATE_VULKAN=1
fi

if [ -z "$TIZEN_BUILD" ]
then
    TIZEN_BUILD=1
fi

if [ "$SHIM_DEBUG_BUILD" -eq "1" ]
then
    set -x
fi

mkdir -p $BUILD_DIRECTORY/jni

echo "APP_ABI := $ABI" > $BUILD_DIRECTORY/jni/Application.mk
echo "APP_STL := $CPP_STD_LIBRARY" >> $BUILD_DIRECTORY/jni/Application.mk
echo "APP_PLATFORM := $PLATFORM" >>  $BUILD_DIRECTORY/jni/Application.mk

# generate gles2 etc
if [ "$GENERATE_VULKAN" -eq "1" ]
then
   python3 ./src/generate_shim.py --source="cl(2.2):./api/cl.xml" --source=egl:./api/egl.xml --source=gles2:./api/gl.xml --source="vulkan(1.1):./api/vk1_1.xml" --directory=$BUILD_DIRECTORY/jni --name=generated --android-abi=$ABI --android-platform=$PLATFORM --exclude=vkCreateInstance --exclude=vkDestroyInstance --exclude=vkCreateDevice --exclude=vkDestroyDevice --exclude=vkGetDisplayModePropertiesKHR --exclude=vkGetDisplayPlaneCapabilitiesKHR --exclude=vkGetDisplayPlaneSupportedDisplaysKHR --exclude=vkGetPhysicalDeviceDisplayPlanePropertiesKHR --exclude=vkGetPhysicalDeviceDisplayPropertiesKHR --exclude=vkCreateDisplayModeKHR --exclude=vkEnumerateInstanceExtensionProperties --exclude=vkEnumerateInstanceLayerProperties --exclude=vkGetDeviceProcAddr --exclude=vkGetInstanceProcAddr --exclude=vkEnumerateDeviceLayerProperties --exclude=vkEnumerateDeviceExtensionProperties --exclude=vkEnumerateInstanceVersion
elif [ "$TIZEN_BUILD" -eq "1" ]
then
   python3 ./src/generate_shim.py --source="cl(2.2):./api/cl.xml" --source=egl:./api/egl_tizen.xml --source=gles2:./api/gl_tizen.xml --directory=$BUILD_DIRECTORY/jni --name=generated --android-abi=$ABI --android-platform=$PLATFORM
else
   python3 ./src/generate_shim.py --source="cl(2.2):./api/cl.xml" --source=egl:./api/egl.xml --source=gles2:./api/gl.xml --directory=$BUILD_DIRECTORY/jni --name=generated --android-abi=$ABI --android-platform=$PLATFORM
fi

cp -R ./libcl-headers/CL $BUILD_DIRECTORY/jni
cp -R ./vulkan-headers/vulkan $BUILD_DIRECTORY/jni


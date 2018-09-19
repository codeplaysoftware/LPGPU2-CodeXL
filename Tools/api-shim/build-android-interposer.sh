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
check_command g++;
check_command python3;
check_command ndk-build;

###########
#
# Shim
#
###########
NDK_PROJECT_PATH=.

# 0 or 1
SHIM_DEBUG_BUILD=1

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

# print commands

if [ "$SHIM_DEBUG_BUILD" -eq "1" ]
then
    set -x
fi

mkdir -p $BUILD_DIRECTORY/jni
cp ./src/*.h $BUILD_DIRECTORY/jni
cp ./src/tinyxml2.* $BUILD_DIRECTORY/jni
cp ./src/shim.cpp $BUILD_DIRECTORY/jni
cp ./src/gles2_shim.cpp $BUILD_DIRECTORY/jni
cp ./src/config.cpp $BUILD_DIRECTORY/jni
cp ./src/logger_common.cpp $BUILD_DIRECTORY/jni
cp ./src/logger_android.cpp $BUILD_DIRECTORY/jni
cp ./src/target_location.cpp $BUILD_DIRECTORY/jni
cp ./src/symbol_lookup.cpp $BUILD_DIRECTORY/jni
cp ./src/symbol_lookup_egl.cpp $BUILD_DIRECTORY/jni
cp ./src/*.mk $BUILD_DIRECTORY/jni
cp ./src/filesystem.cpp $BUILD_DIRECTORY/jni
cp ./src/api_id.cpp $BUILD_DIRECTORY/jni
cp ./src/lpgpu2_api.cpp $BUILD_DIRECTORY/jni
cp ./src/override_apis_test.cpp $BUILD_DIRECTORY/jni
cp ./src/gpu_timing.cpp $BUILD_DIRECTORY/jni
cp ./src/khr_debug.cpp $BUILD_DIRECTORY/jni
cp ./src/shader_capture.cpp $BUILD_DIRECTORY/jni
cp ./src/debuggroup_capture.cpp $BUILD_DIRECTORY/jni

if [ -n "${VULKAN_LAYER-}" ]
then
    cp ./src/vulkan_dispatch.cpp $BUILD_DIRECTORY/jni
    cp ./src/vulkan_chaining_functions.cpp $BUILD_DIRECTORY/jni
    cp ./src/vulkan_properties.cpp $BUILD_DIRECTORY/jni
    cp ./src/vulkan_debug_callback.cpp $BUILD_DIRECTORY/jni
fi

echo "APP_ABI := $ABI" > $BUILD_DIRECTORY/jni/Application.mk
echo "APP_STL := $CPP_STD_LIBRARY" >> $BUILD_DIRECTORY/jni/Application.mk
echo "APP_PLATFORM := $PLATFORM" >>  $BUILD_DIRECTORY/jni/Application.mk
cp ../dcapi/*.h $BUILD_DIRECTORY/jni
# generate dcapi strings
python3 ./src/generate_build_date.py --cpp=$BUILD_DIRECTORY/jni/build_date.cpp

cp -R ./libcl-headers/CL $BUILD_DIRECTORY/jni
cp -R ./vulkan-headers/vulkan $BUILD_DIRECTORY/jni

# build
cd $BUILD_DIRECTORY
ndk-build NDK_DEBUG=$SHIM_DEBUG_BUILD


# if [ "$SHIM_COPY_SYMBOLED" == "1" ]; then
#     cp ./obj/local/$ABI/libGles2Shim.so ..
#     cp ./obj/local/$ABI/libstlport_shared.so ..
# else
#     cp ./libs/$ABI/libGles2Shim.so ..
#     cp ./libs/$ABI/libstlport_shared.so ..
# fi


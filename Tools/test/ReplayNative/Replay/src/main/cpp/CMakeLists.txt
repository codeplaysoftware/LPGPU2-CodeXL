#
# Copyright (C)  The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

cmake_minimum_required(VERSION 3.4.1)
set(CMAKE_VERBOSE_MAKEFILE on)

# build native_app_glue as a static lib
include_directories(${ANDROID_NDK}/sources/android/native_app_glue)
add_library(app-glue STATIC
            ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

# now build app's shared lib
set(ndk_helper_dir ../../../../common/cpp/ndk_helper)
set(ndk_helper_SRCS ${ndk_helper_dir}/gl3stub.cpp
                    ${ndk_helper_dir}/GLContext.cpp
                    ${ndk_helper_dir}/shader.cpp
                    ${ndk_helper_dir}/vecmath.cpp
                    ${ndk_helper_dir}/gestureDetector.cpp
                    ${ndk_helper_dir}/interpolator.cpp
                    ${ndk_helper_dir}/tapCamera.cpp
                    ${ndk_helper_dir}/sensorManager.cpp
                    ${ndk_helper_dir}/JNIHelper.cpp
                    ${ndk_helper_dir}/perfMonitor.cpp)

set(lpgpu2_common_dir ../../../../common/cpp)
set(lpgpu2_common_SRC ${lpgpu2_common_dir}/ProfilingRenderer.cpp
                      ${lpgpu2_common_dir}/ProfilingNDK.cpp)

set(choreographer_teapot_SRCS
 ProfilingNativeActivity.cpp
 AppRenderer.cpp
 ${ndk_helper_SRCS}
 ${lpgpu2_common_SRC}
)

include_directories(
 ${ndk_helper_dir}
 ${lpgpu2_common_dir}
 ${ANDROID_NDK}/sources/android/cpufeatures
)

set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -Wall -fno-exceptions -fno-rtti")
add_library(ProfilingNativeActivity SHARED ${choreographer_teapot_SRCS})

add_library (lpgpu2shim_link SHARED IMPORTED)
set_target_properties(lpgpu2shim_link PROPERTIES
  IMPORTED_LOCATION ${CMAKE_SOURCE_DIR}/../jniLibs/${ANDROID_ABI}/libGles2Shim.so)

# add lib dependencies
#below version uses standard system graphics apis
#target_link_libraries(ProfilingNativeActivity android log EGL GLESv2 atomic app-glue )
#use shim instead of standard libraries
target_link_libraries(ProfilingNativeActivity android log lpgpu2shim_link atomic app-glue )

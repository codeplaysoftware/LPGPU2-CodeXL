# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

get_filename_component(SHADERC_SRC
                       ${ANDROID_NDK}/sources/third_party/shaderc
                       ABSOLUTE)

set(GLSLANG_SPIRV_INCLUDE_DIR ${SHADERC_SRC}/third_party/glslang)

get_filename_component(GLMINC_PREFIX
                       "${CMAKE_SOURCE_DIR}/src/main/cpp/utils"
                       ABSOLUTE)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Werror -DVK_USE_PLATFORM_ANDROID_KHR")

set(SAMPLES_DATA_DIR src/main/data)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             native-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/cpp/native-lib.cpp
             src/main/cpp/tinyxml2.cpp
             src/main/cpp/apps/App.cpp
             src/main/cpp/utils/util.cpp
             src/main/cpp/utils/util_init.cpp
             src/main/cpp/vulkan_wrapper/vulkan_wrapper.cpp
)

# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.


add_library(native-app-glue STATIC ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

add_library(shaderc-lib STATIC IMPORTED)
set_target_properties(shaderc-lib PROPERTIES IMPORTED_LOCATION
                      ${SHADERC_SRC}/libs/${ANDROID_STL}/${ANDROID_ABI}/libshaderc.a)


set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -u ANativeActivity_onCreate")

#target_include_directories(
# native-lib
#
# PRIVATE

include_directories(
 src/main/cpp/utils
 src/main/cpp/vulkan_wrapper
 src/main/assets # because of cube_data.h

 ${GLMINC_PREFIX}
 ${SHADERC_SRC}/include
 ${SHADERC_SRC}/third_party/spirv-tools/include
 ${SHADERC_SRC}/third_party/spirv-tools/include/spirv-tools

 ${ANDROID_NDK}/sources/android/native_app_glue
)

target_link_libraries( # Specifies the target library.
                       native-lib

                       shaderc-lib
                       native-app-glue
                       android

                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )

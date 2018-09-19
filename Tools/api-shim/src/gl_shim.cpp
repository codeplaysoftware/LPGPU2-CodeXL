/*
 * The MIT License
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#include <string>

std::string CONFIG_PATH("CollectionDefinition.xml");
std::string DEVICE_CONFIG_PATH("LPGPU2Device.xml");
std::string GL_LIBRARY_PATH("libGL.so");
std::string EGL_LIBRARY_PATH("/system/lib/libEGL.so");
std::string GLES2_LIBRARY_PATH("/system/lib/libGLESv2.so");
std::string GLES3_LIBRARY_PATH("/system/lib/libGLESv3.so");
std::string CL_LIBRARY_PATH("libCL.so");
std::string VULKAN_LIBRARY_PATH("/system/lib/libGLESv3.so");

//
// Include generated files here to static initialize them first
//
#include "generated_names.cpp"
#include "generated.cpp"
#include "logger_common.cpp"
#include "config.cpp"
#include "symbol_lookup.cpp"

namespace {

void InitializeFunctionPointers()
{
  gl_InitFunctionPointers();
}

}

#include "shim.cpp"


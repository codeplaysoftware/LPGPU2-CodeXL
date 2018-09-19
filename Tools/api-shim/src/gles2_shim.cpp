/*
 * The MIT License
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#include <string>

std::string CONFIG_PATH("/files/CollectionDefinition2.xml");
std::string DEVICE_CONFIG_PATH("/data/local/tmp/LPGPU2Device.xml");
std::string GL_LIBRARY_PATH("");
std::string EGL_LIBRARY_PATH("libEGL.so");
std::string GLES2_LIBRARY_PATH("libGLESv2.so");
std::string GLES3_LIBRARY_PATH("libGLESv2.so");
std::string CL_LIBRARY_PATH("libOpenCL.so");
std::string VULKAN_LIBRARY_PATH("libEGL.so");

//
// Include generated files here to static initialize them first
//
#include "generated_names.cpp"
#include "generated.cpp"

#include "logger_common.cpp"
#include "config.cpp"
#include "symbol_lookup_egl.cpp"

namespace {

void InitializeFunctionPointers()
{
  LogPrint("            EGL: %s\n", EGL_LIBRARY_PATH.c_str());
  egl_InitFunctionPointers();
  LogPrint("          GLES2: %s\n", GLES2_LIBRARY_PATH.c_str());
  gles2_InitFunctionPointers();
  LogPrint("             CL: %s\n", GLES2_LIBRARY_PATH.c_str());
  cl_InitFunctionPointers();
}

}

#include "shim.cpp"

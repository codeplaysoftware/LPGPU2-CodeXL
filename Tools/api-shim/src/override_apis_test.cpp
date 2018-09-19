/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */

#include <EGL/egl.h>

#include "export.h"
#include "logger.h"
#include "generated.h"
#include "get_time.h"

#if defined(GENERATED_VULKAN)
#  include <vulkan/vulkan.h>
#endif

extern "C" {

#if defined(GENERATED_VULKAN)
VKAPI_ATTR
#endif
EGLBoolean DLL_EXPORT time_eglSwapBuffers(Type_eglSwapBuffers command, EGLDisplay dpy, EGLSurface surface, bool params)
{
  static uint64_t last = GetTimeMicroSeconds();
  uint64_t now = GetTimeMicroSeconds();
  unsigned long dt = now - last;
  LogPrint("eglSwapBuffers: Now %llu dt %lu (us) %f Hz\n", now, dt, (1e6/dt));
  last = now;
  ShimBeginCommand(APIID_EGL, ID_eglSwapBuffers);
  EGLBoolean ret =  command(dpy, surface);
  int32_t err = static_cast<int32_t>(original_glGetError());
  #pragma pack(0)
  struct Params { ParamsHeader _header; uint32_t _api_id; uint32_t _cmd_id; int32_t _err; uint32_t _hasReturn; EGLBoolean ret; EGLDisplay dpy; EGLSurface surface; } __attribute__((packed)) p = { {0, 0, 0}, APIID_GLES2, ID_eglSwapBuffers, err, 1, ret, dpy, surface };
  #pragma pack()
  if (params)
  {
    ShimStoreParameters( p._header, sizeof(p) );
  }
  ShimEndCommand(APIID_EGL, ID_eglSwapBuffers);
  return ret;
}

EGLBoolean DLL_EXPORT printtime_eglSwapBuffers(Type_eglSwapBuffers command, EGLDisplay dpy, EGLSurface surface)
{
  return time_eglSwapBuffers(command, dpy, surface, false);
}

EGLBoolean DLL_EXPORT printtime_params_eglSwapBuffers(Type_eglSwapBuffers command, EGLDisplay dpy, EGLSurface surface)
{
  return time_eglSwapBuffers(command, dpy, surface, true);
}

}; // extern "C"

/*
 * The MIT License
 *
 * Copyright (c) 2017 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#define DEFINITION_CLASH 1
#include <EGL/egl.h>

#include "export.h"
#include "generated.h"
#include <GLES2/gl2ext.h>
#include <stdlib.h>
#include "lpgpu2_api.h"

#if DEFINITION_CLASH
// 'EGLClientPixmapHI' defined in both generated.h and EGL/eglext.h
// These are the define needed from EGL/eglext.h:
#define EGL_CONTEXT_FLAGS_KHR             0x30FC
#define EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR  0x00000001
#elif
#include <EGL/eglext.h>
#endif

GLDEBUGPROCKHR  appUserCallback = 0;
const void      *appUserParam   = 0;

// Callback function, callled from driver.
void khr_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length,
                        const GLchar *message, const void * /*userParam*/)
{
  LPGPU2_ANNOTATION annotation;
  annotation.dwType = LPGPU2_ANNOTATION_TYPE_KHR_DEBUG;
  annotation.dwString = (char *)message;

  lpgpu2_AnnotateCapture(&annotation);

  if (appUserCallback)
  {
    appUserCallback(source, type, id, severity, length, message, appUserParam );
  }
}

extern "C" {

EGLContext khr_eglCreateContext(Type_eglCreateContext command, EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint * attrib_list, bool params)
{
  EGLint * debug_attrib_list;
  EGLContext context;
  int count = 0;

  LogPrint("khr_debug_eglCreateContext");
  // Modify the context into a debug version. Parse it first.
  while ( attrib_list[count] && attrib_list[count] != EGL_NONE )
  {
    count += 2;
  }

  debug_attrib_list = (EGLint *)malloc( sizeof(EGLint) * (count + 3));

  memcpy(debug_attrib_list, attrib_list, count * sizeof(EGLint));
  debug_attrib_list[count] = EGL_CONTEXT_FLAGS_KHR;
  debug_attrib_list[count + 1] = EGL_CONTEXT_OPENGL_DEBUG_BIT_KHR;
  debug_attrib_list[count + 2] = EGL_NONE;

  ShimBeginCommand(APIID_EGL, ID_eglCreateContext);
  context = command(dpy, config, share_context, debug_attrib_list);
  int32_t err = static_cast<int32_t>(original_glGetError());
  #pragma pack(0)
  struct Params { ParamsHeader _header; uint32_t _api_id; uint32_t _cmd_id; int32_t _err; uint32_t _hasReturn; EGLContext context; EGLDisplay dpy; EGLConfig config; EGLContext share_context; const EGLint * attrib_list; } __attribute__((packed)) p = { {0, 0, 0}, APIID_EGL, ID_eglCreateContext, err, 1, context, dpy, config, share_context, attrib_list };
  #pragma pack()
  if (params)
  {
    ShimStoreParameters( p._header, sizeof(p) );
  }
  ShimEndCommand(APIID_EGL, ID_eglCreateContext);

  free(debug_attrib_list);

  return context;
}

DLL_EXPORT EGLContext khr_debug_eglCreateContext(Type_eglCreateContext command, EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint * attrib_list)
{
  return khr_eglCreateContext(command, dpy, config, share_context, attrib_list, false);
}

DLL_EXPORT EGLContext khr_debug_params_eglCreateContext(Type_eglCreateContext command, EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint * attrib_list)
{
  return khr_eglCreateContext(command, dpy, config, share_context, attrib_list, true);
}


EGLBoolean  khr_eglMakeCurrent(Type_eglMakeCurrent command, EGLDisplay  dpy, EGLSurface  draw, EGLSurface  read, EGLContext  ctx, bool params)
{
  LogPrint("khr_debug_eglMakeCurrent");
  ShimBeginCommand(APIID_EGL, ID_eglMakeCurrent);
  EGLBoolean ret = command(dpy, draw, read, ctx);
  original_glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_KHR);
  original_glDebugMessageCallbackKHR(khr_debug_callback, NULL);
  int32_t err = static_cast<int32_t>(original_glGetError());
  #pragma pack(0)
  struct Params { ParamsHeader _header; uint32_t _api_id; uint32_t _cmd_id; int32_t _err; uint32_t _hasReturn; EGLBoolean ret; EGLDisplay  dpy; EGLSurface  draw; EGLSurface  read; EGLContext  ctx; } __attribute__((packed)) p = { {0, 0, 0}, APIID_EGL, ID_eglMakeCurrent, err, 1, ret, dpy, draw, read, ctx };
  #pragma pack()
  if (params)
  {
    ShimStoreParameters( p._header, sizeof(p) );
  }
  ShimEndCommand(APIID_EGL, ID_eglMakeCurrent);

  return ret;
}

DLL_EXPORT EGLBoolean  khr_debug_eglMakeCurrent(Type_eglMakeCurrent command, EGLDisplay  dpy, EGLSurface  draw, EGLSurface  read, EGLContext  ctx)
{
  return khr_eglMakeCurrent(command, dpy, draw, read, ctx, false);
}

DLL_EXPORT EGLBoolean  khr_debug_params_eglMakeCurrent(Type_eglMakeCurrent command, EGLDisplay  dpy, EGLSurface  draw, EGLSurface  read, EGLContext  ctx)
{
  return khr_eglMakeCurrent(command, dpy, draw, read, ctx, true);
}

 void khr_glDebugMessageCallbackKHR(Type_glDebugMessageCallbackKHR /*command*/, GLDEBUGPROCKHR  callback, const void * userParam, bool params)
{
  // Store the app callback parameters to call later.
  ShimBeginCommand(APIID_GLES2, ID_glDebugMessageCallbackKHR);
  appUserCallback = callback;
  appUserParam = userParam;

  int32_t err = static_cast<int32_t>(original_glGetError());
  #pragma pack(0)
  struct Params { ParamsHeader _header; uint32_t _api_id; uint32_t _cmd_id; int32_t _err; uint32_t _hasReturn; GLDEBUGPROCKHR  callback; const void * userParam; } __attribute__((packed)) p = { {0, 0, 0}, APIID_GLES2, ID_glDebugMessageCallbackKHR, err, 0, callback, userParam };
  #pragma pack()
  if (params)
  {
    ShimStoreParameters( p._header, sizeof(p) );
  }
  ShimEndCommand(APIID_GLES2, ID_glDebugMessageCallbackKHR);
}

DLL_EXPORT void khr_debug_glDebugMessageCallbackKHR(Type_glDebugMessageCallbackKHR command, GLDEBUGPROCKHR  callback, const void * userParam)
{
  khr_glDebugMessageCallbackKHR(command, callback, userParam, false);
}

DLL_EXPORT void khr_debug_params_glDebugMessageCallbackKHR(Type_glDebugMessageCallbackKHR command, GLDEBUGPROCKHR  callback, const void * userParam)
{
  khr_glDebugMessageCallbackKHR(command, callback, userParam, true);
}

}; // extern "C"

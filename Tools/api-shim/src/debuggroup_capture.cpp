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
#include "lpgpu2_api.h"
#include <map>

#define PUSHDEBUG "glPushDebugGroup: "
#define POPDEBUG "glPopDebugGroup"

extern "C" {

void debuggroup_capture_glPushDebugGroup(Type_glPushDebugGroup command,GLenum  source,
    GLuint  id, GLsizei  length, const GLchar * message, bool params)
{
  ShimBeginCommand(APIID_GLES2, ID_glPushDebugGroup);
  command(source,id,length,message);
  int32_t err = static_cast<int32_t>(original_glGetError());
  #pragma pack(0)
  struct Params { ParamsHeader _header; uint32_t _api_id; uint32_t _cmd_id; int32_t _err; uint32_t _hasReturn;
    GLenum  source;GLuint  id;GLsizei  length;const GLchar * message; } __attribute__((packed))
      p = { {0, 0, 0}, APIID_GLES2, ID_glPushDebugGroup, err, 0, source,id,length,message };

  #pragma pack()
  if (params)
  {
    ShimStoreParameters( p._header, sizeof(p) );
  }
  ShimEndCommand(APIID_GLES2, ID_glPushDebugGroup);

  int str_len = length;
  if (str_len < 0)
  {
    str_len = strlen(message);
  }

  str_len += sizeof(PUSHDEBUG);
  char * msg = (char *)malloc(str_len + 1);
  sprintf(msg, "%s%s", PUSHDEBUG, message);
  LPGPU2_ANNOTATION annotation;
  annotation.dwType = LPGPU2_ANNOTATION_TYPE_KHR_DEBUG;
  annotation.dwString = (char *)msg;
  lpgpu2_AnnotateCapture(&annotation);
  free(msg);
}

void DLL_EXPORT debuggroup_glPushDebugGroup(Type_glPushDebugGroup command,GLenum  source,
    GLuint  id, GLsizei  length, const GLchar * message)
{
  debuggroup_capture_glPushDebugGroup(command, source, id, length, message, false);
}

void DLL_EXPORT debuggroup_params_glPushDebugGroup(Type_glPushDebugGroup command,GLenum  source,
    GLuint  id, GLsizei  length, const GLchar * message)
{
  debuggroup_capture_glPushDebugGroup(command, source, id, length, message, true);
}


void debuggroup_capture_glPopDebugGroup(Type_glPopDebugGroup command, bool params)
{
  ShimBeginCommand(APIID_GLES2, ID_glPopDebugGroup);
  command();
  int32_t err = static_cast<int32_t>(original_glGetError());
  #pragma pack(0)
  struct Params { ParamsHeader _header; uint32_t _api_id; uint32_t _cmd_id; int32_t _err; uint32_t _hasReturn;  } __attribute__((packed)) p = { {0, 0, 0}, APIID_GLES2, ID_glPopDebugGroup, err, 0,  };
  #pragma pack()
  if (params)
  {
    ShimStoreParameters( p._header, sizeof(p) );
  }
  ShimEndCommand(APIID_GLES2, ID_glPopDebugGroup);

  int str_len = sizeof(POPDEBUG);
  char * msg = (char *)malloc(str_len + 1);
  sprintf(msg, "%s", POPDEBUG);
  LPGPU2_ANNOTATION annotation;
  annotation.dwType = LPGPU2_ANNOTATION_TYPE_KHR_DEBUG;
  annotation.dwString = (char *)msg;
  lpgpu2_AnnotateCapture(&annotation);
  free(msg);
}

void DLL_EXPORT debuggroup_glPopDebugGroup(Type_glPopDebugGroup command)
{
  debuggroup_capture_glPopDebugGroup(command, false);
}

void DLL_EXPORT debuggroup_params_glPopDebugGroup(Type_glPopDebugGroup command)
{
  debuggroup_capture_glPopDebugGroup(command, true);
}
}; // extern "C"

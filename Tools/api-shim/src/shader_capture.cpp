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

extern int (*ShimStoreShader)(uint, uint64_t, uint, int, char *);
extern int (*ShimStoreShaderMetaData)(uint, uint64_t, uint ,uint ,int ,char *);

extern "C" {

std::map <uint, uint> shader_list;

GLuint shader_capture_glCreateShader(Type_glCreateShader command, GLenum shaderType, bool params)
{
  ShimBeginCommand(APIID_GLES2, ID_glCreateShader);
  GLuint ret = command( shaderType );
  int32_t err = static_cast<int32_t>(original_glGetError());
#pragma pack(0)
  struct Params { ParamsHeader _header; uint32_t _api_id; uint32_t _cmd_id; int32_t _err; uint32_t _hasReturn;  GLuint  _ret; GLenum shaderType; } __attribute__((packed)) p = { {0, 0, 0}, APIID_GLES2, ID_glCreateShader, err, 1, ret, shaderType };
#pragma pack()
  if (params)
  {
    ShimStoreParameters( p._header, sizeof(p) );
  }

  // Convert GL Shader defines into LPGPU2
  uint shader_type;
  switch(shaderType)
  {
    default:
      shader_type = LPGPU2_SHADER_TYPE_UNKNOWN;
      break;

    case GL_VERTEX_SHADER:
      shader_type = LPGPU2_SHADER_TYPE_VERTEX;
      break;

    case GL_FRAGMENT_SHADER:
      shader_type = LPGPU2_SHADER_TYPE_FRAGMENT;
      break;

  }
  shader_list[ret] = shader_type;
  ShimEndCommand(APIID_GLES2, ID_glCreateShader);
  return ret;
}


GLuint DLL_EXPORT shader_glCreateShader(Type_glCreateShader command, GLenum shaderType)
{
  return shader_capture_glCreateShader(command, shaderType, false);
}

GLuint DLL_EXPORT shader_params_glCreateShader(Type_glCreateShader command, GLenum shaderType)
{
  return shader_capture_glCreateShader(command, shaderType, true);
}


void shader_capture_glShaderSource(Type_glShaderSource command, GLuint shader,
    GLsizei count,
    const GLchar **string,
    const GLint *length,
    bool params)
{
  uint type;
  uint offset = 0;
  uint size = 0;
  ShimBeginCommand(APIID_GLES2, ID_glShaderSource);
  if(shader_list.find(shader) == shader_list.end())
  {
    // Unknown
    type = LPGPU2_SHADER_TYPE_UNKNOWN;
  }
  else
  {
     type =shader_list[shader];
  }

  for (int i = 0; i < count; i++)
  {
    if (length)
    {
      size += length[i];
    }
    else
    {
      size += strlen(string[i]);
    }
  }
  char *buffer = (char *)malloc(size);
  for (int i = 0; i < count; i++)
  {
    if (length)
    {
      memcpy(buffer + offset, string[i], length[i]);
      offset += length[i];
    }
    else
    {
      memcpy(buffer + offset, string[i], strlen(string[i]));
      offset += strlen(string[i]);
    }
  }
  ShimStoreShader(APIID_GLES2, (uint64_t)shader, type, size, buffer);

  command( shader, count, string, length );
  int32_t err = static_cast<int32_t>(original_glGetError());
  #pragma pack(0)
  struct Params { ParamsHeader _header; uint32_t _api_id; uint32_t _cmd_id; int32_t _err; uint32_t _hasReturn; GLuint shader; GLsizei count; const GLchar **string; const GLint *length; } __attribute__((packed)) p = { {0, 0, 0}, APIID_GLES2, ID_glShaderSource, err, 0, shader, count, string, length };
  #pragma pack()
  if (params)
  {
    ShimStoreParameters( p._header, sizeof(p) );
  }
  ShimEndCommand(APIID_GLES2, ID_glShaderSource);
  return;
}

void DLL_EXPORT shader_glShaderSource(Type_glShaderSource command, GLuint shader,
    GLsizei count,
    const GLchar **string,
    const GLint *length)
{
  shader_capture_glShaderSource(command, shader, count, string, length, false);
}

void DLL_EXPORT shader_params_glShaderSource(Type_glShaderSource command, GLuint shader,
    GLsizei count,
    const GLchar **string,
    const GLint *length)
{
  shader_capture_glShaderSource(command, shader, count, string, length, true);
}


cl_program  shader_capture_clCreateProgramWithSource(Type_clCreateProgramWithSource command, cl_context  context,
    cl_uint  count,
    const char** strings,
    const size_t* lengths,
    cl_int* errcode_ret,
    bool params)
{
  uint size = 0;
  uint offset = 0;
  uint type = 0;
  uint64_t shader_id = 0;
  cl_program ret;
  ShimBeginCommand(APIID_CL, ID_clCreateProgramWithSource);
  ret = command(context, count, strings, lengths, errcode_ret);

  for (uint i = 0; i < count; i++)
  {
    if (lengths)
    {
      size += lengths[i];
    }
    else
    {
      size += strlen(strings[i]);
    }
  }
  char *buffer = (char *)malloc(size);
  for (uint i = 0; i < count; i++)
  {
    if (lengths)
    {
      memcpy(buffer + offset, strings[i], lengths[i]);
      offset += lengths[i];
    }
    else
    {
      memcpy(buffer + offset, strings[i], strlen(strings[i]));
      offset += strlen(strings[i]);
    }
  }

  ShimStoreShader(APIID_CL, shader_id, type, size, buffer);
  free(buffer);

  int32_t err = 0;
  #pragma pack(0)
  struct Params { ParamsHeader _header; uint32_t _api_id; uint32_t _cmd_id; int32_t _err; uint32_t _hasReturn;  cl_program _ret; cl_context  context;cl_uint  count;const char** strings;const size_t* lengths;cl_int* errcode_ret; } __attribute__((packed)) p = { {0, 0, 0}, APIID_CL, ID_clCreateProgramWithSource, err, 1, ret, context,count,strings,lengths,errcode_ret };
  #pragma pack()
  if (params)
  {
    ShimStoreParameters( p._header, sizeof(p) );
  }
  ShimEndCommand(APIID_CL, ID_clCreateProgramWithSource);

  return ret;
}

cl_program DLL_EXPORT shader_clCreateProgramWithSource(Type_clCreateProgramWithSource command, cl_context  context,
    cl_uint  count,
    const char** strings,
    const size_t* lengths,
    cl_int* errcode_ret)
{
  return shader_capture_clCreateProgramWithSource(command, context, count, strings, lengths, errcode_ret, false);
}

cl_program DLL_EXPORT shader_params_clCreateProgramWithSource(Type_clCreateProgramWithSource command, cl_context  context,
    cl_uint  count,
    const char** strings,
    const size_t* lengths,
    cl_int* errcode_ret)
{
  return shader_capture_clCreateProgramWithSource(command, context, count, strings, lengths, errcode_ret, true);
}

#if defined(GENERATED_VULKAN)
VkResult shader_capture_vkCreateShaderModule(Type_vkCreateShaderModule command,
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkShaderModule*                             pShaderModule,
    bool                                        params)
{
  uint type = LPGPU2_SHADER_TYPE_UNKNOWN;
  uint64_t shader_id = 0;
  ShimBeginCommand(APIID_VULKAN, ID_vkCreateShaderModule);

  VkResult ret = command( device, pCreateInfo, pAllocator, pShaderModule );
  shader_id = (uint64_t)*pShaderModule;

  ShimStoreShader(APIID_VULKAN, shader_id, type, 0, NULL);
  ShimStoreShaderMetaData(APIID_VULKAN, shader_id, LPGPU2_SHADER_TYPE_UNKNOWN, LPGPU2_SHADER_META_TYPE_BINARY, pCreateInfo->codeSize, (char *)pCreateInfo->pCode);

  int32_t err = 0;
  #pragma pack(0)
  struct Params { ParamsHeader _header; uint32_t _api_id; uint32_t _cmd_id; int32_t _err; uint32_t _hasReturn; VkResult  _ret; VkDevice  device;const VkShaderModuleCreateInfo*  pCreateInfo;const VkAllocationCallbacks*  pAllocator;VkShaderModule*  pShaderModule;} __attribute__((packed)) p = { {0, 0, 0}, APIID_VULKAN, ID_vkCreateShaderModule, err, 1, ret, device,pCreateInfo,pAllocator,pShaderModule  };
#pragma pack()
  if (params)
  {
    ShimStoreParameters( p._header, sizeof(p) );
  }
  ShimEndCommand(APIID_VULKAN, ID_vkCreateShaderModule);
  return ret;
}

VkResult DLL_EXPORT shader_vkCreateShaderModule(Type_vkCreateShaderModule command,
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkShaderModule*                             pShaderModule)
{
  return shader_capture_vkCreateShaderModule(command, device, pCreateInfo, pAllocator, pShaderModule, false);
}

VkResult DLL_EXPORT shader_params_vkCreateShaderModule(Type_vkCreateShaderModule command,
    VkDevice                                    device,
    const VkShaderModuleCreateInfo*             pCreateInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkShaderModule*                             pShaderModule)
{
  return shader_capture_vkCreateShaderModule(command, device, pCreateInfo, pAllocator, pShaderModule, true);
}
#endif
}; // extern "C"

int ShimStoreShaderNoOp(uint, uint64_t, uint, int , char *)
{
  static bool firsttime = true;
  if (firsttime == true)
  {
    Log(LogError, "Error: Calling ShimStoreShaderNoOp(). \n");
    firsttime = false;
  }
  return 0;
}

int ShimStoreShaderMetaDataNoOp(uint, uint64_t, uint ,uint ,int ,char *)
{
  static bool firsttime = true;

  if (firsttime == true)
  {
    Log(LogError, "Error: Calling ShimStoreShaderMetaDataNoOp(). \n");
    firsttime = false;
  }
  return 0;
}

int (*ShimStoreShader)(uint, uint64_t, uint, int, char *) = ShimStoreShaderNoOp;
int (*ShimStoreShaderMetaData)(uint, uint64_t, uint ,uint ,int ,char *) = ShimStoreShaderMetaDataNoOp;

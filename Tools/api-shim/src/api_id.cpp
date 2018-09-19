/*
 * The MIT License
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#include "api_id.h"

#include <map>

#include "string_bitfield.h"

int ApiIdFromString(const std::string& nameList)
{
  int ret = APIID_ALL;

  typedef std::map<std::string, int> Lut;
  static Lut table;

  if(!table.size())
  {
    table["gles2"]   = (int)APIID_GLES2;
    table["gles3"]   = (int)APIID_GLES3;
    table["gl"]      = (int)APIID_GL;
    table["egl"]     = (int)APIID_EGL;
    table["cl"]      = (int)APIID_CL;
    table["vulkan"]  = (int)APIID_VULKAN;
  }

  if(nameList.size())
  {
    ret = GetStringBitField(nameList.c_str(), table);
  }

  return ret;
}

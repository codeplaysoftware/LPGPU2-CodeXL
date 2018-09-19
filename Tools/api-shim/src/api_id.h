/*
 * The MIT License
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#ifndef API_ID_H
#define API_ID_H

#include <string>

enum APIID {
  APIID_GLES2  = 0x0100,
  APIID_GLES3  = 0x0200,
  APIID_EGL    = 0x0400,
  APIID_GL     = 0x0800,
  APIID_CL     = 0x1000,
  APIID_VULKAN = 0x2000,

  APIID_ALL    = 0x3F00
};

int ApiIdFromString(const std::string& nameList);


#endif // header

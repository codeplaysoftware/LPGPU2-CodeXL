/*
 * The MIT License
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#include "filesystem.h"
#include "logger.h"

std::string PathJoin(const std::string& a, const std::string&b)
{
#if defined _WIN32
  static std::string fspathSeparator("\\");
#else
  static const std::string fspathSeparator("/");
#endif

  std::string ret;

  if(size_t len = a.size())
  {
    if( (a[len-1] == '/') ||
        (a[len-1] == '\\') )
    {
      ret = a + b;
    }
    else
    {
      ret = a;
      ret += fspathSeparator;
      ret += b;
    }
  }
  else
  {
    ret = b;
  }

  return ret;
}

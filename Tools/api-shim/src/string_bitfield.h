/*
 * The MIT License
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#ifndef STRING_BITFIELD_HPP
#define STRING_BITFIELD_HPP

#include <map>
#include <string>

inline const char* findSpaceOrNull(const char*p)
{
  while( ! ((*p == 0) || (*p == ' ')) )
  {
    p++;
  }
  return p;
}

template <typename T>
T GetStringBitField(const char* spaceSeparated,
                    std::map<std::string, T> nameMap)
{
  T ret = (T)0;

  if(spaceSeparated)
  {
    const char* begin = spaceSeparated;

    if(begin)
    {
      const char* end = findSpaceOrNull(begin);

      std::string s(begin, end);

      while(s.size())
      {
        typename std::map<std::string, T>::iterator i = nameMap.find(s);

        if(i != nameMap.end())
        {
          ret = (T)( ((int)ret | (int)(i->second)) );
        }

        if(*end)
        {
          begin = end + 1;
          end = findSpaceOrNull(begin);
          s = std::string(begin, end);
        }
        else
        {
          s.clear();
        }
      }
    }
  } // spaceSeparated

  return ret;
} // GetBitField



#endif // header

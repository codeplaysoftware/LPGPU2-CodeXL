/*
 * The MIT License
 *
 * Copyright (c) 2016-17 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

//--------------------------------------------------------------------------------
// ProfilingNDK.h
//
// Contains more even more helpful functions than ndk_helper :-)
//--------------------------------------------------------------------------------

#ifndef _PROFILINGNDK_H
#define _PROFILINGNDK_H


#include "JNIHelper.h"


namespace LPGPU2 {

  /*
   * Load and create image and return it as a unsigned char* allocated with new[]
   *
   * arguments:
   * in: file_name, file name to read, PNG&JPG is supported
   * outWidth(Optional) pointer to retrieve original bitmap width
   * outHeight(Optional) pointer to retrieve original bitmap height
   * return:
   * unsigned char* array of RGBA values
   */
  unsigned char* LoadImage(const char* file, int32_t& xsiz, int32_t& ysiz);



  /*
   * Convenience function for when the dimensions are not required
   */
  inline unsigned char* LoadImage(const char* file) {int32_t xsiz,ysiz; return LoadImage(file,xsiz,ysiz);}



  uint32_t LoadTexture(const char* file, int32_t& xsiz, int32_t& ysiz);

  inline uint32_t LoadTexture(const char* file) {int32_t xsiz,ysiz; return LoadTexture(file,xsiz,ysiz);}

} // namespace LPGPU2


#endif

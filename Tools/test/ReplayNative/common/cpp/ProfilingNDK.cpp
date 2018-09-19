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

#include "ProfilingNDK.h"

namespace LPGPU2 {

  unsigned char* LoadImage(const char* file, int32_t& xsiz, int32_t& ysiz) {
   jobject obj = ndk_helper::JNIHelper::GetInstance()->LoadImage(file,&xsiz,&ysiz);

// xsiz = 100; ysiz = 100;

   unsigned char* dat = new unsigned char[4*xsiz*ysiz];

   for(int j=0,k=0;j<ysiz;j++     )
   for(int i=0    ;i<xsiz;i++,k+=4) {
    dat[k+0]=255;
    dat[k+1]=(int)(i*255.0/xsiz);
    dat[k+2]=(int)(j*255.0/ysiz);
    dat[k+3]=255;

    dat[k+1]=(int)(i*255.0/xsiz);
    dat[k+2]=(int)(j*255.0/ysiz);
   }

// ndk_helper::JNIHelper::GetInstance()->AttachCurrentThread()->DeleteLocalRef(obj);

   return dat;
  }



  uint32_t LoadTexture(const char *file, int32_t& xsiz, int32_t& ysiz) {
   return ndk_helper::JNIHelper::GetInstance()->LoadTexture(file,&xsiz,&ysiz);
  }

} // namespace LPGPU2

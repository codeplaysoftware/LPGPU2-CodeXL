/*
 * The MIT License
 *
 * Copyright (c) 2017 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#ifndef FRAME_TERMINATOR_H
#define FRAME_TERMINATOR_H

enum FRAME_TERMINATOR {
  FT_GL_CLEAR            = 1 << 0,
  FT_GL_FLUSH            = 1 << 1,
  FT_GL_FINISH           = 1 << 2,
  FT_GL_ALL              = FT_GL_CLEAR | FT_GL_FLUSH | FT_GL_FINISH,

  FT_EGL_SWAPBUFFERS     = 1 << 6,
  FT_EGL_ALL             = FT_EGL_SWAPBUFFERS,

  FT_CL_FLUSH            = 1 << 10,
  FT_CL_FINISH           = 1 << 11,
  FT_CL_WAITFOREVENTS    = 1 << 12,
  FT_CL_ALL              = FT_CL_FLUSH | FT_CL_FINISH | FT_CL_WAITFOREVENTS,

  FT_VK_QUEUEPRESENTKHR  = 1 << 15,
  FT_VK_ALL              = FT_VK_QUEUEPRESENTKHR,

  FT_ALL                 = 0xFFFFFFFF
};


#endif // header

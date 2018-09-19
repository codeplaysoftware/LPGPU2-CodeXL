/*
 * The MIT License
 *
 * Copyright (c) 2017 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */

#ifndef GPU_TIMING_H
#define GPU_TIMING_H

#include "generated.h"

#include <stdint.h>
#include <cassert>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <time.h>
#include <sys/ioctl.h>
#include <linux/fb.h>
#include <GLES2/gl2ext.h>

#include "get_time.h"
#include "packet_types.h"
#include "api_id.h"

// Strings for use CollectionDefinition
#define GPU_TIMING_DISJOINT_FRAME_STRING "DisjointFrame"  // maps to GPU_TIMING_MODE_FRAME_DISJOINT
#define GPU_TIMING_DRAW_CALL_STRING "DrawCall"             // maps to GPU_TIMING_MODE_DRAW

// Return values
#define  GPUTIMER_SUCCESS       0
#define  GPUTIMER_INITIALISED   1
#define  GPUTIMER_UNINITIALISED 2
#define  GPUTIMER_ERROR         3
#define  GPUTIMER_UNSUPPORTED   4
#define  GPUTIMER_TIMEOUT       5
#define  GPUTIMER_UNKNOWN       6


#define NUM_GPU_TIMER_QUERIES   15

#define FRAME_TIMER_DELAY 4    // The number of frame timers to leave running when collecting results.
#define WAIT_THRESHOLD    1000 // The time in microseconds to wait for the results.

// TODO - The following are defined in GLES2/gl2ext from android-19
#define GL_QUERY_RESULT                   0x8866
#define GL_QUERY_RESULT_AVAILABLE         0x8867


enum GPU_TIMING_MODE {
  GPU_TIMING_MODE_NONE            = 0x0000,
  GPU_TIMING_MODE_FRAME_MASK      = 0x0100,
  GPU_TIMING_MODE_FRAME_DISJOINT  = 0x0101,
  GPU_TIMING_MODE_DRAW_MASK       = 0x0200,
  GPU_TIMING_MODE_DRAW            = 0x0201
};

class GpuTiming
{
  public:
    GpuTiming();
    ~GpuTiming();

    int InitialiseGpuTimer(uint64_t Mode);
    int FrameTimer(int64_t frame);
    int DisjointFrameTimer(int64_t frame);

  private:
    uint64_t gpuTimingMode;
    int64_t  gpuTimerFrame[NUM_GPU_TIMER_QUERIES];
    GLuint   gpuTimerQueries[NUM_GPU_TIMER_QUERIES];
    int      queryIndex;
    int      newFrameTimer;
    int      lastQuery;
    bool     gpuTimerInitialised;

    struct GpuTimerRecord {
      uint64_t timer_type;
      uint64_t frame_no;
      uint64_t draw_no;
      uint64_t time;
    };
};

#endif // header

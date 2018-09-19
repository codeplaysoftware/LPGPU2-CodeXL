/*
 * The MIT License
 *
 * Copyright (c) 2017 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */

#include "gpu_timing.h"

extern int (*ShimStoreGPUTimer)(uint8_t *, size_t);

  GpuTiming::GpuTiming()
    :  gpuTimingMode(0),
       queryIndex(0),
       newFrameTimer(1),
       lastQuery(NUM_GPU_TIMER_QUERIES - 1),
       gpuTimerInitialised(false)
  {
  }

  GpuTiming::~GpuTiming()
  {
  }

  int GpuTiming::InitialiseGpuTimer(uint64_t Mode)//,  NumberedBufferWriter<uint8_t> *writer)
  {
    GLint disjointOccurred = 0;
    GLenum error;
    if (gpuTimerInitialised)
    {
      return GPUTIMER_INITIALISED;
    }
    gpuTimingMode = Mode;

    if (gpuTimingMode == GPU_TIMING_MODE_FRAME_DISJOINT)
    {
      /* Clear disjoint error */
      original_glGetIntegerv(GL_GPU_DISJOINT_EXT, &disjointOccurred);
      /* Create a query object. */
      original_glGenQueries(NUM_GPU_TIMER_QUERIES, (GLuint *)gpuTimerQueries);
      error = original_glGetError();
      if (error)
      {
        Log(LogError, "Error: glGenQueries failed. glGetError returned %d \n", error);
        return GPUTIMER_ERROR;
      }
      else
      {
        gpuTimerInitialised = true;
      }

      for (int i = 0 ; i < NUM_GPU_TIMER_QUERIES; i++)
      {
        gpuTimerFrame[i] = 0;
      }
    }
    else if (gpuTimingMode == GPU_TIMING_MODE_DRAW)
    {
      Log(LogError, "Error: GPU_TIMING_MODE_DRAW is currently unsupported \n");
      return GPUTIMER_UNSUPPORTED;
    }
    else
    {
      Log(LogError, "Error: Unknown GPU Timing Mode %d \n", gpuTimingMode);
      return GPUTIMER_UNKNOWN;
    }
    return GPUTIMER_SUCCESS;
  }

  int GpuTiming::FrameTimer(int64_t frame)
  {
    if (gpuTimingMode == GPU_TIMING_MODE_FRAME_DISJOINT)
    {
      return DisjointFrameTimer(frame);
    }
    return GPUTIMER_UNSUPPORTED;
  }

  int GpuTiming::DisjointFrameTimer(int64_t frame)
  {
    GLuint64 available = 0;
    GLuint64 timeElapsed = 0;
    GLint disjointOccurred = 0;
    uint64_t startTime, totalTime = 0;

    struct GpuTimerRecord timerRecord;
    int ret = GPUTIMER_SUCCESS;

    gpuTimerFrame[queryIndex] = frame;

    if (!gpuTimerInitialised)
      return GPUTIMER_UNINITIALISED;

    if (!newFrameTimer)
    {
      // End previous Timer
      original_glEndQuery(GL_TIME_ELAPSED_EXT);
    }
    else
    {
      newFrameTimer = 0;
    }

    if (queryIndex == lastQuery) // All the timers are in use, get some results
    {
      int waitIndex = lastQuery - FRAME_TIMER_DELAY;
      if (waitIndex < 0)
      {
        waitIndex += NUM_GPU_TIMER_QUERIES;
      }

      // "waitIndex" is the last timer to wait for just now.
      // There will still be "FRAME_TIMER_DELAY" timers potentially still running after the query.
      startTime = GetTimeMicroSeconds();
      while (!available && (totalTime < WAIT_THRESHOLD))
      {
        original_glGetQueryObjectui64vEXT(gpuTimerQueries[waitIndex] , GL_QUERY_RESULT_AVAILABLE, &available);
        if (!available)
        {
          usleep(WAIT_THRESHOLD / 5); //microseconds
          totalTime = GetTimeMicroSeconds() - startTime;
        }
      }

      if (totalTime >= WAIT_THRESHOLD)
      {
        Log(LogError, "Error: Timed out waiting for GPU Timer results. Results will be unreliable. %d \n");
        ret = GPUTIMER_SUCCESS;
      }

      original_glGetIntegerv(GL_GPU_DISJOINT_EXT, &disjointOccurred);

      // If a disjoint operation occurred, all timer queries in between
      // the last two disjoint checks that were filled are invalid.
      if (!disjointOccurred)
      {
        int index = lastQuery + 1;

        for (int i = 0; i < NUM_GPU_TIMER_QUERIES - FRAME_TIMER_DELAY; i++)
        {
          if (index >= NUM_GPU_TIMER_QUERIES)
          {
            index = 0;
          }
          original_glGetQueryObjectui64vEXT(gpuTimerQueries[index], GL_QUERY_RESULT, &timeElapsed);

          Log(LogVerbose, "GpuTiming: Frame %lld, Gpu Time = %lld(ms)\n", gpuTimerFrame[index], timeElapsed / 1000);
          timerRecord.timer_type = GPU_TIMING_MODE_FRAME_DISJOINT;
          timerRecord.frame_no = gpuTimerFrame[index];
          timerRecord.draw_no = 0;
          timerRecord.time = timeElapsed;
          index++;
          // Write the results to the CFS
          ShimStoreGPUTimer((uint8_t *)&timerRecord, sizeof(timerRecord));
        }
        lastQuery = index - 1;
      }
    }

    if (queryIndex >= NUM_GPU_TIMER_QUERIES - 1)
    {
      queryIndex = 0;
    }
    else
    {
      queryIndex++;
    }

    // Start next query
    original_glBeginQuery(GL_TIME_ELAPSED_EXT, gpuTimerQueries[queryIndex]);

    return ret;
  }

  int ShimStoreGPUTimerNoOp(uint8_t *, size_t)
  {
    Log(LogError, "Error: Calling ShimStoreGPUTimerNoOp(). \n");
    return 0;
  }

  int (*ShimStoreGPUTimer)(uint8_t *, size_t) = ShimStoreGPUTimerNoOp;

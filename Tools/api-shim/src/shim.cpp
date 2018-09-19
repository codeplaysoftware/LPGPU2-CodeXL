/*
 * The MIT License
 *
 * Copyright (c) 2016 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */
#include <stdint.h>
#include <cassert>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <linux/fb.h>
#include <unwind.h>

#include "tinyxml2.h"
#include "get_time.h"
#include "lock.h"
#include "export.h"
#include "generated.h"
#include "build_date.cpp"
#include "api_id.h"
#include "lpgpu2_api.h"
#include "frame_terminators.h"
#include "gpu_timing.h"

extern int (*ShimStoreShader)(uint, uint64_t, uint, int, char *);
extern int (*ShimStoreShaderMetaData)(uint, uint64_t, uint ,uint ,int ,char *);
extern int (*ShimStoreGPUTimer)(uint8_t *, size_t);

#define MIN_MEGACHUNK_SIZE (64 * 1024) // 64k
#define SHIM_VERSION 2.23f  // Increment by 0.01

#define CALLSTACK_MAX       30

typedef uint32_t (*SHIM_SEND_DATA_CALLBACK)(uint8_t *r, size_t N, uint16_t chunkID, void *userData);

using namespace tinyxml2;

namespace {

  uint64_t (* const GetTime)() = GetTimeNanoSeconds;

  class Init {
  public:
    Init();
    ~Init();
    static Init* Instance() { return _instance; }
    void SetupDevicePaths(const Config& cfg);
    void NewFrame(void);
    void BeginCommand(int api, int id);
    void EndCommand(int api, int id);
    int EnableExplicitCapture(LPGPU2_CAPTURE *pCapture);
    int DisableExplicitCapture(LPGPU2_CAPTURE *pCapture);
    int ExplicitTerminate(void);
    bool SendData(uint8_t *buffer, size_t dataSize, uint32_t dataType);
    int ExplicitAnnotate(LPGPU2_ANNOTATION *pAnn);
    int TrackResource(LPGPU2_RESOURCE_TYPE *pType, LPGPU2_RESOURCE_FLAGS *pFlags, uint64_t resource);
    bool ShimStoreParameters(ParamsHeader& header, size_t noBytes);
    int StoreShader(uint api, uint64_t shader_id, uint type, int length, char *shader);
    int StoreShaderMetaData(uint api, uint64_t shader_id, uint shader_type, uint data_type, int length, char *shader);
    int GetVersion(float *);
    int StoreAffinity(uint id, uint cpu);
    int StoreUserCounter(uint id, uint32_t value);
    int SetSendDataCallback(SHIM_SEND_DATA_CALLBACK pfn, void *usrData);
    int StartCollecting(void);
    int StopCollecting(void);
    int GenerateTargetCharacteristics(XMLDocument& doc);

    std::string DeviceInfoXml(const std::string& additional_xml);

  private:
    static Init* _instance;
    Config deviceConfig;
    Config config;
    uint8_t *megaBuffer;
    int megaBufferOffset;
    int megaBufferMaxSize;
    GpuTiming gpuTiming;
    uint64_t timelineBegin;
    int64_t frameNumber;
    int64_t drawNumber;
    uint64_t gpuTimingMode;
    bool traceThisProcess;
    int traceAPI;
    int rangeStartFrame;
    int rangeStopFrame;
    int rangeStartDraw;
    int rangeStopDraw;
    bool explicitControl;
    bool explicitCapture;
    bool explicitTerminator;
    bool dumpCallstack;
    bool callstackDumpAll;
    bool vulkanAllowCounters;

    bool seenBegin = false;
    bool seenStart = false;

    uint64_t frameTerminators;
    uint64_t parameterOffset;
    uint64_t parameterLength;

    pthread_t stopThreadID;

    struct Stats {
      int64_t writeThreshold;
      uint64_t startTime;
      uint64_t bytesWritten;
      uint64_t sumKbPerSecond;
      uint64_t averageLimit;
      uint64_t averageCount;
      Stats() :
        writeThreshold(-1),
        startTime(GetTime()),
        bytesWritten(0), sumKbPerSecond(0), averageLimit(10), averageCount(1) {}
    };

    Stats stats;

    Lock lock;
    struct fb_var_screeninfo v_si;
    GLenum type;
    GLenum format;
    uint32_t fb_size;
    bool inRange;
    uint8_t *frameBuffer;
    SHIM_SEND_DATA_CALLBACK pfnSendDataCallback = NULL;
    void *userData;

    struct TimelineRecord {
      uint64_t id;
      uint64_t startTime;
      uint64_t endTime;
      uint64_t frame;
      uint64_t draw;
      uint64_t apiID;
      uint64_t poffset;
      uint64_t plength;
    };

    struct FB_Record {
      uint64_t frame_no;
      uint64_t draw_no;
      uint64_t x_res;
      uint64_t y_res;
      uint64_t bpp;
      uint64_t fb_size;
      uint64_t type;
      uint64_t format;
    };

    struct AnnotateRecord {
      uint64_t id ;
      uint64_t frame_no;
      uint64_t draw_no;
      uint64_t type;
      uint64_t startTime;
      uint64_t endTime;
      uint64_t strlen;
    };

    struct ShaderRecord {
      uint64_t frame_no;
      uint64_t draw_no;
      uint64_t shader_id;
      uint64_t api;
      uint64_t shader_type;
      uint64_t cpu_time;
      uint64_t strlen;
    };

    struct ShaderMetaRecord {
      uint64_t shader_id;
      uint64_t api;
      uint64_t shader_type;
      uint64_t data_type;
      uint64_t strlen;
    };

    struct TrackResourceRecord {
      uint64_t frame_no;
      uint64_t draw_no;
      uint64_t type;
      uint64_t flag;
      uint64_t resource;
      uint64_t cpu_time;
    };

    struct CallstackTrace {
      void** current;
      void** end;
    };

    struct CallstackHeader {
      uint64_t apiID;
      uint64_t id;
      uint64_t frame_no;
      uint64_t draw_no;
      uint64_t startTime;
      uint64_t records;
    };

    struct CallstackRecord {
      uint64_t dli_fbase;      // Base address at which shared object is loaded
      uint64_t dli_saddr;      // Exact address of symbol named in dli_sname
      uint64_t dli_fname_len;  // Length of Pathname of shared object that contains address
      uint64_t dli_sname_len;  // Length of Name of symbol whose definition overlaps addr
    };

    struct MegaChunkHeader {
      int magicHeader;
      int chunkID;
      int chunkFlags;
      int chunkSize;
    };

    void WriteTimeLine(uint64_t api, uint64_t id, uint64_t begin, uint64_t end);
    size_t getCallstack(void** buffer, size_t max);
    static _Unwind_Reason_Code unwindHandler(struct _Unwind_Context* context, void* arg);
    void CallstackToLog(void);
    void WriteCallstack(uint64_t api, uint64_t id, uint64_t begin);

    void CheckStats(size_t bytesWritten);

    bool IsFrameTerminator(int api, int id);
    bool ShouldTrace(int api, int id);
    bool RangeCheck(void);

    void configFrameCapture();
    void frameCapture();
  }; // class Init

  Init* Init::_instance = NULL;

  //
  // Callbacks
  //

  int ShimStartCaptureLogToFile(LPGPU2_CAPTURE *pCapture)
  {
    return Init::Instance()->EnableExplicitCapture(pCapture);
  }

  int ShimStopCaptureLogToFile(LPGPU2_CAPTURE *pCapture)
  {
    return Init::Instance()->DisableExplicitCapture(pCapture);
  }

  void ShimBeginCommandLogToFile(int api, int id)
  {
    Init::Instance()->BeginCommand(api,id);
  }

  int ShimShimTerminateFrameLogToFile(void)
  {
    return Init::Instance()->ExplicitTerminate();
  }

  void ShimEndCommandLogToFile(int api, int id)
  {
    Init::Instance()->EndCommand(api,id);
  }

  int ShimAnnotateCaptureLogToFile(LPGPU2_ANNOTATION *pAnn)
  {
    return Init::Instance()->ExplicitAnnotate(pAnn);
  }

  int ShimTrackResourceLogToFile(LPGPU2_RESOURCE_TYPE *pType, LPGPU2_RESOURCE_FLAGS *pFlags, uint64_t resource)
  {
    return Init::Instance()->TrackResource(pType, pFlags, resource);
  }

  int ShimStoreShaderLogToFile(uint api, uint64_t shader_id, uint type, int length, char *shader)
  {
    return Init::Instance()->StoreShader(api, shader_id, type, length, shader);
  }

  int ShimStoreShaderMetaDataLogToFile(uint api, uint64_t shader_id, uint shader_type, uint data_type, int length, char *shader)
  {
    return Init::Instance()->StoreShaderMetaData(api, shader_id, shader_type, data_type, length, shader);
  }

  int ShimStoreGPUTimingLogToFile(uint8_t *data, size_t size)
  {
    return Init::Instance()->SendData(data, size, CFS_LPGPU2_TYPE_GPUTIMER);
  }

  int ShimGetVersionLogToFile(float *version)
  {
    return Init::Instance()->GetVersion(version);
  }

  int ShimStoreAffinityLogToFile(uint id, uint cpu)
  {
    return Init::Instance()->StoreAffinity(id, cpu);
  }

  int ShimStoreUserCounterLogToFile(uint id, uint32_t value)
  {
    return Init::Instance()->StoreUserCounter(id, value);
  }

  extern "C" {

  extern DLL_PUBLIC int ShimSetSendDataCallback(SHIM_SEND_DATA_CALLBACK pfn, void *usrData)
  {
    return Init::Instance()->SetSendDataCallback(pfn,usrData );
  }

  extern DLL_PUBLIC int ShimStartCollecting(void)
  {
    return Init::Instance()->StartCollecting();
  }

  extern DLL_PUBLIC int ShimStopCollecting(void)
  {
    return Init::Instance()->StopCollecting();
  }

  extern DLL_PUBLIC int ShimGenerateTargetCharacteristics(XMLDocument& doc)
  {
    return Init::Instance()->GenerateTargetCharacteristics(doc);
  }
}

  std::string GetCommandLine()
  {
    std::string ret;
    char buffer[256];

    FILE *fp = fopen("/proc/self/cmdline", "rb");
    if ( fp ){
      fgets(buffer, sizeof(buffer), fp);
      fclose(fp);
      ret = std::string(buffer);
    }

    return ret;
  }

  Init::Init()
    : frameNumber(0),
      drawNumber(0),
      traceThisProcess(false),
      traceAPI(0),
      rangeStartFrame(-1),
      rangeStopFrame(-1),
      rangeStartDraw(-1),
      rangeStopDraw(-1),
      explicitControl(false),
      explicitCapture(false),
      explicitTerminator(false),
      dumpCallstack(false),
      callstackDumpAll(false),
      vulkanAllowCounters(true),
      frameTerminators(0),
      parameterOffset(0),
      parameterLength(0),
      type(0),
      format(0),
      fb_size(0),
      inRange(false),
      frameBuffer(nullptr)
  {
    lock.init();

    LogPrint("---------- Interposer Loading ----------\n");
    if(_instance != NULL)
    {
      LogPrint("Fatal: Only one instance\n");
      assert(_instance==NULL && "Only one init instance");
    }
    _instance = this;

    std::string processName( GetCommandLine() );

    LogPrint("Build date: %s\n", BUILD_DATE);

    InitializeLogger("Default");
    InitializeFunctionPointers();
    InitCommandPointers();
  } // Init()::Init()

  Init::~Init()
  {
    lock.destroy();

    if (frameBuffer)
    {
      free(frameBuffer);
    }
  }

  int Init::StartCollecting()
  {
    lock.init();

    // Reset State.
    frameNumber = 0;
    drawNumber = 0;
    traceThisProcess = false;
    traceAPI = 0;
    rangeStartFrame = -1;
    rangeStopFrame = -1;
    rangeStartDraw = -1;
    rangeStopDraw = -1;
    explicitControl = false;
    explicitCapture = false;
    explicitTerminator = false;
    dumpCallstack = false;
    callstackDumpAll = false;
    vulkanAllowCounters = true;
    frameTerminators = 0;
    parameterOffset = 0;
    parameterLength = 0;
    type = 0;
    format = 0;
    fb_size = 0;
    inRange = false;
    frameBuffer = nullptr;

    LogPrint("---------- Start Collecting. Shim Version v%.4f ----------\n", SHIM_VERSION);
    std::string filename(DEVICE_CONFIG_PATH);
    std::string processName( GetCommandLine() );

    filename = CONFIG_PATH;


    LogPrint("Command Line: %s\n", processName.c_str());
    filename = "/data/data/" + processName + filename;
    LogPrint("ConfigName: %s\n", filename.c_str());

    // Look for a config file in /data/local/tmp first
    FILE *fp = fopen("/data/local/tmp/CollectionDefinition2.xml", "r");
    if (fp)
    {
      fclose(fp);
      LogPrint("Found a CollectionDefinition2.xml in /data/local/tmp.=/ Using this instead of %s", filename.c_str());
      filename = "/data/local/tmp/CollectionDefinition2.xml";
    }
    else
    {
      LogPrint("No CollectionDefinition2.xml in /data/local/tmp/ Using the default %s", filename.c_str());
    }

    if(config.Parsed(filename))
    {
      std::string configLogLevel = config.QueryLogLevel();
      if(configLogLevel.size())
      {
        InitializeLogger(configLogLevel);
        LogPrint("LogLevel: %d '%s'\n", logLevel, LogLevelToString(logLevel).c_str());
      }
    }
    else
    {
      Log(LogError, "Error: No capture configuration file found '%s'\n", filename.c_str());
    }

    config.Read();

#if USE_MEGACHUNKS
    OptionalInt value = config.configLut.FindInt("NumberedBufferSize");
    megaBufferMaxSize = (MIN_MEGACHUNK_SIZE < *value) ? *value : MIN_MEGACHUNK_SIZE;
    megaBufferOffset = 0;
    megaBuffer = (uint8_t *) calloc(1, megaBufferMaxSize);
#endif
    std::string tracingProcessName(config.processName);
    if(char* env = getenv("LPGPU2_TRACE_PROCESS"))
    {
      LogPrint("Environment changed process name : %s\n", env);
      tracingProcessName = env;
    }

    if(OptionalInt value = config.configLut.FindInt("StatsWriteThreshold"))
    {
      stats.writeThreshold = *value;
    }

    LogPrint("   Process Request: '%s'\n", tracingProcessName.c_str());
    LogPrint("           Process: '%s'\n", processName.c_str());
    LogPrint("               PID: %d\n", getpid());
#if defined(GENERATED_GL)
    LogPrint("               TID: %d\n", syscall(SYS_gettid)); // gettid cant be called directly on host?
#else
    LogPrint("               TID: %d\n", gettid());
#endif

    char cwd[100];
    if(char* got = getcwd(cwd, sizeof(cwd)))
    {
      LogPrint("               CWD: %s\n", got);
    }
    else
    {
      LogPrint("               CWD: unknown?\n");
    }

    traceThisProcess = true;
    traceAPI = ApiIdFromString(config.apiNames);
    LogPrint("   TRACING PROCESS: ON\n");

    CheckStats(0); // reset the stats start time

    if(traceThisProcess)
    {

      // state init
      rangeStartFrame = config.startFrame;
      rangeStopFrame = config.stopFrame;
      rangeStartDraw = config.startDraw;
      rangeStopDraw = config.stopDraw;
      explicitControl = config.explicitControl;
      explicitTerminator = config.explicitTerminator;
      dumpCallstack = config.dumpCallstack;
      gpuTimingMode = config.gpuTimingMode;
      LogPrint("gpuTimingMode = 0x%llx \n", gpuTimingMode);
      // If frameTerminators not set, default to FT_EGL_SWAPBUFFERS
      LogPrint("PRE: frameTerminators = 0x%llx \n", config.frameTerminators);
      frameTerminators = config.frameTerminators ? config.frameTerminators : FT_EGL_SWAPBUFFERS;
      LogPrint("POST: frameTerminators = 0x%llx \n", config.frameTerminators);
      // Only dump an entry that has a symbol name. Set true to dump all.
      callstackDumpAll = false;

      Log(LogDebug,
          "            Frames: %d - %d Draws: %d - %d\n",
          rangeStartFrame,
          rangeStopFrame,
          rangeStartDraw,
          rangeStopDraw);

#if defined(GENERATED_VULKAN)
      auto currentPid = config.ReadPid();
      if(currentPid != getpid())
      {
        // in a vulkan context we get loaded once to iterate devices then we get loaded again as a layer.
        vulkanAllowCounters = false;
        Log(LogInfo, "Vulkan layer first load pid=%d pid read=%d", getpid(), currentPid);
      }
      else
      {
        Log(LogInfo, "Vulkan layer second load pid=%d pid read=%d", getpid(), currentPid);
      }
#endif // GENERATED_VULKAN


      config.WriteTargetCharacteristics();

      Log(LogInfo, "          Timeline: %d\n", (int)config.timelineNeeded);

      if(config.timelineNeeded)
      {
        ShimBeginCommand = ShimBeginCommandLogToFile;
        ShimEndCommand = ShimEndCommandLogToFile;
      }

      if(explicitControl)
      {
        Log(LogInfo, "Explicit Control has been enabled. The application has control of capture. \n");
        ShimStartCapture = ShimStartCaptureLogToFile;
        ShimStopCapture = ShimStopCaptureLogToFile;
      }

      if (explicitTerminator)
      {
        ShimTerminateFrame = ShimShimTerminateFrameLogToFile;
      }

      ShimAnnotateCapture = ShimAnnotateCaptureLogToFile;
      ShimTrackResource = ShimTrackResourceLogToFile;
      ShimStoreShader = ShimStoreShaderLogToFile;
      ShimStoreShaderMetaData = ShimStoreShaderMetaDataLogToFile;
      ShimStoreGPUTimer = ShimStoreGPUTimingLogToFile;
      ShimGetVersion = ShimGetVersionLogToFile;
      ShimStoreAffinity = ShimStoreAffinityLogToFile;
      ShimStoreUserCounter = ShimStoreUserCounterLogToFile;
    } // if(traceThisProcess)


    if (config.frameCapture)
    {
        configFrameCapture();
    }

    config.SetOverridingFunctions(CommandPointers, CommandCount);

    Log(LogInfo, "Finished Initialize\n");

    return 0;
  } // Init()::Init()

  int Init::StopCollecting()
  {
    lock.lock();
#if USE_MEGACHUNKS
    // Flush the MegaChunk.
    SendData(NULL, 0, CFS_LPGPU2_TYPE_FLUSHMEGA);
#endif

    // Send the End packet to mark the end of sending data.
    if ( pfnSendDataCallback )
    {
      pfnSendDataCallback(NULL, 0, CFS_LPGPU2_TYPE_END, userData);
    }
    // Switch off all the capture flags.
    gpuTimingMode = 0;
    config.frameCapture = 0;
    explicitControl = 0;
    traceThisProcess = 0;
    pfnSendDataCallback = NULL;

    lock.unlock();

    return 0;
  }

  int Init::GenerateTargetCharacteristics(XMLDocument& doc)
  {
    config.WriteClientTargetCharacteristics(doc);
    return 0;
  }

  void Init::SetupDevicePaths(const Config& cfg)
  {
    if(OptionalString path = cfg.configLut.FindString(Config::GL_PATH))
    {
      GL_LIBRARY_PATH = *path;
    }
    if(OptionalString path = cfg.configLut.FindString(Config::EGL_PATH))
    {
      EGL_LIBRARY_PATH = *path;
    }
    if(OptionalString path = cfg.configLut.FindString(Config::GLES2_PATH))
    {
      GLES2_LIBRARY_PATH = *path;
    }
    if(OptionalString path = cfg.configLut.FindString(Config::GLES3_PATH))
    {
      GLES3_LIBRARY_PATH = *path;
    }
    if(OptionalString path = cfg.configLut.FindString(Config::CL_PATH))
    {
      CL_LIBRARY_PATH = *path;
    }
  }

  void Init::NewFrame()
  {
    if (config.frameCapture)
    {
        frameCapture();
    }

    frameNumber ++;
    drawNumber = 0;
    if (gpuTimingMode & GPU_TIMING_MODE_FRAME_MASK)
    {
      if ( ! seenStart ){
        seenStart = true;
        // Need to wait until there is a context before initialising.
        gpuTiming.InitialiseGpuTimer(gpuTimingMode);
      }
      gpuTiming.FrameTimer(frameNumber);
    }
  }
  // an optimization here would be to replace the shimbegincommand function pointer
  // once a target pid has been identified, so we only have this overhead once
  void Init::BeginCommand(int api, int id)
  {
    lock.lock();

    // Check a previous Begin hasn't been seen before an End
    if (seenBegin)
    {
      lock.unlock();
      return;
    }
    seenBegin = true;

    if( ShouldTrace(api, id) )
    {
//      CallstackToLog(); - This causes a crash in vulkan capture Jira 224
      if (dumpCallstack)
        WriteCallstack(api, id, timelineBegin);
      timelineBegin = GetTime();
    }

    lock.unlock();
  }

  void Init::EndCommand(int api, int id)
  {
    lock.lock();
    // Check a previous End hasn't been seen before a Begin
    if (seenBegin == false)
    {
      lock.unlock();
      return;
    }
    seenBegin = false;

    if( ShouldTrace(api, id) )
    {
      WriteTimeLine((uint64_t)api, (uint64_t)id, timelineBegin, GetTime());
    }

    if ( gpuTimingMode && ( api == APIID_EGL ) && (ID_eglMakeCurrent == id))
    {
      seenStart = true;
      // Need to wait until there is a context before initialising.
      gpuTiming.InitialiseGpuTimer(gpuTimingMode);
    }

    if( !explicitTerminator && IsFrameTerminator(api, id) )
    {
      // If the Gpu timing causes a khr debug output, there is a potential lock situation.
      lock.unlock();
      NewFrame();
      lock.lock();
    }
    else
    {
      drawNumber ++;
    }

    lock.unlock();
  }

  int Init::EnableExplicitCapture(LPGPU2_CAPTURE* /*pCapture*/)
  {
    // Enable app controlled capture
    explicitCapture = true;
    return LPGPU2_API_SUCCESS;
  }

  int Init::DisableExplicitCapture(LPGPU2_CAPTURE* /*pCapture*/)
  {
    // Disable app controlled capture
    explicitCapture = false;
    StopCollecting();
    return LPGPU2_API_SUCCESS;
  }

  int Init::ExplicitTerminate()
  {
    // app controlled frame terminator
    if (explicitTerminator)
    {
      NewFrame();
      return LPGPU2_API_SUCCESS;
    }
    else
      return LPGPU2_API_FEATURE_DISABLED;
  }

  int Init::GetVersion(float *version)
  {
    *version = SHIM_VERSION;
    return LPGPU2_API_SUCCESS;
  }

  #define COUNTER_PACKET_DWORDS 5
  int Init::StoreAffinity(uint id, uint cpu)
  {
    lock.lock();
    uint32_t record[COUNTER_PACKET_DWORDS];
    uint64_t time = GetTime();

    *((uint64_t*)record) = time;
    record[2] = 1; // Number of counters
    record[3] = id;
    record[4] = cpu;

    SendData((uint8_t *)record, COUNTER_PACKET_DWORDS * sizeof(uint32_t), CFS_LPGPU2_TYPE_COUNTERV2);

    lock.unlock();
    return LPGPU2_API_SUCCESS;
  }

  int Init::StoreUserCounter(uint id, uint32_t value)
  {
    lock.lock();
    uint32_t record[COUNTER_PACKET_DWORDS];
    uint64_t time = GetTime();

    *((uint64_t*)record) = time;
    record[2] = 1; // Number of counters
    record[3] = id;
    record[4] = value;

    SendData((uint8_t *)record, COUNTER_PACKET_DWORDS * sizeof(uint32_t), CFS_LPGPU2_TYPE_COUNTERV2);

    lock.unlock();
    return LPGPU2_API_SUCCESS;
  }

  bool Init::SendData(uint8_t *buffer, size_t dataSize, uint32_t dataType)
  {

#if USE_MEGACHUNKS
    // Add the data to the Mega Chunk
    // Create the header.

    MegaChunkHeader header;
    header.magicHeader = ( 'D' << 24 | 'C' << 16 | 'C' << 8 | 'H' );
    header.chunkID = dataType;
    header.chunkFlags = 0;
    header.chunkSize = sizeof(header) + dataSize;

    // Has the buffer got enough space?
    if (((megaBufferOffset + header.chunkSize) > megaBufferMaxSize) ||
        ((dataType == CFS_LPGPU2_TYPE_FLUSHMEGA) && megaBufferOffset))
    {
      if ( pfnSendDataCallback )
      {
        pfnSendDataCallback(megaBuffer, megaBufferOffset, CFS_LPGPU2_TYPE_MEGA, userData);
      }
      // reset the state.
      megaBufferOffset = 0;
    }

    memcpy(megaBuffer + megaBufferOffset, &header, sizeof(header));
    megaBufferOffset += sizeof(header);
    memcpy(megaBuffer + megaBufferOffset, buffer, dataSize);
    megaBufferOffset += dataSize;
    return true;
#else
    if ( pfnSendDataCallback )
    {
      return pfnSendDataCallback(buffer, dataSize, dataType, userData);
    }
    return false;
#endif
  }

  int Init::ExplicitAnnotate(LPGPU2_ANNOTATION *pAnn)
  {
    lock.lock();
    int strlength = strlen(pAnn->dwString);
    int dataSize = strlength + sizeof(struct AnnotateRecord);

    struct AnnotateRecord record;
    record.id = 0;
    record.frame_no = static_cast<uint64_t>(frameNumber);
    record.draw_no = static_cast<uint64_t>(drawNumber);
    record.type = pAnn->dwType;
    record.startTime = GetTime();
    record.endTime = record.startTime;
    record.strlen = strlength;

    uint8_t *annBuffer = (uint8_t *) malloc(dataSize);
    if (nullptr == annBuffer)
    {
       Log(LogError, "Error: OOM creating frame buffer for annotation: buffer size %d.\n", dataSize);
       lock.unlock();
       return LPGPU2_API_OOM_ERROR;
    }
    memcpy(annBuffer, &record, sizeof(struct AnnotateRecord));
    memcpy(annBuffer + sizeof(struct AnnotateRecord), pAnn->dwString, strlength);
    SendData(annBuffer, (size_t)(dataSize), CFS_LPGPU2_TYPE_ANNOTATIONS);
    free(annBuffer);
    lock.unlock();
    return LPGPU2_API_SUCCESS;
  }

  int Init::TrackResource(LPGPU2_RESOURCE_TYPE *pType, LPGPU2_RESOURCE_FLAGS *pFlags, uint64_t resource)
  {
    lock.lock();

    struct TrackResourceRecord record;
    record.frame_no = static_cast<uint64_t>(frameNumber);
    record.draw_no = static_cast<uint64_t>(drawNumber);
    record.type = pType->dwFlags;
    record.flag = pFlags->dwFlags;
    record.resource = resource;
    record.cpu_time = GetTime();

    SendData(reinterpret_cast<unsigned char*>(&record), (size_t)(sizeof(record)), CFS_LPGPU2_TYPE_TRACKING);

    lock.unlock();
    return LPGPU2_API_SUCCESS;
  }

  bool Init::ShimStoreParameters(ParamsHeader& header, size_t noBytes)
  {
    bool ret = true;
    lock.lock();
    header.frameNumber = frameNumber;
    header.drawNumber = drawNumber;

#if defined(GENERATED_GL)
    header.threadId = static_cast<uint64_t>((SYS_gettid)); // gettid cant be called directly on host?
#else
    header.threadId = static_cast<uint64_t>(gettid());
#endif

    // A bit clunky. api_id and cmd_id follow after the header.
    uint32_t *api_id = (uint32_t *)((unsigned char*)(&header) + sizeof(ParamsHeader));
    uint32_t *cmd_id = (uint32_t *)((unsigned char*)(&header) + sizeof(ParamsHeader) + sizeof(uint32_t));
    if (ShouldTrace(*api_id, *cmd_id))
    {
      ret = SendData(reinterpret_cast<unsigned char*>(&header), noBytes, CFS_LPGPU2_TYPE_PARAMS);
    }

    lock.unlock();
    return ret;
  }

  int Init::StoreShader(uint api, uint64_t shader_id, uint shader_type, int length, char *shader)
  {
    if (!ShouldTrace(api, 0))
    {
      return LPGPU2_API_SUCCESS;
    }
    lock.lock();

    if (length < 0)
    {
      length = strlen(shader);
    }
    int dataSize = length + sizeof(struct ShaderRecord);

    struct ShaderRecord record;
    record.frame_no = static_cast<uint64_t>(frameNumber);
    record.draw_no = static_cast<uint64_t>(drawNumber);
    record.shader_id = shader_id;
    record.shader_type = shader_type;
    record.api = api;
    record.cpu_time = GetTime();
    record.strlen = length;

    uint8_t *shaderBuffer = (uint8_t *) malloc(dataSize);
    if (nullptr == shaderBuffer)
    {
       Log(LogError, "Error: OOM creating buffer for shader: buffer size %d.\n", dataSize);
       lock.unlock();
       return LPGPU2_API_OOM_ERROR;
    }
    memcpy(shaderBuffer, &record, sizeof(struct ShaderRecord));
    if (length)
    {
      memcpy(shaderBuffer + sizeof(struct ShaderRecord), shader, length);
    }
    SendData(shaderBuffer, (size_t)(dataSize), CFS_LPGPU2_TYPE_SHADER);

    free(shaderBuffer);
    lock.unlock();
    return LPGPU2_API_SUCCESS;
  }

  int Init::StoreShaderMetaData(uint api, uint64_t shader_id, uint shader_type, uint data_type, int length, char *shader)
  {
    if (!ShouldTrace(api, 0))
    {
      return LPGPU2_API_SUCCESS;
    }
    lock.lock();

    if (length < 0)
    {
      length = strlen(shader);
    }
    int dataSize = length + sizeof(struct ShaderMetaRecord);

    struct ShaderMetaRecord record;

    record.shader_id = shader_id;
    record.shader_type = shader_type;
    record.data_type = data_type;
    record.api = api;
    record.strlen = length;

    uint8_t *shaderBuffer = (uint8_t *) malloc(dataSize);
    if (nullptr == shaderBuffer)
    {
       Log(LogError, "Error: OOM creating buffer for shader: buffer size %d.\n", dataSize);
       lock.unlock();
       return LPGPU2_API_OOM_ERROR;
    }
    memcpy(shaderBuffer, &record, sizeof(struct ShaderMetaRecord));
    memcpy(shaderBuffer + sizeof(struct ShaderMetaRecord), shader, length);

    SendData(shaderBuffer, (size_t)(dataSize), CFS_LPGPU2_TYPE_SHADER_META);

    free(shaderBuffer);
    lock.unlock();
    return LPGPU2_API_SUCCESS;
  }

  int Init::SetSendDataCallback(SHIM_SEND_DATA_CALLBACK pfn, void *usrData)
  {
    lock.lock();
    pfnSendDataCallback = pfn;
    userData = usrData;
    lock.unlock();
    return (pfnSendDataCallback == NULL);
  }

  void Init::WriteTimeLine(uint64_t api, uint64_t id, uint64_t begin, uint64_t end)
  {
    TimelineRecord rec = { id,
                           begin,
                           end,
                           static_cast<uint64_t>(frameNumber),
                           static_cast<uint64_t>(drawNumber),
                           api,
                           parameterOffset,
                           parameterLength };

    if(SendData((uint8_t *)&rec, (size_t)sizeof(rec), CFS_LPGPU2_TYPE_TRACE))
    {
      CheckStats(sizeof(rec));
    }
  }

  _Unwind_Reason_Code Init::unwindHandler(struct _Unwind_Context* ctx, void* arg)
  {
    CallstackTrace* state = static_cast<CallstackTrace*>(arg);
    _Unwind_Ptr ptr = _Unwind_GetIP(ctx);
    if (ptr && state)
    {
        if (state->current == state->end)
        {
          return _URC_END_OF_STACK;
        }
        else
        {
          *state->current = (void*)(ptr);
          state->current++;
        }
    }
    return _URC_NO_REASON;
  }

  size_t Init::getCallstack(void** buffer, size_t max)
  {
    CallstackTrace state;
    state.current = buffer;
    state.end = buffer + max;
    _Unwind_Backtrace(unwindHandler, &state);

    return state.current - buffer;
  }

  void Init::CallstackToLog(void)
  {
    void* buffer[CALLSTACK_MAX];

    int count = getCallstack(buffer, CALLSTACK_MAX);

    Log(LogVerbose, " Start Callstack Dump ");
    for (int i = 0; i < count; ++i)
    {
      Dl_info info;
      if (dladdr(buffer[i], &info))
      {
        if (callstackDumpAll || info.dli_sname)
        {
          Log(LogVerbose, "0x%x 0x%x 0x%x %s %s \n", buffer[i], info.dli_fbase, info.dli_saddr, info.dli_fname, info.dli_sname);
        }
      }
    }
    Log(LogVerbose, " End Callstack Dump ");
  }

  void Init::WriteCallstack(uint64_t api, uint64_t id, uint64_t begin)
  {
    void* buffer[CALLSTACK_MAX];
    struct CallstackHeader header;
    uint8_t *pBuffer;
    struct CallstackRecord *record;
    int bufferSize;
    uint8_t *cbBuffer;
    int strSize = 0, records = 0;
    int count = getCallstack(buffer, CALLSTACK_MAX);

    header.apiID = api;
    header.id = id;
    header.startTime = begin;
    header.frame_no = static_cast<uint64_t>(frameNumber);
    header.draw_no = static_cast<uint64_t>(drawNumber);

    // Allocate buffer to store records
    bufferSize = count * sizeof(struct CallstackRecord);
    record = (struct CallstackRecord *) malloc(bufferSize);
    if (nullptr == record)
    {
      Log(LogError, "Error: OOM creating record buffer for callstack: buffer size %d.\n", bufferSize);
      return;
    }

    // Calculate size to write to CFS, and fill records.
    for (int i = 0; i < count; ++i)
    {
      const void* addr = buffer[i];

      Dl_info info;
      if (dladdr(addr, &info))
      {
        if (callstackDumpAll || info.dli_sname)
        {
          record[records].dli_fbase = (uint64_t)info.dli_fbase;
          record[records].dli_saddr = (uint64_t)info.dli_saddr;
          if (info.dli_fname)
            record[records].dli_fname_len = strlen(info.dli_fname) + 1;
          else
            record[records].dli_fname_len = 0;
          if (info.dli_sname)
            record[records].dli_sname_len = strlen(info.dli_sname) + 1;
          else
            record[records].dli_sname_len = 0;

          strSize += record[records].dli_fname_len + record[records].dli_sname_len;
          records++;
        }
      }
    }
    header.records = records;

    bufferSize = sizeof(CallstackHeader) + (records * sizeof(CallstackRecord)) + strSize;

    // Allocate buffer to write to CFS
    cbBuffer = (uint8_t *) malloc(bufferSize);
    if (nullptr == cbBuffer)
    {
      Log(LogError, "Error: OOM creating output buffer for callstack: buffer size %d.\n", bufferSize);
      free(record);
      return;
    }
    pBuffer = cbBuffer;

    memcpy(pBuffer, &header, sizeof(struct CallstackHeader));
    pBuffer += sizeof(struct CallstackHeader);
    records = 0;
    // Dump the records, and names into the buffer.
    for (int i = 0; i < count; ++i)
    {
      Dl_info info;
      if (dladdr(buffer[i], &info))
      {
        if (callstackDumpAll || info.dli_sname)
        {
          memcpy(pBuffer, &record[records], sizeof(CallstackRecord));
          pBuffer += sizeof(CallstackRecord);
          memcpy(pBuffer, info.dli_fname, record[records].dli_fname_len);
          pBuffer += record[records].dli_fname_len;
          memcpy(pBuffer, info.dli_sname, record[records].dli_sname_len);
          pBuffer += record[records].dli_sname_len;
          records++;
        }
      }
    }
    // Check the size is as expected.
    if (bufferSize != (pBuffer - cbBuffer))
    {
        Log(LogError, "Error: Buffer size mis-match. Expected %d, got %d\n", bufferSize, (pBuffer - cbBuffer));
        free(cbBuffer);
        free(record);
        return;
    }

    SendData(cbBuffer, (size_t)(bufferSize), CFS_LPGPU2_TYPE_STACK);
    free(cbBuffer);
    free(record);
  }

  bool Init::IsFrameTerminator(int api, int id)
  {
    bool rv = false;

    // Need a way to lookup ID based on string names for the various egl swap functions
    if ( api == APIID_GLES2)
    {
      if ( (ID_glClear == id) && (frameTerminators & FT_GL_CLEAR) )
      {
          rv = true;
      }
      if ( (ID_glFlush == id) && (frameTerminators & FT_GL_FLUSH) )
      {
          rv = true;
      }
      if ( (ID_glFinish == id) && (frameTerminators & FT_GL_FINISH) )
      {
          rv = true;
      }
    }
    else if ( api == APIID_EGL )
    {
#if defined(GENERATED_EGL)
      if ( (ID_eglSwapBuffers == id) && (frameTerminators & FT_EGL_SWAPBUFFERS) )
      {
          rv = true;
      }
#endif
    }
    else if ( api == APIID_CL )
    {
      if ( (ID_clFlush == id) && (frameTerminators & FT_CL_FLUSH) )
      {
          rv = true;
      }
      if ( (ID_clFinish == id) && (frameTerminators & FT_CL_FINISH) )
      {
          rv = true;
      }
      if ( (ID_clWaitForEvents == id) && (frameTerminators & FT_CL_WAITFOREVENTS) )
      {
          rv = true;
      }
    }
#if defined(GENERATED_VULKAN)
    else if ( api == APIID_VULKAN )
    {
      if ( (ID_vkQueuePresentKHR == id) && (frameTerminators & FT_VK_QUEUEPRESENTKHR) )
      {
          rv = true;
      }
    }
#endif
    return ( rv );
  }

  bool Init::RangeCheck(void)
  {
    if (rangeStartFrame >= 0 && rangeStopFrame >= 0)
    {
      // Frame range enabled.
      if (!inRange && (frameNumber >= rangeStartFrame))
      {
        // Not in range, look for start
        if (rangeStartDraw >= 0 && rangeStopDraw >= 0 )
        {
          // Draw Range also enabled
          if ( drawNumber >= rangeStartDraw )
          {
            inRange = true;
          }
          else if (frameNumber > rangeStartFrame)
          {
            // The rangeStartDraw is higher than the number of draws.
            inRange = true;
          }
        }
        else
        {
          // Only Frame ranging
          inRange = true;
        }
      }
      if (inRange)
      {
        // In range, look for end
        if (frameNumber >=  rangeStopFrame)
        {
          // Not in range, look for start
          if (rangeStartDraw >= 0 && rangeStopDraw >= 0 )
          {
            // Draw Range also enabled
            if (drawNumber > rangeStopDraw)
            {
              inRange = false;
            }
            else if (frameNumber > rangeStopFrame)
            {
              // The rangeStartDraw is higher than the number of draws.
              inRange = false;
            }
          }
          else if (frameNumber > rangeStopFrame)
          {
            // Only Frame ranging
            inRange = false;
          }
        }
        if (!inRange)
        {
          // Finished collecting the range.
          // StopCollecting() locks, so unlock first
          lock.unlock();
          StopCollecting();
          // ... and then lock again.
          lock.lock();
        }
      }
    }
    else if (rangeStartDraw >= 0 && rangeStopDraw >= 0)
    {
      // Only draw range enabled
      if (rangeStartDraw <= drawNumber && drawNumber <= rangeStopDraw)
      {
        inRange = true;
      }
      else
      {
        inRange = false;
      }
    }
    else // Range not enabled / valid.
    {
      inRange = true;
    }

    return inRange;
  }

  bool Init::ShouldTrace(int api, int /*id*/)
  {
    // Called from inside a lock.
    bool rv = true;

    if (explicitControl)
    {
      rv = explicitCapture;
      if(!explicitCapture)
      {
        Log(LogVerbose, "Rejecting as under explicit control and rejected\n");
      }
    }
    else if( traceThisProcess && (traceAPI & api) )
    {
      rv = RangeCheck();
    }
    else
    {
      // were not tracing this process or API
      Log(LogVerbose, "Rejecting as not tracing this process or API traceAPI ID=%d This API ID=%d traceThisProcess?=%d\n",
          traceAPI, api, traceThisProcess);
      rv = false;
    }

    return ( rv );
  }

  void Init::configFrameCapture()
  {
    int fd;

    fd = open("/dev/graphics/fb0", O_RDWR);

    if (fd < 0)
    {
        Log(LogError, "Error: Cannot open Frame Buffer for screen info fd=%d.\n", fd);
        return;
    }

    if (!ioctl(fd, FBIOGET_VSCREENINFO, &v_si))
    {

#if 0
        // When using this call to get implementation specific pixel info,
        // values returned to not work with glReadPixels()
        _glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE, (GLint *) &type);
        _glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, (GLint *) &format);
#else
#if 0
        // When using an S6 device the FB driver returns a bpp of 32, but
        // 32 bit pixel formats aren't recognised by Android 9 platforms
        // as they use only GLES2.0.
        switch(v_si.bits_per_pixel)
        {
#ifdef GL_ES_VERSION_3_0
            case 32:
                    if (8 == v_si.green.length)
                    {
                        type = GL_UNSIGNED_INT_8_8_8_8;
                        format = GL_RGBA;
                    }
                    else
                    {
                        type = GL_UNSIGNED_INT_10_10_10_2;
                        format = GL_RGBA;
                    }
                break;
#endif
            case 16:
                    if (6 == v_si.green.length)
                    {
                        type = GL_UNSIGNED_SHORT_5_6_5;
                        format = GL_RGB;
                        Log(LogError, "RGB565\n");
                        // Fixing to 16 bits_per_pixel as implementation of
                        // pixel info does not work with glReadPixels()
                        v_si.bits_per_pixel = 16;
                    }
                    else if (4 == v_si.green.length)
                    {
                        type = GL_UNSIGNED_SHORT_4_4_4_4;
                        format = GL_RGBA;
                        Log(LogError, "RGBA4444\n");
                    }
                    else
                    {
                        type = GL_UNSIGNED_SHORT_5_5_5_1;
                        format = GL_RGBA;
                        Log(LogError, "RGBA5551\n");
                    } */
                break;
            default:
                Log(LogError, "Error: bits_per_pixel is invalid/not supported %d.\n", v_si.bits_per_pixel);
                config.frameCapture = false;
        }
#else
        // Fixing to 16 bits_per_pixel as implementation of
        // pixel info does not work with glReadPixels()
        type = GL_UNSIGNED_SHORT_5_6_5;
        format = GL_RGB;
        Log(LogError, "RGB565\n");
        v_si.bits_per_pixel = 16;
#endif
#endif

        fb_size = v_si.xres * v_si.yres * (v_si.bits_per_pixel >> 3);

        if (fb_size == 0)
        {
                Log(LogError, "Error: FB Size is 0. XRes %d Yres %d bpp %d\n", v_si.xres, v_si.yres, v_si.bits_per_pixel);
                config.frameCapture = false;
        }

        if (config.frameCapture)
        {
            frameBuffer = (uint8_t *) malloc(fb_size + sizeof(struct FB_Record));
        }

        if ((nullptr == frameBuffer) && config.frameCapture)
        {
            Log(LogError, "Error: OOM creating frame buffer for frame capture: fb_size %d.\n", fb_size);
            config.frameCapture = false;
        }
        else if (nullptr != frameBuffer)
        {
            Log(LogInfo, "Configured for Frame capture\n");
        }
    }
    else
    {
        Log(LogError, "Error: Cannot get screen info for frame capture.\n");
        config.frameCapture = false;
    }

    close(fd);
  }

  void Init::frameCapture()
  {
    struct FB_Record record;
    record.frame_no = static_cast<uint64_t>(frameNumber);
    record.draw_no = static_cast<uint64_t>(drawNumber);
    record.x_res = v_si.xres;
    record.y_res = v_si.yres;
    record.bpp = v_si.bits_per_pixel;
    record.fb_size = fb_size;
    record.type = type;
    record.format = format;

    original_glReadPixels(0, 0, v_si.xres, v_si.yres, format, type, (frameBuffer + sizeof(struct FB_Record)));
    GLenum err = original_glGetError();
    if (!err)
    {
        memcpy(frameBuffer, &record, sizeof(struct FB_Record));
        SendData((uint8_t *)frameBuffer, (size_t)(fb_size  + sizeof(struct FB_Record)), CFS_LPGPU2_TYPE_TEXTURE);
    }
    else
    {
        Log(LogError, "ERROR! failed to read FB pixel data: err 0x%0x\n", err);
    }
  }

  void Init::CheckStats(size_t bytesWritten)
  {
    if(stats.writeThreshold > 0)
    {
      stats.bytesWritten += bytesWritten;
      if(static_cast<int64_t>(stats.bytesWritten) > stats.writeThreshold)
      {
        uint64_t now = GetTime();
        double seconds = ((now - stats.startTime) / 1e6);
        if(0.0 == seconds)
        {
          seconds = 0.0000000001;
        }

        stats.sumKbPerSecond += (stats.bytesWritten/1000.0) / seconds;

        Log(LogInfo, "Bytes written: %010d bytes, %f seconds (%f Kb/sec). Avg over %02d %f Kb/Sec",
            stats.bytesWritten,
            seconds,
            (stats.bytesWritten/1000.0) / seconds,
            stats.averageCount,
            (float)stats.sumKbPerSecond/stats.averageCount
          );

        stats.averageCount++;

        if(stats.averageCount > stats.averageLimit)
        {
          stats.averageCount = 1;
          stats.sumKbPerSecond = 0;
        }

        stats.bytesWritten = 0;
        stats.startTime = now;
      }
    }
  }

  //
  // static initialization; ie on dynamic library load
  //
  Init init;

} // anon namespace

void ShimStoreParameters(ParamsHeader& header, size_t noBytes)
{
  Init::Instance()->ShimStoreParameters(header, noBytes);
}


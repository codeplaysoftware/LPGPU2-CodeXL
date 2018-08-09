//==============================================================================
// Copyright (c) 2015 Advanced Micro Devices, Inc. All rights reserved.
/// \author AMD Developer Tools Team
/// \file
/// \brief This file contains the entry point for the CLTraceAgent.
//==============================================================================

#include <string.h>
#include <iostream>
#include <fstream>
#include "CLTraceAgent.h"
#include "../CLCommon/CLFunctionDefs.h"
#include "CLAPITraceEntry.h"
#include "CLEventHandler.h"
#include "CLEventManager.h"
#include "CLAPIInfoManager.h"
#include "PMCSamplerManager.h"
#include "../Common/Logger.h"
#include "../Common/FileUtils.h"
#include "../Common/GlobalSettings.h"
#include "../Common/Version.h"
#include "../Common/OSUtils.h"
#include "../Common/StackTracer.h"
#include <AMDTOSWrappers/Include/osModule.h>
#include <AMDTOSWrappers/Include/osFilePath.h>
#include <AMDTOSAPIWrappers/Include/oaDriver.h>
#include <AMDTOSWrappers/Include/osProcess.h>

#ifndef _WIN32
#include <dlfcn.h>
#endif // WIN32

#include <iostream>
#include <string>

using namespace std;
using namespace GPULogger;

static bool                  bAlreadyInit = false;
static cl_icd_dispatch_table original_dispatch;
static cl_icd_dispatch_table modified_dispatch;
#ifdef _WIN32
static osModuleHandle        cl_module_handle;
#endif // _WIN32



void DumpTrace()
{
    static bool alreadyDumped = false;

    if (!alreadyDumped)
    {
        alreadyDumped = true;

        if (!CLAPIInfoManager::Instance()->IsTimeOutMode())
        {
            CLAPIInfoManager::Instance()->SaveToOutputFile();
        }
        else
        {
            CLAPIInfoManager::Instance()->StopTimer();
            CLAPIInfoManager::Instance()->FlushTraceData( true );
            CLAPIInfoManager::Instance()->TrySwapBuffer();
            CLAPIInfoManager::Instance()->FlushTraceData( true );
        }

        CLEventManager::Instance()->Release();
        CLAPIInfoManager::Instance()->Release();
    }
}

extern "C" DLL_PUBLIC void amdtCodeXLStopProfiling()
{
    CLAPIInfoManager::Instance()->StopTracing();
}

extern "C" DLL_PUBLIC void amdtCodeXLResumeProfiling()
{
    CLAPIInfoManager::Instance()->ResumeTracing();
}

#ifdef _WIN32
    #include <windows.h>
#endif

#ifdef _WIN32
/// Vectored Exception Handler used to handle crashes in a profiled app during automated tests
/// This exception handler is only installed when the profiler is started in test mode (--__testmode__)
/// This handler just terminates the process (the assumption being that test apps will never raise an OS exception)
/// \param exception_info the exception information structure containing the exception record
/// \return a value indicating what should be done with the exception
LONG CALLBACK TestModeExceptionHandler(PEXCEPTION_POINTERS except_info)
{
    if (except_info->ExceptionRecord->ExceptionCode >= STATUS_ACCESS_VIOLATION)
    {
        TerminateProcess(GetCurrentProcess(), TEST_EXCEPTION_EXIT_CODE);
        return EXCEPTION_CONTINUE_EXECUTION;
    }

    return EXCEPTION_CONTINUE_SEARCH;
}

// On Windows, we can't dump data in OnUnload() because of ocl runtime bug
extern "C" DLL_PUBLIC void OnExitProcess()
{
    DumpTrace();
}

BOOL APIENTRY DllMain(HMODULE,
                      DWORD   ul_reason_for_call,
                      LPVOID)
{
    switch (ul_reason_for_call)
    {
        case DLL_PROCESS_ATTACH:
            CLTraceAgentInit();
            break;

        case DLL_THREAD_ATTACH:
            break;

        case DLL_PROCESS_DETACH:
        {
            // Dump all data out
            CLTraceAgentDeinit();
            OSUtils::Instance()->ShutdownUserTimer(); //TODO: need to call this on Linux as well
        }
        break;

        case DLL_THREAD_DETACH:
            break;
    }

    return TRUE;
}

#else

void __attribute__((destructor)) libUnload(void)
{
    /*
    Don't call unload() before ocl runtime fixed this bug, anything called inside this function will have unexpected behaviors
    if( CLAPIInfoManager::Instance()->IsTimeOutMode() )
    {
    CLAPIInfoManager::Instance()->StopTimer();
    CLAPIInfoManager::Instance()->FlushTraceData( true );
    CLAPIInfoManager::Instance()->TrySwapBuffer();
    CLAPIInfoManager::Instance()->FlushTraceData( true );
    }
    else
    {
    CLAPIInfoManager::Instance()->SaveAPITraceDataToFile();
    CLAPIInfoManager::Instance()->SaveTimestampToFile();
    }

    CLAPIInfoManager::Instance()->StopTimer();

    CLEventManager::Instance()->Release();
    CLAPIInfoManager::Instance()->Release();
    */
}

#endif

cl_icd_dispatch_table* GetRealDispatchTable()
{
    return  &original_dispatch;
}

#ifdef CL_UNITTEST_MOCK
void SetRealDispatchTblToMock(void)
{
    SetRealDispatchTblToMock(original_dispatch);
}
#endif

void* GetFunctionByName(const char* name) {
#ifdef _WIN32
    osProcedureAddress pFunction = nullptr;
    osGetProcedureAddress(cl_module_handle, name, pFunction, false);

    // TODO some of these lookups fail! Not sure what happened in previous implemenation.
    // This hasn't caused any problems that I've found (yet)
    if (pFunction == nullptr)
    {
        std::cerr << "Could not find procedure " << name << std::endl;
    }

    return (void*)(pFunction);
#else
    return dlsym(RTLD_DEFAULT, name);
#endif // _WIN32
}

void GetDispatchTable(cl_icd_dispatch_table* table) {
    table->GetPlatformIDs = (decltype(table->GetPlatformIDs))GetFunctionByName("clGetPlatformIDs");
    table->GetPlatformInfo = (decltype(table->GetPlatformInfo))GetFunctionByName("clGetPlatformInfo");
    table->GetDeviceIDs = (decltype(table->GetDeviceIDs))GetFunctionByName("clGetDeviceIDs");
    table->GetDeviceInfo = (decltype(table->GetDeviceInfo))GetFunctionByName("clGetDeviceInfo");
    table->CreateContext = (decltype(table->CreateContext))GetFunctionByName("clCreateContext");
    table->CreateContextFromType = (decltype(table->CreateContextFromType))GetFunctionByName("clCreateContextFromType");
    table->RetainContext = (decltype(table->RetainContext))GetFunctionByName("clRetainContext");
    table->ReleaseContext = (decltype(table->ReleaseContext))GetFunctionByName("clReleaseContext");
    table->GetContextInfo = (decltype(table->GetContextInfo))GetFunctionByName("clGetContextInfo");
    table->CreateCommandQueue = (decltype(table->CreateCommandQueue))GetFunctionByName("clCreateCommandQueue");
    table->RetainCommandQueue = (decltype(table->RetainCommandQueue))GetFunctionByName("clRetainCommandQueue");
    table->ReleaseCommandQueue = (decltype(table->ReleaseCommandQueue))GetFunctionByName("clReleaseCommandQueue");
    table->GetCommandQueueInfo = (decltype(table->GetCommandQueueInfo))GetFunctionByName("clGetCommandQueueInfo");
    table->SetCommandQueueProperty = (decltype(table->SetCommandQueueProperty))GetFunctionByName("clSetCommandQueueProperty");
    table->CreateProgramWithSource = (decltype(table->CreateProgramWithSource))GetFunctionByName("clCreateProgramWithSource");
    table->CreateProgramWithBinary = (decltype(table->CreateProgramWithBinary))GetFunctionByName("clCreateProgramWithBinary");
    table->RetainProgram = (decltype(table->RetainProgram))GetFunctionByName("clRetainProgram");
    table->ReleaseProgram = (decltype(table->ReleaseProgram))GetFunctionByName("clReleaseProgram");
    table->BuildProgram = (decltype(table->BuildProgram))GetFunctionByName("clBuildProgram");
    table->UnloadCompiler = (decltype(table->UnloadCompiler))GetFunctionByName("clUnloadCompiler");
    table->GetProgramInfo = (decltype(table->GetProgramInfo))GetFunctionByName("clGetProgramInfo");
    table->GetProgramBuildInfo = (decltype(table->GetProgramBuildInfo))GetFunctionByName("clGetProgramBuildInfo");
    table->CreateKernel = (decltype(table->CreateKernel))GetFunctionByName("clCreateKernel");
    table->CreateKernelsInProgram = (decltype(table->CreateKernelsInProgram))GetFunctionByName("clCreateKernelsInProgram");
    table->RetainKernel = (decltype(table->RetainKernel))GetFunctionByName("clRetainKernel");
    table->ReleaseKernel = (decltype(table->ReleaseKernel))GetFunctionByName("clReleaseKernel");
    table->SetKernelArg = (decltype(table->SetKernelArg))GetFunctionByName("clSetKernelArg");
    table->GetKernelInfo = (decltype(table->GetKernelInfo))GetFunctionByName("clGetKernelInfo");
    table->GetKernelWorkGroupInfo = (decltype(table->GetKernelWorkGroupInfo))GetFunctionByName("clGetKernelWorkGroupInfo");
    table->WaitForEvents = (decltype(table->WaitForEvents))GetFunctionByName("clWaitForEvents");
    table->GetEventInfo = (decltype(table->GetEventInfo))GetFunctionByName("clGetEventInfo");
    table->RetainEvent = (decltype(table->RetainEvent))GetFunctionByName("clRetainEvent");
    table->ReleaseEvent = (decltype(table->ReleaseEvent))GetFunctionByName("clReleaseEvent");
    table->GetEventProfilingInfo = (decltype(table->GetEventProfilingInfo))GetFunctionByName("clGetEventProfilingInfo");
    table->Flush = (decltype(table->Flush))GetFunctionByName("clFlush");
    table->Finish = (decltype(table->Finish))GetFunctionByName("clFinish");
    table->EnqueueNDRangeKernel = (decltype(table->EnqueueNDRangeKernel))GetFunctionByName("clEnqueueNDRangeKernel");
    table->EnqueueTask = (decltype(table->EnqueueTask))GetFunctionByName("clEnqueueTask");
    table->EnqueueNativeKernel = (decltype(table->EnqueueNativeKernel))GetFunctionByName("clEnqueueNativeKernel");
    table->EnqueueMarker = (decltype(table->EnqueueMarker))GetFunctionByName("clEnqueueMarker");
    table->EnqueueWaitForEvents = (decltype(table->EnqueueWaitForEvents))GetFunctionByName("clEnqueueWaitForEvents");
    table->EnqueueBarrier = (decltype(table->EnqueueBarrier))GetFunctionByName("clEnqueueBarrier");
    table->CreateBuffer = (decltype(table->CreateBuffer))GetFunctionByName("clCreateBuffer");
    table->CreateImage2D = (decltype(table->CreateImage2D))GetFunctionByName("clCreateImage2D");
    table->CreateImage3D = (decltype(table->CreateImage3D))GetFunctionByName("clCreateImage3D");
    table->RetainMemObject = (decltype(table->RetainMemObject))GetFunctionByName("clRetainMemObject");
    table->ReleaseMemObject = (decltype(table->ReleaseMemObject))GetFunctionByName("clReleaseMemObject");
    table->GetSupportedImageFormats = (decltype(table->GetSupportedImageFormats))GetFunctionByName("clGetSupportedImageFormats");
    table->GetMemObjectInfo = (decltype(table->GetMemObjectInfo))GetFunctionByName("clGetMemObjectInfo");
    table->GetImageInfo = (decltype(table->GetImageInfo))GetFunctionByName("clGetImageInfo");
    table->CreateSampler = (decltype(table->CreateSampler))GetFunctionByName("clCreateSampler");
    table->RetainSampler = (decltype(table->RetainSampler))GetFunctionByName("clRetainSampler");
    table->ReleaseSampler = (decltype(table->ReleaseSampler))GetFunctionByName("clReleaseSampler");
    table->GetSamplerInfo = (decltype(table->GetSamplerInfo))GetFunctionByName("clGetSamplerInfo");
    table->EnqueueReadBuffer = (decltype(table->EnqueueReadBuffer))GetFunctionByName("clEnqueueReadBuffer");
    table->EnqueueWriteBuffer = (decltype(table->EnqueueWriteBuffer))GetFunctionByName("clEnqueueWriteBuffer");
    table->EnqueueReadImage = (decltype(table->EnqueueReadImage))GetFunctionByName("clEnqueueReadImage");
    table->EnqueueWriteImage = (decltype(table->EnqueueWriteImage))GetFunctionByName("clEnqueueWriteImage");
    table->EnqueueMapBuffer = (decltype(table->EnqueueMapBuffer))GetFunctionByName("clEnqueueMapBuffer");
    table->EnqueueMapImage = (decltype(table->EnqueueMapImage))GetFunctionByName("clEnqueueMapImage");
    table->EnqueueUnmapMemObject = (decltype(table->EnqueueUnmapMemObject))GetFunctionByName("clEnqueueUnmapMemObject");
    table->EnqueueCopyBuffer = (decltype(table->EnqueueCopyBuffer))GetFunctionByName("clEnqueueCopyBuffer");
    table->EnqueueCopyImage = (decltype(table->EnqueueCopyImage))GetFunctionByName("clEnqueueCopyImage");
    table->EnqueueCopyImageToBuffer = (decltype(table->EnqueueCopyImageToBuffer))GetFunctionByName("clEnqueueCopyImageToBuffer");
    table->EnqueueCopyBufferToImage = (decltype(table->EnqueueCopyBufferToImage))GetFunctionByName("clEnqueueCopyBufferToImage");
        // OpenCL 1.1
    table->CreateUserEvent = (decltype(table->CreateUserEvent))GetFunctionByName("clCreateUserEvent");
    table->SetUserEventStatus = (decltype(table->SetUserEventStatus))GetFunctionByName("clSetUserEventStatus");
    table->SetEventCallback = (decltype(table->SetEventCallback))GetFunctionByName("clSetEventCallback");
    table->CreateSubBuffer = (decltype(table->CreateSubBuffer))GetFunctionByName("clCreateSubBuffer");
    table->SetMemObjectDestructorCallback = (decltype(table->SetMemObjectDestructorCallback))GetFunctionByName("clSetMemObjectDestructorCallback");
    table->EnqueueReadBufferRect = (decltype(table->EnqueueReadBufferRect))GetFunctionByName("clEnqueueReadBufferRect");
    table->EnqueueWriteBufferRect = (decltype(table->EnqueueWriteBufferRect))GetFunctionByName("clEnqueueWriteBufferRect");
    table->EnqueueCopyBufferRect = (decltype(table->EnqueueCopyBufferRect))GetFunctionByName("clEnqueueCopyBufferRect");

    table->CreateFromGLBuffer = (decltype(table->CreateFromGLBuffer))GetFunctionByName("clCreateFromGLBuffer");
    table->CreateFromGLTexture2D = (decltype(table->CreateFromGLTexture2D))GetFunctionByName("clCreateFromGLTexture2D");
    table->CreateFromGLTexture3D = (decltype(table->CreateFromGLTexture3D))GetFunctionByName("clCreateFromGLTexture3D");
    table->CreateFromGLRenderbuffer = (decltype(table->CreateFromGLRenderbuffer))GetFunctionByName("clCreateFromGLRenderbuffer");
    table->GetGLObjectInfo = (decltype(table->GetGLObjectInfo))GetFunctionByName("clGetGLObjectInfo");
    table->GetGLTextureInfo = (decltype(table->GetGLTextureInfo))GetFunctionByName("clGetGLTextureInfo");
    table->EnqueueAcquireGLObjects = (decltype(table->EnqueueAcquireGLObjects))GetFunctionByName("clEnqueueAcquireGLObjects");
    table->EnqueueReleaseGLObjects = (decltype(table->EnqueueReleaseGLObjects))GetFunctionByName("clEnqueueReleaseGLObjects");
    table->GetGLContextInfoKHR = (decltype(table->GetGLContextInfoKHR))GetFunctionByName("clGetGLContextInfoKHR");
        // clCreateEventFromGLsyncKHR is not an addition to OpenCL 1.2, but the
        // dispatch table in pre 1.2 versions did not contain an entry for this API
    table->CreateEventFromGLsyncKHR = (decltype(table->CreateEventFromGLsyncKHR))GetFunctionByName("clCreateEventFromGLsyncKHR");
    table->GetExtensionFunctionAddress = (decltype(table->GetExtensionFunctionAddress))GetFunctionByName("clGetExtensionFunctionAddress");

//    table->_reservedForDeviceFissionEXT[0] = (decltype(table->_reservedForDeviceFissionEXT[0]))(void*)GetFunctionByName("clCreateSubDevicesEXT");
//    table->_reservedForDeviceFissionEXT[1] = (decltype(table->_reservedForDeviceFissionEXT[1]))(void*)GetFunctionByName("clRetainDeviceEXT");
//    table->_reservedForDeviceFissionEXT[2] = (decltype(table->_reservedForDeviceFissionEXT[2]))(void*)GetFunctionByName("clReleaseDeviceEXT");
#ifdef _WIN32
// CF:LPGPU2 disabling these entries for now until we can test properly on Windows...
    // table->_reservedForD3D10KHR[0] = (decltype(table->_reservedForD3D10KHR[0]))GetFunctionByName("clGetDeviceIDsFromD3D10KHR");
    // table->_reservedForD3D10KHR[1] = (decltype(table->_reservedForD3D10KHR[1]))GetFunctionByName("clCreateFromD3D10BufferKHR");
    // table->_reservedForD3D10KHR[2] = (decltype(table->_reservedForD3D10KHR[2]))GetFunctionByName("clCreateFromD3D10Texture2DKHR");
    // table->_reservedForD3D10KHR[3] = (decltype(table->_reservedForD3D10KHR[3]))GetFunctionByName("clCreateFromD3D10Texture3DKHR");
    // table->_reservedForD3D10KHR[4] = (decltype(table->_reservedForD3D10KHR[4]))GetFunctionByName("clEnqueueAcquireD3D10ObjectsKHR");
    // table->_reservedForD3D10KHR[5] = (decltype(table->_reservedForD3D10KHR[5]))GetFunctionByName("clEnqueueReleaseD3D10ObjectsKHR");
#endif
        // OpenCL 1.2
    table->CreateSubDevices = (decltype(table->CreateSubDevices))GetFunctionByName("clCreateSubDevices");
    table->RetainDevice = (decltype(table->RetainDevice))GetFunctionByName("clRetainDevice");
    table->ReleaseDevice = (decltype(table->ReleaseDevice))GetFunctionByName("clReleaseDevice");
    table->CreateImage = (decltype(table->CreateImage))GetFunctionByName("clCreateImage");
    table->CreateProgramWithBuiltInKernels = (decltype(table->CreateProgramWithBuiltInKernels))GetFunctionByName("clCreateProgramWithBuiltInKernels");
    table->CompileProgram = (decltype(table->CompileProgram))GetFunctionByName("clCompileProgram");
    table->LinkProgram = (decltype(table->LinkProgram))GetFunctionByName("clLinkProgram");
    table->UnloadPlatformCompiler = (decltype(table->UnloadPlatformCompiler))GetFunctionByName("clUnloadPlatformCompiler");
    table->GetKernelArgInfo = (decltype(table->GetKernelArgInfo))GetFunctionByName("clGetKernelArgInfo");
    table->EnqueueFillBuffer = (decltype(table->EnqueueFillBuffer))GetFunctionByName("clEnqueueFillBuffer");
    table->EnqueueFillImage = (decltype(table->EnqueueFillImage))GetFunctionByName("clEnqueueFillImage");
    table->EnqueueMigrateMemObjects = (decltype(table->EnqueueMigrateMemObjects))GetFunctionByName("clEnqueueMigrateMemObjects");
    table->EnqueueMarkerWithWaitList = (decltype(table->EnqueueMarkerWithWaitList))GetFunctionByName("clEnqueueMarkerWithWaitList");
    table->EnqueueBarrierWithWaitList = (decltype(table->EnqueueBarrierWithWaitList))GetFunctionByName("clEnqueueBarrierWithWaitList");
    table->GetExtensionFunctionAddressForPlatform = (decltype(table->GetExtensionFunctionAddressForPlatform))GetFunctionByName("clGetExtensionFunctionAddressForPlatform");
    table->CreateFromGLTexture = (decltype(table->CreateFromGLTexture))GetFunctionByName("clCreateFromGLTexture");

    /*
    table->_reservedD3DExtensions[0] = GetFunctionByName("cl_reservedD3DExtensions[0]");
    table->_reservedD3DExtensions[1] = GetFunctionByName("cl_reservedD3DExtensions[1]");
    table->_reservedD3DExtensions[2] = GetFunctionByName("cl_reservedD3DExtensions[2]");
    table->_reservedD3DExtensions[3] = GetFunctionByName("cl_reservedD3DExtensions[3]");
    table->_reservedD3DExtensions[4] = GetFunctionByName("cl_reservedD3DExtensions[4]");
    table->_reservedD3DExtensions[5] = GetFunctionByName("cl_reservedD3DExtensions[5]");
    table->_reservedD3DExtensions[6] = GetFunctionByName("cl_reservedD3DExtensions[6]");
    table->_reservedD3DExtensions[7] = GetFunctionByName("cl_reservedD3DExtensions[7]");
    table->_reservedD3DExtensions[8] = GetFunctionByName("cl_reservedD3DExtensions[8]");
    table->_reservedD3DExtensions[9] = GetFunctionByName("cl_reservedD3DExtensions[9]");

    table->_reservedEGLExtensions[0] = GetFunctionByName("cl_reservedEGLExtensions[0]");
    table->_reservedEGLExtensions[1] = GetFunctionByName("cl_reservedEGLExtensions[1]");
    table->_reservedEGLExtensions[2] = GetFunctionByName("cl_reservedEGLExtensions[2]");
    table->_reservedEGLExtensions[3] = GetFunctionByName("cl_reservedEGLExtensions[3]");
    */

    table->CreateCommandQueueWithProperties = (decltype(table->CreateCommandQueueWithProperties))GetFunctionByName("clCreateCommandQueueWithProperties");
    table->CreatePipe = (decltype(table->CreatePipe))GetFunctionByName("clCreatePipe");
    table->GetPipeInfo = (decltype(table->GetPipeInfo))GetFunctionByName("clGetPipeInfo");
    table->SVMAlloc = (decltype(table->SVMAlloc))GetFunctionByName("clSVMAlloc");
    table->SVMFree = (decltype(table->SVMFree))GetFunctionByName("clSVMFree");
    table->EnqueueSVMFree = (decltype(table->EnqueueSVMFree))GetFunctionByName("clEnqueueSVMFree");
    table->EnqueueSVMMemcpy = (decltype(table->EnqueueSVMMemcpy))GetFunctionByName("clEnqueueSVMMemcpy");
    table->EnqueueSVMMemFill = (decltype(table->EnqueueSVMMemFill))GetFunctionByName("clEnqueueSVMMemFill");
    table->EnqueueSVMMap = (decltype(table->EnqueueSVMMap))GetFunctionByName("clEnqueueSVMMap");
    table->EnqueueSVMUnmap = (decltype(table->EnqueueSVMUnmap))GetFunctionByName("clEnqueueSVMUnmap");
    table->CreateSamplerWithProperties = (decltype(table->CreateSamplerWithProperties))GetFunctionByName("clCreateSamplerWithProperties");
    table->SetKernelArgSVMPointer = (decltype(table->SetKernelArgSVMPointer))GetFunctionByName("clSetKernelArgSVMPointer");
    table->SetKernelExecInfo = (decltype(table->SetKernelExecInfo))GetFunctionByName("clSetKernelExecInfo");
}

// NOTE: In order for stack tracing to work correctly in a release build, clAgent_OnLoad must be the last function exported from CLTraceAgent.dll
// See Windows implementation of CLAPIBase::CreateStackEntry(), which expects clAgent_OnLoad to be in the stack trace
cl_int CL_CALLBACK
clAgent_OnLoad(cl_agent* agent)
{
  SP_UNREFERENCED_PARAMETER(agent);
    //CF:LPGPU2 - We have disabled this entry point for now, which is the original AMD-specific implementation.
    //            This function is a callback that is called by the AMD OCL driver when ready. It can maybe
    //            be re-enabled once it's clear how it would interact with our own changes, although this
    //            would require the 'next' parameter added to all the CLAPITraceEntry functions to maybe be
    //            macro'd out? It could also be the case that the AMD-specific version doesn't offer much
    //            additional functionality over our agnostic version now.
/*
#ifdef _WIN32

#ifdef _DEBUG
    FileUtils::CheckForDebuggerAttach();
#endif

     std::cout << "CodeXL GPU Profiler " << GPUPROFILER_BACKEND_VERSION_STRING << " is Enabled\n";

     cl_int err = agent->GetICDDispatchTable(
         agent, &original_dispatch, sizeof(original_dispatch));

     if (err != CL_SUCCESS)
     {
       return err;
     }

    memcpy(&modified_dispatch, &original_dispatch, sizeof(modified_dispatch));

    InitNextCLFunctions(original_dispatch);

    Parameters params;
    FileUtils::GetParametersFromFile(params);


    if (params.m_bTestMode)
    {
        AddVectoredExceptionHandler(1, &TestModeExceptionHandler);
    }

    if (params.m_bStartDisabled)
    {
        CLAPIInfoManager::Instance()->StopTracing();
    }

    OSUtils::Instance()->SetupUserTimer(params);
    StackTracer::Instance()->InitSymPath();
    CLAPIInfoManager::Instance()->SetOutputFile(params.m_strOutputFile);
    GlobalSettings::GetInstance()->m_params = params;

    SetGlobalTraceFlags(params.m_bQueryRetStat, params.m_bCollapseClGetEventInfo);

    if (!params.m_strAPIFilterFile.empty())
    {
        CLAPIInfoManager::Instance()->LoadAPIFilterFile(params.m_strAPIFilterFile);
    }

    std::string strLogFile = FileUtils::GetDefaultOutputPath() + "cltraceagent.log";
    LogFileInitialize(strLogFile.c_str());

    CreateAPITraceDispatchTable(modified_dispatch);

    err = agent->SetICDDispatchTable(
      agent, &modified_dispatch, sizeof(modified_dispatch));

    // Set Event callback
    cl_agent_callbacks callbacks;
    memset(&callbacks, '\0', sizeof(callbacks));
    CreateCLEventCallbackDispatchTable(callbacks);
    agent->SetCallbacks(agent, &callbacks, sizeof(callbacks));

    cl_agent_capabilities caps;
    memset(&caps, '\0', sizeof(caps));
    caps.canGenerateEventEvents = 1;
    agent->SetCapabilities(agent, &caps, CL_AGENT_ADD_CAPABILITIES);

    if (params.m_bTimeOutBasedOutput)
    {
        CLAPIInfoManager::Instance()->SetInterval(params.m_uiTimeOutInterval);
        CLEventManager::Instance()->SetTimeOutMode(true);

        if (!CLAPIInfoManager::Instance()->StartTimer(TimerThread))
        {
            std::cout << "Failed to initialize CLTraceAgent." << std::endl;
        }
    }

    if (params.m_bUserPMC)
    {
        PMCSamplerManager::Instance()->LoadPMCSamplers(params.m_strUserPMCLibPath.c_str());
    }

#endif //_WIN32
*/
    return CL_SUCCESS;
}

so_constructor void CLTraceAgentInit()
{
    if (bAlreadyInit)
    {
        return;
    }
    bAlreadyInit = true;
#ifdef _DEBUG
    FileUtils::CheckForDebuggerAttach();
#endif

//++CF:LPGPU2 Here we append the current PID to the temp file logging the IDs of all profiled processes. This is so the backend can
//            can choose which file(s) to actually merge into the ATP file.
    auto pidOutPath = FileUtils::GetDefaultOutputPath();
    pidOutPath.append("/cxlpidlog");
    std::ofstream pidOutFile{pidOutPath.c_str(), std::ios::app};
    pidOutFile << osGetCurrentProcessId() << std::endl;
//--CF:LPGPU2

    // CF:LPGPU2 TODO here we could test to see if an AMD specific module (libamdocl64.so etc)
    // is loaded and if so use the original methods to load the ICD dispatch table rather than
    // the detours.

#ifdef _WIN32
    bool bOK = osGetLoadedModuleHandle(osFilePath{L"OpenCL.dll"}, cl_module_handle);
    if (!bOK)
    {
        std::cerr << "Error: Target program does not have OpenCL library loaded." << std::endl;
        return;
    }
#endif //_WIN32
    GetDispatchTable(&original_dispatch);


    memcpy(&modified_dispatch, &original_dispatch, sizeof(modified_dispatch));
    CreateAPITraceDispatchTable(modified_dispatch);

    InitNextCLFunctions(original_dispatch);

    //++CF:LPGPU2 init detour bypass functions
    original_dispatch.GetEventInfo = (decltype(original_dispatch.GetEventInfo)) GetFunctionByName("detour_bypass_clGetEventInfo");
    original_dispatch.GetContextInfo = (decltype(original_dispatch.GetContextInfo)) GetFunctionByName("detour_bypass_clGetContextInfo");
    original_dispatch.GetDeviceInfo = (decltype(original_dispatch.GetDeviceInfo)) GetFunctionByName("detour_bypass_clGetDeviceInfo");
    original_dispatch.GetCommandQueueInfo = (decltype(original_dispatch.GetCommandQueueInfo)) GetFunctionByName("detour_bypass_clGetCommandQueueInfo");
    original_dispatch.GetKernelInfo = (decltype(original_dispatch.GetKernelInfo)) GetFunctionByName("detour_bypass_clGetKernelInfo");
    original_dispatch.GetKernelArgInfo = (decltype(original_dispatch.GetKernelArgInfo)) GetFunctionByName("detour_bypass_clGetKernelArgInfo");
    original_dispatch.GetKernelWorkGroupInfo = (decltype(original_dispatch.GetKernelWorkGroupInfo)) GetFunctionByName("detour_bypass_clGetKernelWorkGroupInfo");
    original_dispatch.ReleaseEvent = (decltype(original_dispatch.ReleaseEvent)) GetFunctionByName("detour_bypass_clReleaseEvent");
    original_dispatch.RetainEvent = (decltype(original_dispatch.RetainEvent)) GetFunctionByName("detour_bypass_clRetainEvent");
    original_dispatch.GetEventProfilingInfo = (decltype(original_dispatch.GetEventProfilingInfo)) GetFunctionByName("detour_bypass_clGetEventProfilingInfo");
    original_dispatch.GetImageInfo = (decltype(original_dispatch.GetImageInfo)) GetFunctionByName("detour_bypass_clImageInfo");
    //--CF:LPGPU2

    Parameters params;
    FileUtils::GetParametersFromFile(params);

#ifdef _WIN32

    if (params.m_bTestMode)
    {
        AddVectoredExceptionHandler(1, &TestModeExceptionHandler);
    }

#endif //_WIN32
    if (params.m_bStartDisabled)
    {
        CLAPIInfoManager::Instance()->StopTracing();
    }

    OSUtils::Instance()->SetupUserTimer(params);
    StackTracer::Instance()->InitSymPath();
    CLAPIInfoManager::Instance()->SetOutputFile(params.m_strOutputFile);
    GlobalSettings::GetInstance()->m_params = params;

    SetGlobalTraceFlags(params.m_bQueryRetStat, params.m_bCollapseClGetEventInfo);

    if (!params.m_strAPIFilterFile.empty())
    {
        CLAPIInfoManager::Instance()->LoadAPIFilterFile(params.m_strAPIFilterFile);
    }

    std::string strLogFile = FileUtils::GetDefaultOutputPath() + "cltraceagent.log";
    LogFileInitialize(strLogFile.c_str());

    if (params.m_bTimeOutBasedOutput)
    {
        CLAPIInfoManager::Instance()->SetInterval(params.m_uiTimeOutInterval);
        CLEventManager::Instance()->SetTimeOutMode(true);

        if (!CLAPIInfoManager::Instance()->StartTimer(TimerThread))
        {
            std::cout << "Failed to initialize CLTraceAgent." << std::endl;
        }
    }

    if (params.m_bUserPMC)
    {
        PMCSamplerManager::Instance()->LoadPMCSamplers(params.m_strUserPMCLibPath.c_str());
    }
}

so_destructor void CLTraceAgentDeinit()
{
    DumpTrace();
}

// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

/// @file
///
/// @brief  LPGPU2Feedback engine is an add on to LPGPU2Database Layer that
///         allows execution of custom operations on the database regarding
///         crunching of Android Profiling data.
///
/// Defines the class LPGPU2FeedbackEnginePreProcessor for including in the
/// database the data required for the actual feedback analysis.
///
/// @copyright
/// Copyright (C) 2002-2018 Codeplay Software Limited. All Rights Reserved.

// Local:
#include <LPGPU2Database/LPGPU2FeedbackEngine/LPGPU2_db_FeedbackEnginePreProcessor.h>

#define FRAME_DELIMITER_CATEGORY L"FrameDelimiter"
#define DRAW_CATEGORY L"Draw"
#define BIND_CATEGORY L"Bind"
#define WAIT_CATEGORY L"Wait"
#define SETUP_CATEGORY L"Setup"
#define ERROR_CATEGORY L"Error"
#define COMPILE_CATEGORY L"Compile"
#define COMPUTE_CATEGORY L"Compute"
#define DATA_TRANSFER_CATEGORY L"DataTransfer"
#define STATE_CATEGORY L"State"
#define FRAME_CATEGORY L"Frame"
#define TEXTURE_CATEGORY L"Texture"
#define PIPELINE_CATEGORY L"Pipeline"
#define DESCRIPTOR_CATEGORY L"Descriptor"
#define BUFFER_CATEGORY L"Buffer"
#define MEMORY_ALLOCATION_CATEGORY L"MemoryAllocation"
#define BLOCK_CATEGORY L"Block"
#define SHADER_CATEGORY L"Shader"
#define OTHER_CATEGORY L"Other"

#define OPENCL_API L"OpenCL"
#define OPENGL_API L"OpenGL"
#define VULKAN_API L"Vulkan"
#define OPENGLES_API L"OpenGLES"

#define CPU_TEMP_CATEGORY L"CPU_TEMP"
#define GPU_TEMP L"GPU_TEMP"
#define CPU_LOAD L"CPU_LOAD"
#define GPU_LOAD L"GPU_LOAD"
#define FPS L"FPS"
#define GPU_FREQ L"GPU_FREQ"
#define CPU_FREQ L"CPU_FREQ"
#define POWER L"POWER"
#define FB_STALLS L"FB_STALLS"
#define TOTAL_CYCLES L"TOTAL_CYCLES"
#define TEX_STALLS L"TEX_STALLS"
#define NUM_INST L"NUM_INST"
#define NUM_PIXELS L"NUM_PIXELS"
#define ACTIVITY L"ACTIVITY"

std::vector<std::string> openclFrameDelimiters
{
    "clFinish",
    "clFlush"
};


std::vector<std::string> vulkanFrameDelimiters
{
    "vkQueuePresentKHR"
};

std::vector<std::string> eglFrameDelimiters
{
    "eglSwapBuffers"
};

std::vector<std::string> openglFrameDelimiters
{
    "glClear",
    "glFlush",
    "glFinish"
};

std::vector<std::string> openclDrawTypeCalls
{
    "clEnqueueNDRangeKernel",
    "clEnqueueNativeKernel"
};

std::vector<std::string> vulkanDrawTypeCalls
{
    "vkCmdExecuteCommands",
    "vkCmdBeginRenderPass",
    "vkCmdNextSubpass",
    "vkCmdEndRenderPass",
    "vkCmdClearColorImage",
    "vkCmdClearDepthStencilImage",
    "vkCmdClearAttachments",
    "vkCmdDraw",
    "vkCmdDrawIndexed",
    "vkCmdDrawIndirect",
    "vkCmdDrawIndexedIndirect"
};

std::vector<std::string> openglDrawTypeCalls
{
    "glClear",
    "glDrawArrays",
    "glDrawArraysInstancedBaseInstance",
    "glDrawArraysInstanced",
    "glDrawArraysIndirect",
    "glMultiDrawArrays",
    "glMultiDrawArraysIndirect",
    "glDrawElements",
    "glDrawElementsInstancedBaseInstance",
    "glDrawElementsInstanced",
    "glMultiDrawElements",
    "glDrawRangeElements",
    "glDrawElementsBaseVertex",
    "glDrawRangeElementsBaseVertex",
    "glDrawElementsInstancedBaseVertex",
    "glDrawElementsInstancedBase-VertexBaseInstance",
    "glDrawElementsIndirect",
    "glMultiDrawElementsIndirect",
    "glDrawElementsBaseVertex",
    "glDrawTransformFeedback",
    "glDrawTransformFeedbackInstanced",
    "glDrawTransformStream",
    "glDrawTransformStreamInstanced",
    "glDrawBuffer",
    "glDrawBuffers",
    "glDispatchCompute",
    "glDispatchComputeIndirect",
    "glNamedFramebufferDrawBuffers",
    "glClearColor",
    "glClearDepth",
    "glClearDepthf",
    "glClearStencil",
    "glClearBufferiv",
    "glClearBufferfv",
    "glClearBufferuiv",
    "glClearNamedFramebufferiv",
    "glClearNamedFramebufferfv",
    "glClearNamedFramebufferuiv",
    "glClearBufferfi",
    "glClearNamedFramebufferfi"
};

std::vector<std::string> openclBindTypeCalls
{
    "clSetKernelArg",
    "clSetKernelArgSVMPointer",
    "clEnqueueReadBuffer",
    "clEnqueueReadBufferRect",
    "clEnqueueWriteBuffer",
    "clEnqueueWriteBufferRect",
    "clEnqueueFillBuffer",
    "clEnqueueCopyBuffer",
    "clEnqueueCopyBufferRect",
    "clEnqueueMapBuffer",
    "clEnqueueUnmapMemObject",
    "clEnqueueMigrateMemObjects",
    "clEnqueueSVMFree",
    "clEnqueueSVMMemcpy",
    "clEnqueueSVMMemFill",
    "clEnqueueSVMMap",
    "clEnqueueSVMUnmap",
    "clSetEventCallback",
    "clEnqueueReadImage",
    "clEnqueueWriteImage",
    "clEnqueueFillImage",
    "clEnqueueCopyImage",
    "clEnqueueCopyImageToBuffer",
    "clEnqueueCopyBufferToImage",
    "clEnqueueMapImage",
    "clEnqueueAcquireGLObjects",
    "clEnqueueReleaseGLObjects"
};

std::vector<std::string> vulkanBindTypeCalls
{
    "vkCmdBindPipeline",
    "vkQueueSubmit",
    "vkBindBufferMemory",
    "vkBindImageMemory",
    "vkCmdBindDescriptorSets",
    "vkCmdFillBuffer",
    "vkCmdUpdateBuffer",
    "vkCmdBindIndexBuffer",
    "vkCmdBindVertexBuffers",
    "vkCmdSetLineWidth",
    "vkCmdSetDepthBias",
    "vkCmdSetBlendConstants",
    "vkCmdSetScissor",
    "vkCmdSetDepthBounds",
    "vkCmdSetStencilCompareMask",
    "vkSetCmdSetStencilWriteMask",
    "vkCmdSetStencilReference"
};

std::vector<std::string> eglBindTypeCalls
{
    "eglBindAPI"
};

std::vector<std::string> openglBindTypeCalls
{
    "glBindFramebuffer",
    "glBindVertexArray",
    "glBindBuffer",
    "glBindBufferRange",
    "glBindBufferBase",
    "glBindBuffersRange",
    "glBindBuffersBase",
    "glBufferStorage",
    "glNamedBufferStorage",
    "glBufferData",
    "glNamedBufferData",
    "glBufferSubData",
    "glNamedBufferSubData",
    "glClearBufferData",
    "glClearNamedBufferData",
    "glClearBufferSubData",
    "glClearNamedBufferSubData",
    "glMapBuffer",
    "glMapNamedBuffer",
    "glMapNamedBufferRange"
    "glFlushMapppedBufferRange",
    "glFlushMapppedNamedBufferRange"
    "glUnmapBuffer",
    "glUnmapNamedBuffer",
    "glCopyBufferSubData",
    "glCopyNamedBufferSubData",
    "glBindTexture",
    "glBindTextures",
    "glBindSampler",
    "glBindSamplers",
    "glBindImageTexture",
    "glBindImageTextures",
    "glBindTextureUnit",
    "glBindVertexArray",
    "glFramebufferTexture",
    "glFramebufferTexture1D",
    "glFramebufferTexture2D",
    "glFramebufferTexture3D",
    "glNamedFramebufferTexture",
    "glBlitFramebuffer",
    "glAttachShader",
    "glFramebufferRenderbuffer",
    "glBindAttribLocation",
    "glBindRenderbuffer",
    "glBindVertexArray",
    "glBindVertexBuffer",
    "glBindVertexBuffers",
    "glBindAttribLocation",
    "glBindTransformFeedback",
    "glBindFragDataLocationIndex",
    "glBindFragDataLocation",
    "glReadBuffer",
    "glNamedFramebufferReadBuffer"
    "glReadPixels",
    "glReadnPixels",
    "glCopyImageSubData"
};

std::vector<std::string> openclWaitTypeCalls
{
    "clWaitForEvents"
};

std::vector<std::string> vulkanWaitTypeCalls
{
    "vkDeviceWaitIdle",
    "vkQueueWaitIdle",
    "vkWaitForFences",
    "vkCmdWaitForEvents",
    "vkCmdPipelineBarrier",
    "vkCmdPushConstants"
};

std::vector<std::string> eglWaitTypeCalls
{
    "eglWaitGL",
    "eglWaitNative"
};

std::vector<std::string> openglWaitTypeCalls
{
    "glClientWaitSync",
    "glWaitSync"
};

std::vector<std::string> openclSetupTypeCalls
{
    "clGetPlatformIDs",
    "clGetPlatformIDsKHR",
    "clGetPlatformInfo",
    "clGetDeviceIDs",
    "clGetDeviceInfo",
    "clCreateSubDevices",
    "clCreateContext",
    "clCreateContextFromType",
    "clRetainContext",
    "clReleaseContext",
    "clGetContextInfo",
    "clGetExtensionFunctionAddressForPlatform",
    "clCreateCommandQueueWithProperties",
    "clRetainCommandQueue",
    "clReleaseCommandQueue",
    "clGetCommandQueueInfo",
    "clCreateCommandQueue", //Deprecated since OpenCL 2.0
    "clCreateBuffer",
    "clCreateSubBuffer",
    "clRetainMemObject",
    "clReleaseMemObject",
    "clGetMemObjectInfo",
    "clCreatePipe",
    "clGetPipeInfo",
    "clSVMAlloc",
    "clSVMFree",
    "clCreateProgramWithSource",
    "clCreateProgramWithBinary",
    "clCreateProgramWithBuiltInKernels",
    "clRetainProgram",
    "clReleaseProgram",
    "clBuildProgram",
    "clCompileProgram",
    "clLinkProgram",
    "clUnloadPlatformCompiler",
    "clGetPrograminfo",
    "clCreateKernel",
    "clCreateKernelsInProgram",
    "clRetainKernel",
    "clReleaseKernel",
    "clSetKernelExecInfo",
    "clGetKernelInfo",
    "clGetWorkgroupKernelInfo",
    "clGetKernelArgInfo",
    "clCreateUserEvent",
    "clSetUserEventStatus",
    "clGetEventInfo",
    "clRetainEvent",
    "clReleaseEvent",
    "clEnqueueMarkerWithWaitList",
    "clEnqueueBarrierWithWaitList",
    "clCreateImage",
    "clGetSupportedImageFormats",
    "clGetImageInfo",
    "clGetMemObjectInfo",
    "clCreateSamplerWithProperties",
    "clRetainSampler",
    "clReleaseSampler",
    "clGetSamplerInfo",
    "clGetGLContextInfoKHR",
    "clCreateFromGLBuffer",
    "clCreateFromGLTexture",
    "clCreateFromGLRenderBuffer",
    "clGetGLObjectInfo",
    "clGetGLTextureInfo",
    "clCreateEventFromGLsyncKHR",
    "clGetDeviceIDsFromD3DD10KHR",
    "clCreateFromFromD3DD10BufferKHR",
    "clCreateFromFromD3DD10Texture2DKHR",
    "clCreateFromFromD3DD10Texture3DKHR",
    "clEnqueueAcquireD3D10ObjectsKHR",
    "clEnqueueReleaseD3D10ObjectsKHR",
    "clGetDeviceIDsFromDX9MediaAdapterKHR",
    "clCreateFromDX9MediaAdapterSurfaceKHR",
    "clEnqueueAcquireDX9MediaAdapterKHR",
    "clEnqueueReleaseDX9MediaAdapterKHR",
    "clGetDeviceIDsFromD3D11KHR",
    "clCreateFromD3D11BufferKHR",
    "clCreateFromD3D11Texture3DKHR",
    "clCreateFromD3D11Texture2DKHR",
    "clEnqueueAcquireD3D11ObjectsKHR",
    "clEnqueueReleaseD3D11ObjectsKHR",
    "clCreateFromEGLImageKHR",
    "clEnqueueAcquireEGLObjectsKHR",
    "clEnqueueReleaseEGLObjectsKHR",
    "clreateEventFromEGLsyncKHR"
};

std::vector<std::string> vulkanSetupTypeCalls
{
    "vkGetDeviceQueue", //Vulkan
    "vkGetDeviceQueueFamilyProperties",
    "vkGetPhysicalDeviceQueueFamilyProperties",
    "vkGetPhysicalDeviceProperties",
    "vkEnumeratePhysicalDevices",
    "vkCreateInstance",
    "vkDestroyInstance",
    "vkGetInstanceProcAddr",
    "vkGetDeviceProcAddr",
    "vkCreateDevice",
    "vkDestroyDevice",
    "vkCreateCommandPool",
    "vkResetCommandPool",
    "vkDestroyCommandPool",
    "vkAllocateCommandBuffers",
    "vkResetCommandBuffer",
    "vkFreeCommandBuffers",
    "vkBeginCommandBuffer",
    "vkEndCommandBuffer",
    "vkCreateFence",
    "vkDestroyFence",
    "vkGetFenceStatus",
    "vkResetFences",
    "vkCreateSemaphore",
    "vkDestroySemaphore",
    "vkCreateEvent",
    "vkDestroyEvent",
    "vkGetEventStatus",
    "vkSetEvent",
    "vkResetEvent",
    "vkCmdSetEvent",
    "vkCmdResetEvent",
    "vkCreateRenderPass",
    "vkDestroyRenderPass",
    "vkCreateFramebuffer",
    "vkDestroyFramebuffer",
    "vkCmdGetRenderAreaGranularity",
    "vkCreateShaderModule",
    "vkDestroyShaderModule",
    "vkCreateComputePipelines",
    "vkCreateGraphicsPipelines",
    "vkDestroyPipeline",
    "vkCreatePipelineCache",
    "vkMergePipelineCaches",
    "vkGetPipelineCacheData",
    "vkDestroyPipelineCache",
    "vkGetPhysicalDeviceMemoryProperties",
    "vkAllocateMemory",
    "vkFreeMemory",
    "vkMapMemory",
    "vkFlushMappedMemoryRanges",
    "vkInvalidateMappedMemoryRanges",
    "vkUnmapMemory",
    "vkGetDeviceMemoryCommitment",
    "vkCreateBuffer",
    "vkDestroyBuffer",
    "vkCreateBufferView",
    "vkDestroyBufferView",
    "vkCreateImage",
    "vkGetImageSubresourceLayout",
    "vkDestroyImage",
    "vkCreateImageView",
    "vkDestroyImageView",
    "vkGetBufferMemoryRequirements",
    "vkGetImageMemoryRequirements",
    "vkCreateSampler",
    "vkDestroySampler",
    "vkCreateDescriptorSetLayout",
    "vkDestroyDescriptorSetLayout",
    "vkCreatePipelineLayout",
    "vkDestroyPipelineLayout",
    "vkCreateDescriptorPool",
    "vkDestroyDescriptorPool",
    "vkAllocateDescriptorSets",
    "vkFreeDescriptorSets",
    "vkResetDescriptorPool",
    "vkUpdateDescriptorSets",
    "vkCreateQueryPool",
    "vkDestroyQueryPool",
    "vkCmdResetQueryPool",
    "vkCmdBeginQuery",
    "vkCmdEndQuery",
    "vkGetQueryPoolResults",
    "vkCmdWriteTimestamp",
    "vkCreateAndroidSurfaceKHR", //Is it correct from here?
    "vkCreateMirSurfaceKHR",
    "vkCreateWaylandSurfaceKHR",
    "vkCreateWin32SurfaceKHR",
    "vkCreateXcbSurfaceKHR",
    "vkCreateXlibSurfaceKHR",
    "vkDestroySurfaceKHR",
    "vkGetPhycicalDeviceDisplayPropertiesKHR",
    "vkGetPhysicalDeviceDisplayPlanePropertiesKHR",
    "vkGetDisplayPlaneSupportedDisplaysKHR",
    "vkGetDisplayModePropertiesKHR",
    "vkCreateDisplayModeKHR",
    "vkGetDisplayPlaneCapabilitiesKHR",
    "vkCreateDisplayPlaneSurfaceKHR",
    "vkGetPhysicalDeviceSurfaceSupportKHR",
    "vkGetPhysicalMirPresentationSupportKHR",
    "vkGetPhysicalWin32PresentationSupportKHR",
    "vkGetPhysicalDeviceWaylandPresentationSupportkHR",
    "vkGetPhysicalDeviceXcbPresentationSupportKHR",
    "vkGetPhysicalDeviceXlibPresentationSupportKHR",
    "vkGetPhysicalDeviceSurfaceCapabilitiesKHR",
    "vkGetPhysicalDeviceSurfaceFormatsKHR",
    "vkGetPhysicalDeviceSurfacePresentModesKHR",
    "vkCreateSwapchainKHR",
    "vkDestroySwapchainKHR",
    "vkCreateSharedSwapchainsKHR",
    "vkGetSwapchainImagesKHR",
    "vkAcquireNextImageKHR",
    "vkEnumerateInstanceLayerProperties",
    "vkEnumerateDeviceLayerProperties",
    "vkEnumerateInstanceExtensionProperties",
    "vkEnumerateDeviceExtensionProperties",
    "vkGetPhysicalDeviceFeatures",
    "vkGetPhysicalDeviceFormatProperties",
    "vkGetPhysicalDeviceImageFormatProperties"
};

std::vector<std::string> eglSetupTypeCalls
{
    "eglGetProcAddress",
    "eglChooseConfig",
    "eglGetConfigAttrib",
    "eglCreateWindowSurface",
    "eglInitialize",
    "eglCreateContext",
    "eglSetBlobCacheFuncsANDROID",
    "eglQueryString",
    "eglMakeCurrent",
    "eglReleaseThread",
    "eglQuerySurface",
    "eglDestroySurface",
    "eglDestroyContext"
};

std::vector<std::string> openglSetupTypeCalls
{
    "glGetGraphicsResetStatus",
    "glGetIntegerv",
    "glGetInteger64v",
    "glDeteleSync",
    "glFenceSync",
    "glGetSynciv",
    "glIsSync",
    "glGenQueries",
    "glDeleteQueries",
    "glBeginQuery",
    "glBeginQueryIndexed",
    "glEndQuery",
    "glEndQueryIndexed",
    "glIsQuery",
    "glGetQueryiv",
    "glGetQueryIndexediv",
    "glGetQueryObjectuiv",
    "glGetQueryObjecti64v",
    "glGetQueryObjectui64v",
    "glQueryCounter",
    "glGenBuffers",
    "glCreateBuffers",
    "glDeleteBuffers",
    "glIsBuffer",
    "glGetBufferSubData",
    "glGetNamedBufferSubData",
    "glGetBufferParameteriv",
    "glGetBufferParameteri64v",
    "glGetNamedBufferParameteriv",
    "glGetNamedBufferParameteri64v",
    "glGetBufferPointerv",
    "glGetNamedBufferPointerv",
    "glCreateShader",
    "glShaderSource",
    "glCompileShader",
    "glReleaseShaderCompiler",
    "glDeleteShader",
    "glIsShader",
    "glShaderBinary",
    "glCreateProgram",
    "glAttachShader",
    "glDetachShader",
    "glLinkProgram",
    "glUseProgram",
    "glCreateShaderProgramv",
    "glProgramParameteri",
    "glDeleteProgram",
    "glIsProgram",
    "glGetProgramInterfaceiv",
    "glGetProgramResourceIndex",
    "glGetProgramResourceName",
    "glGetProgramResourceiv",
    "glGetProgramResourceLocation",
    "glGetProgramResourceLocationIndex",
    "glGenProgramPipelines",
    "glDeleteProgramPipelines",
    "glIsProgramPipeline",
    "glBindProgramPipeline",
    "glCreateProgramPipelines",
    "glUseProramStages",
    "glActiveShaderProgram",
    "glGetProgramBinary",
    "glProgramBinary",
    "glGetUniformLocation",
    "glGetActiveUniformName",
    "glGetUniformIndices",
    "glgetActiveUniform",
    "glgetActiveUniformsiv",
    "glGetActiveUniformBlockIndex",
    "glGetActiveUniformBlockName",
    "glGetActiveUniformBlockiv",
    "glGetActiveAtomicCounterBufferiv",
    "glUniformBlockBinding",
    "glShaderStorageBlockBinding",
    "glGetSubroutineUniformLocation",
    "glGetSubroutineIndex",
    "glGetActiveSubroutineName",
    "glGetActiveSubroutineUniformName",
    "glGetActiveSubroutineUniformiv",
    "glUniformSubroutinesuiv",
    "glMemoryBarrier",
    "glMemoryBarrierByRegion",
    "glGetShaderiv",
    "glGetProgramiv",
    "glGetProgramPipelineiv",
    "glGetAttachedShaders",
    "glGetShaderInfoLog",
    "glGetProgramInfoLog",
    "glGetProgramPipelineInfoLog",
    "glgetShaderSource",
    "glGetShaderPrecisionFormat",
    "glGetUniformfv",
    "glGetUniformiv",
    "glGetUniformdv",
    "glGetUniformuiv",
    "glGetnUniformfv",
    "glGetnUniformiv",
    "glGetnUniformdv",
    "glGetnUniformuiv",
    "glGetUniformSubroutineuiv",
    "glGetProgramStageuiv",
    "glActiveTexture",
    "glGenTextures",
    "glCreateTextures",
    "glDeleteTextures",
    "glIsTexture",
    "glGenSamplers",
    "glCreateSamplers",
    "glSamplerParameteri",
    "glSamplerParameterf",
    "glSamplerParameteriv",
    "glSamplerParameterfv",
    "glSamplerParameterIiv",
    "glSamplerParameterIuiv",
    "glDeleteSamplers",
    "glIsSampler",
    "glGetSamplerParameteri",
    "glGetSamplerParameterf",
    "glGetSamplerParameteriv",
    "glGetSamplerParameterfv",
    "glGetSamplerParameterIiv",
    "glGetSamplerParameterIuiv",
    "glPixelStorei",
    "glPixelStoref",
    "glTexImage3D",             //?? NOT SURE THESE ARE SETUP FUNCTIONS (TEX)
    "glTexImage2D",
    "glTexImage1D",
    "glCopyTexImage2D",
    "glCopyTexImage1D",
    "glTexSubImage3D",          //??
    "glTexSubImage2D",
    "glTexSubImage1D",
    "glCopyTexSubImage3D",
    "glCopyTexSubImage2D",
    "glCopyTexSubImage1D",
    "glTextureSubImage3D",      //??
    "glTextureSubImage2D",
    "glTextureSubImage1D",
    "glCopyTextureSubImage3D",      //??
    "glCopyTextureSubImage2D",
    "glCopyTextureSubImage1D",
    "glCompressedTexImage3D",             //?? NOT SURE THESE ARE SETUP FUNCTIONS (TEX)
    "glCompressedTexImage2D",
    "glCompressedTexImage1D",
    "glCompressedTexSubImage3D",          //??
    "glCompressedTexSubImage2D",
    "glCompressedTexSubImage1D",
    "glCompressedTextureSubImage3D",      //??
    "glCompressedTextureSubImage2D",
    "glCompressedTextureSubImage1D",
    "glTexImage3DMultisample",
    "glTexImage2DMultisample",
    "glTexBufferRange",
    "glTextureBufferRange",
    "glTexBuffer",
    "glTextureBuffer",
    "glTexParameteri",
    "glTexParameterf",
    "glTexParameteriv",
    "glTexParameterfv",
    "glTexParameterliv",
    "glTexParameterluiv",
    "glTextureParameterf",
    "glTextureParameteri",
    "glTextureParameterfv",
    "glTextureParameteriv",
    "glTextureParameterliv",
    "glTextureParameterluiv",
    "glGetTexParameteriv",
    "glGetTexParameterfv",
    "glGetTexParameterliv",
    "glGetTexParameterluiv",
    "glGetTextureParameterfv",
    "glGetTextureParameteriv",
    "glGetTextureParameterliv",
    "glGetTextureParameterluiv",
    "glGetTexLevelParameteriv",
    "glGetTexLevelParameterfv",
    "glGetTexLevelParameterliv",
    "glGetTexLevelParameterluiv",
    "glGetTexImage",
    "glGetTextureImage",
    "glGetTexImage",
    "glGetTextureSubImage",
    "glGetCompressedTexImage",
    "glGetCompressedTextureImage",
    "glGetnCompressedTexImage",
    "glGetCompressedTextureSubImage",
    "glEnable",
    "glDisable",
    "glIsEnabled",
    "glGenerateMipmap",         //??
    "glGenerateTextureMipmap"   //??
    "glTextureView",
    "glTexStorage1D",
    "glTexStorage2D",
    "glTexStorage3D",
    "glTextureStorage1D",
    "glTextureStorage2D",
    "glTextureStorage3D",
    "glTexStorage2DMultisample",
    "glTexStorage2DMultisample",
    "glTextureStorage3DMultisample",
    "glTextureStorage3DMultisample",
    "glInvalidateTexSubImage",
    "glInvalidateTexImage",
    "glClearTexSubImage",
    "glClearTexImage",
    "glCreateFramebuffers",
    "glGenFramebuffers",
    "glDeleteFramebuffers",
    "glIsFrameBuffer",
    "glFramebufferParameteri",
    "glNamedFramebufferParameteri",
    "glGetFramebufferParameteriv",
    "glGetNamedFramebufferParameteriv",
    "glGetFramebufferAttachmentParameteriv",
    "glGetNamedFramebufferAttachmentParameteriv",
    "glGenRenderbuffers",
    "glCreateRenderbuffers",
    "glDeleteRenderbuffers",
    "glIsRenderbuffer",
    "glRenderbufferStorageMultisample",
    "glNamedRenderbufferStorageMultisample",
    "glRenderbufferStorage",
    "glNamedRenderbufferStorage",
    "glGetRenderbufferParameteriv",
    "glGetNamedRenderbufferParameteriv",
    "glFramebufferRenderbuffer",
    "glNamedFramebufferRenderbuffer",
    "glFrameBufferTexture",
    "glNamedFramebufferTexture",
    "glFrameBufferTexture1D",
    "glFrameBufferTexture2D",
    "glFrameBufferTexture3D",
    "glFrameBufferTextureLayer",
    "glNamedFrameBufferTextureLayer",
    "glTextureBarrier",
    "glCheckFramebufferStatus",
    "glCheckNamedFramebufferStatus",
    "glPatchParameteri",
    "glVertexAttrib1s",
    "glVertexAttrib2s",
    "glVertexAttrib3s",
    "glVertexAttrib4s",
    "glVertexAttrib1f",
    "glVertexAttrib2f",
    "glVertexAttrib3f",
    "glVertexAttrib4f",
    "glVertexAttrib1d",
    "glVertexAttrib2d",
    "glVertexAttrib3d",
    "glVertexAttrib4d",
    "glVertexAttrib1sv",
    "glVertexAttrib2sv",
    "glVertexAttrib3sv",
    "glVertexAttrib4sv",
    "glVertexAttrib1fv",
    "glVertexAttrib2fv",
    "glVertexAttrib3fv",
    "glVertexAttrib4fv",
    "glVertexAttrib1dv",
    "glVertexAttrib2dv",
    "glVertexAttrib3dv",
    "glVertexAttrib4dv",
    "glVertexAttrib4bv",
    "glVertexAttrib4sv",
    "glVertexAttrib4iv",
    "glVertexAttrib4fv",
    "glVertexAttrib4dv",
    "glVertexAttrib4ubv",
    "glVertexAttrib4usv",
    "glVertexAttrib4uiv",
    "glVertexAttrib4b",
    "glVertexattribNub",
    "glVertexAttrib4Nbv",
    "glVertexAttrib4Nsv",
    "glVertexAttrib4Niv",
    "glVertexAttrib4Nfv",
    "glVertexAttrib4Ndv",
    "glVertexAttrib4Nubv",
    "glVertexAttrib4Nusv",
    "glVertexAttrib4Nuiv",
    "glVertexAttribI1i",
    "glVertexAttribI2i",
    "glVertexAttribI3i",
    "glVertexAttribI4i",
    "glVertexAttribI1ui",
    "glVertexAttribI2ui",
    "glVertexAttribI3ui",
    "glVertexAttribI4ui",
    "glVertexAttribI1iv",
    "glVertexAttribI2iv",
    "glVertexAttribI3iv",
    "glVertexAttribI4iv",
    "glVertexAttribI1uiv",
    "glVertexAttribI2uiv",
    "glVertexAttribI3uiv",
    "glVertexAttribI4uiv",
    "glVertexAttribI4bv",
    "glVertexAttribI4sv",
    "glVertexAttribI4ubv",
    "glVertexAttribI4usv",
    "glVertexAttribL1d",
    "glVertexAttribL2d",
    "glVertexAttribL3d",
    "glVertexAttribL4d",
    "glVertexAttribL1dv",
    "glVertexAttribL2dv",
    "glVertexAttribL3dv",
    "glVertexAttribL4dv",
    "glVertexAttribP1ui",
    "glVertexAttribP2ui",
    "glVertexAttribP3ui",
    "glVertexAttribP4ui",
    "glVertexAttribP1uiv",
    "glVertexAttribP2uiv",
    "glVertexAttribP3uiv",
    "glVertexAttribP4uiv",
    "glGenVertexArrays",
    "glDeleteVertexArrays",
    "glCreateVertexArrays",
    "glIsVertexArray",
    "glVertexArrayElementBuffer",
    "glVertexAttribFormat",
    "glVertexAttribIFormat",
    "glVertexAttribLFormat",
    "glVertexArrayAttribFormat",
    "glVertexArrayAttribIFormat",
    "glVertexArrayAttribLFormat",
    "glVertexArrayVertexBuffer",
    "glVertexArrayVertexBuffers",
    "glVertexAttribBinding",
    "glVertexArrayAttribBinding",
    "glVertexAttribPointer",
    "glVertexAttribIPointer",
    "glVertexAttribLPointer",
    "glEnableVertexAttribArray",
    "glEnableVertexArrayAttrib",
    "glDisableVertexAttribArray",
    "glDisableVertexArrayAttrib",
    "glVertexBindingDivisor",
    "glVertexArrayBindingDivisor",
    "glVertexAttribDivisor",
    "glPrimitiveRestartIndex",
    "glGetVertexArrayiv",
    "glGetArrayIndexdiv",
    "glGetVertexArrayIndexed64iv",
    "glGetVertexAttribdv",
    "glGetVertexAttribfv",
    "glGetVertexAttribiv",
    "glGetVertexAttribIiv",
    "glGetVertexAttribIuiv",
    "glGetVertexAttribLdv",
    "glGetVertexAttribPointerv",
    "glBeginConditionalRender",
    "glEndConditionalRender",
    "glGetActiveAttrib",
    "glGetAttribLocation",
    "glTransformFeedbackVaryings",
    "glGetTransformFeedbackVarying",
    "glValidateProgram",
    "glValidateProgramPipeline",
    "glPatchParameterfv",
    "glGenTransformFeedbacks",
    "glDeleteTransformFeedbacks",
    "glIsTransformFeedback",
    "glCreateTransformFeedbacks",
    "glBeginTransformFeedback",
    "glEndTransfromFeedback",
    "glPauseTransformFeedback",
    "glResumeTransformFeedback",
    "glTransformBufferFeedbackBufferRange",
    "glTransformFeedbackBufferBase",
    "glProvokingVertex",
    "glClipControl",
    "glDepthRangeArrayv",
    "glDepthRangeIndexed",
    "glDepthRange",
    "glDepthRangef",
    "glViewportArrayv",
    "glViewportIndexedf",
    "glViewportIndexedfv",
    "glViewport",
    "glGetMultisamplefv",
    "glMinSampleShading",
    "glPointSize",
    "glPointParameteri",
    "glPointParameterf",
    "glPointParameteriv",
    "glPointParameterfv",
    "glLineWidth",
    "glFrontFace",
    "glCullFace",
    "glPolygonMode",
    "glPolygonOffset",
    "glGetFragDataLocation",
    "glGetFragDataIndex",
    "glEnablei",
    "glDisablei",
    "glIsEnabledi",
    "glScissorArrayv",
    "glScissorIndexed",
    "glScissorIndexedv",
    "glScissor",
    "glSampleCoverage",
    "glSampleMaski",
    "glStencilFunc",
    "glStencilFuncSeparate",
    "glStencilOp",
    "glStencilOpSeparate",
    "glDepthFunc",
    "glBeginQuery",
    "glEndQuery",
    "glBlendEquation",
    "glBlendEquationSeparate",
    "glBlendEquationi",
    "glBlendEquationSeparatei",
    "glBlendFunc",
    "glBlendFuncSeparate",
    "glBlendFunci",
    "glBlendFuncSeparatei",
    "glBlendColor",
    "glLogicOp",
    "glHint",
    "glNamedFramebufferDrawBuffer",
    "glColorMask",
    "glColorMaski",
    "glDepthMask",
    "glStencilMask",
    "glStencilMaskSeparate",
    "glInvalidateSubFramebuffer",
    "glInvalidateNamedFramebuffersubdata",
    "glInvalidateFramebuffer",
    "glInvalidateNamedFramebufferData",
    "glClampColor",
    "glBlitFramebuffer",
    "glBlitNamedFramebuffer",
    "glGetBooleanv",
    "glGetFloatv",
    "glGetDoublev",
    "glGetDoublei_v",
    "glGetBooleani_v",
    "glgetIntegeri_v"
    "glGetFloati_v",
    "glGetDoublei_v",
    "glgetInteger64i_v"
    "glGetPointerv",
    "glGetString",
    "glGetStringi",
    "glGetInternalformativ",
    "glGetInternalformati64v",
    "glGetTransformFeedbackiv",
    "glGetTransformFeedbacki_v",
    "glGetTransformFeedbacki64_v",

    "glInvalidateFramebuffer",

    "glGetFixedv"
};

std::vector<std::string> openclErrorTypeCalls
{
    "clGetEventProfilingInfo",
    "clGetProgramBuildInfo"
};

std::vector<std::string> vulkanErrorTypeCalls
{
    "vkCreateDebugReportCallbackEXT"
};

std::vector<std::string> eglErrorTypeCalls
{
    "eglGetError"
};

std::vector<std::string> openglErrorTypeCalls
{
    "glGetError",
    "glDebugMessageCallback",
    "glDebugMessageControl",
    "glPushDebugControl",
    "glDebugMessageInsert",
    "glPushDebugGroup",
    "glPopDebugGroup",
    "glObjectLabel",
    "glObjectPtrLabel",
    "glGetDebugMessageLog",
    "glGetObjectLabel",
    "glGetObjectPtrLabel"
};


std::vector<std::string> openclCompileFunctions
{
    "clCreateProgramWithSource",
    "clCreateProgramWithBinary",
    "clCreateProgramWithBuiltInKernels",
    "clRetainProgram",
    "clBuildProgram",
    "clCompileProgram",
    "clLinkProgram",
    "clCreateKernel",
    "clCreateKernelsInProgram",
    "clSetKernelArg",
    "clSetKernelArgSVMPointer"
};

std::vector<std::string> openclComputeFunctions
{
    "clEnqueueNDRangeKernel",
    "clEnqueueNativeKernel",
    "clSetEventCallback",
    "clFinish",
    "clFlush"
};

std::vector<std::string> openclMemAllocationAndSetupFunctions
{
    "clCreateBuffer",
    "clCreateSubBuffer",
    "clRetainMemObject",
    "clReleaseMemObject",
    "clGetMemObjectInfo",
    "clCreatePipe",
    "clGetPipeInfo",
    "clSVMAlloc",
    "clSVMFree",
    "clSetKernelArg",
    "clSetKernelArgSVMPointer"
    "clCreateImage",
    "clGetSupportedImageFormats",
    "clGetImageInfo"
};

std::vector<std::string> openclDataTransferFunctions
{
    "clEnqueueReadBuffer",
    "clEnqueueReadBufferRect",
    "clEnqueueWriteBuffer",
    "clEnqueueWriteBufferRect",
    "clEnqueueFillBuffer",
    "clEnqueueCopyBuffer",
    "clEnqueueCopyBufferRect",
    "clEnqueueMapBuffer",
    //"clEnqueueUnmapMemObject",
    //"clEnqueueMigrateMemObjects",
    //"clEnqueueSVMFree",
    "clEnqueueSVMMemcpy",
    "clEnqueueSVMMemFill",
    "clEnqueueSVMMap",
    //"clEnqueueSVMUnmap",
    "clEnqueueReadImage",
    "clEnqueueWriteImage",
    "clEnqueueFillImage",
    "clEnqueueCopyImage",
    //"clEnqueueCopyImageToBuffer",
    //"clEnqueueCopyBufferToImage",
    "clEnqueueNDRangeKernel", // maybe
    "clEnqueueMapImage",
    "clEnqueueNDRangeKernel" // NDRange
};

std::vector<std::string> openglDrawCalls
{
    "glDrawArrays",
    "glDrawArraysInstancedBaseInstance",
    "glDrawArraysInstanced",
    "glDrawArraysIndirect",
    "glMultiDrawArrays",
    "glMultiDrawArraysIndirect",
    "glDrawElements",
    "glDrawElementsInstancedBaseInstance",
    "glDrawElementsInstanced",
    "glMultiDrawElements",
    "glDrawRangeElements",
    "glDrawElementsBaseVertex",
    "glDrawRangeElementsBaseVertex",
    "glDrawElementsInstancedBaseVertex",
    "glDrawElementsInstancedBase-VertexBaseInstance",
    "glDrawElementsIndirect",
    "glMultiDrawElementsIndirect",
    "glDrawElementsBaseVertex",
    "glDrawTransformFeedback",
    "glDrawTransformFeedbackInstanced",
    "glDrawTransformStream",
    "glDrawTransformStreamInstanced",
    "glDrawBuffer",
    "glDrawBuffers"
};

std::vector<std::string> openglBufferCalls
{
    "glBindBuffer",
    "glBindBufferBase",
    "glBindBufferRange",
    "glBindBuffersBase",
    "glBindBuffersRange",
    "glBindVertexBuffer",
    "glBindVertexBuffers",
    "glBufferData",
    "glBufferStorage",
    "glBufferSubData",
    "glClearBufferData",
    "glClearBufferfi",
    "glClearBufferfv",
    "glClearBufferiv",
    "glClearBufferSubData",
    "glClearBufferuiv",
    "glClearNamedBufferData",
    "glClearNamedBufferSubData",
    "glCopyBufferSubData",
    "glCopyNamedBufferSubData",
    "glCreateBuffers",
    "glDeleteBuffers",
    "glDrawBuffer",
    "glDrawBuffers"
    "glDrawBuffers",
    "glFlushMapppedBufferRange",
    "glFlushMapppedNamedBufferRange"
    "glFrameBufferTexture",
    "glFrameBufferTexture1D",
    "glFrameBufferTexture2D",
    "glFrameBufferTexture3D",
    "glFrameBufferTextureLayer",
    "glGenBuffers",
    "glGetActiveAtomicCounterBufferiv",
    "glGetBufferParameteri64v",
    "glGetBufferParameteriv",
    "glGetBufferPointerv",
    "glGetBufferSubData",
    "glGetNamedBufferParameteri64v",
    "glGetNamedBufferParameteriv",
    "glGetNamedBufferPointerv",
    "glGetNamedBufferSubData",
    "glIsBuffer",
    "glIsFrameBuffer",
    "glMapBuffer",
    "glMapNamedBuffer",
    "glMapNamedBufferRange"
    "glMapNamedBufferRange",
    "glNamedBufferData",
    "glNamedBufferStorage",
    "glNamedBufferSubData"
    "glNamedBufferSubData",
    "glNamedFramebufferDrawBuffer",
    "glNamedFramebufferDrawBuffers",
    "glNamedFramebufferReadBuffer"
    "glNamedFrameBufferTextureLayer",
    "glReadBuffer",
    "glTexBuffer",
    "glTexBufferRange",
    "glTextureBuffer",
    "glTextureBufferRange",
    "glTransformBufferFeedbackBufferRange",
    "glTransformFeedbackBufferBase",
    "glUnmapBuffer",
    "glUnmapNamedBuffer",
    "glVertexArrayElementBuffer",
    "glVertexArrayVertexBuffer",
    "glVertexArrayVertexBuffers"
};

std::vector<std::string> openglStateCalls
{
    "glBindFramebuffer",
    "glBindVertexArray",
    "glBindBuffer",
    "glBindBufferRange",
    "glBindBufferBase",
    "glBindBuffersRange",
    "glBindBuffersBase",
    "glBindProgramPipeline",
    "glBindTexture",
    "glBindTextures",
    "glBindSampler",
    "glBindSamplers",
    "glBindImageTexture",
    "glBindImageTextures",
    "glBindTextureUnit",
    "glBindVertexArray",
    "glBindAttribLocation",
    "glBindRenderbuffer",
    "glBindVertexBuffer",
    "glBindVertexBuffers",
    "glBindTransformFeedback",
    "glBindFragDataLocationIndex",
    "glBindFragDataLocation",
    "glEnable",
    "glEnableVertexAttribArray",
    "glEnableVertexArrayAttrib",
    "glDisable",
    "glDepthMask"
};

std::vector<std::string> openglFrameCalls
{
    "eglSwapBuffers",
    "glFinish",
    "glClear"
};

std::vector<std::string> openglTextureCalls
{
    "glActiveTexture",
    "glBindImageTexture",
    "glBindImageTextures",
    "glBindTexture",
    "glBindTextures",
    "glBindTextureUnit",
    "glClearTexImage",
    "glClearTexSubImage",
    "glCompressedTexImage1D",
    "glCompressedTexImage2D",
    "glCompressedTexImage3D"
    "glCompressedTexSubImage1D",
    "glCompressedTexSubImage2D",
    "glCompressedTexSubImage3D",
    "glCompressedTextureSubImage1D",
    "glCompressedTextureSubImage2D",
    "glCompressedTextureSubImage3D",
    "glCopyTexImage1D",
    "glCopyTexImage2D",
    "glCopyTexSubImage1D",
    "glCopyTexSubImage2D",
    "glCopyTexSubImage3D",
    "glCopyTextureSubImage1D",
    "glCopyTextureSubImage2D",
    "glCopyTextureSubImage3D",
    "glCreateTextures",
    "glDeleteTextures",
    "glFramebufferTexture",
    "glFrameBufferTexture",
    "glFramebufferTexture1D",
    "glFrameBufferTexture1D",
    "glFramebufferTexture2D",
    "glFrameBufferTexture2D",
    "glFramebufferTexture3D",
    "glFrameBufferTexture3D",
    "glFrameBufferTextureLayer",
    "glGenerateTextureMipmap"
    "glGenTextures",
    "glGetCompressedTexImage",
    "glGetCompressedTextureImage",
    "glGetCompressedTextureSubImage",
    "glGetnCompressedTexImage",
    "glGetTexImage",
    "glGetTexLevelParameterfv",
    "glGetTexLevelParameteriv",
    "glGetTexLevelParameterliv",
    "glGetTexLevelParameterluiv",
    "glGetTexParameterfv",
    "glGetTexParameteriv",
    "glGetTexParameterliv",
    "glGetTexParameterluiv",
    "glGetTextureImage",
    "glGetTextureParameterfv",
    "glGetTextureParameteriv",
    "glGetTextureParameterliv",
    "glGetTextureParameterluiv",
    "glGetTextureSubImage",
    "glInvalidateTexImage",
    "glInvalidateTexSubImage",
    "glIsTexture",
    "glNamedFramebufferTexture",
    "glNamedFrameBufferTextureLayer",
    "glTexBuffer",
    "glTexBufferRange",
    "glTexImage1D",
    "glTexImage2D",
    "glTexImage2DMultisample",
    "glTexImage3DMultisample",
    "glTexImage3D",
    "glTexParameterf",
    "glTexParameterfv",
    "glTexParameteri",
    "glTexParameteriv",
    "glTexParameterliv",
    "glTexParameterluiv",
    "glTexStorage1D",
    "glTexStorage2D",
    "glTexStorage2DMultisample",
    "glTexStorage3D",
    "glTexSubImage1D",
    "glTexSubImage2D",
    "glTexSubImage3D",
    "glTexSubImage3D",
    "glTextureBarrier",
    "glTextureBuffer",
    "glTextureBufferRange",
    "glTextureParameterf",
    "glTextureParameterfv",
    "glTextureParameteri",
    "glTextureParameteriv",
    "glTextureParameterliv",
    "glTextureParameterluiv",
    "glTextureStorage1D",
    "glTextureStorage2D",
    "glTextureStorage3D",
    "glTextureStorage3DMultisample",
    "glTextureSubImage1D",
    "glTextureSubImage2D",
    "glTextureSubImage3D",
    "glTextureView"
};

std::vector<std::string> vulkanPipelineCalls
{
    "vkCmdBindPipeline",
    "vkCmdPipelineBarrier",
    "vkCreateComputePipelines",
    "vkCreateGraphicsPipelines",
    "vkDestroyPipeline",
    "vkCreatePipelineCache",
    "vkMergePipelineCaches",
    "vkGetPipelineCacheData",
    "vkDestroyPipelineCache",
    "vkCreatePipelineLayout",
    "vkDestroyPipelineLayout"
};

std::vector<std::string> vulkanDescriptorCalls
{
    "vkCmdBindDescriptorSets",
    "vkCreateDescriptorSetLayout",
    "vkDestroyDescriptorSetLayout",
    "vkCreateDescriptorPool",
    "vkDestroyDescriptorPool",
    "vkAllocateDescriptorSets",
    "vkFreeDescriptorSets",
    "vkResetDescriptorPool",
    "vkUpdateDescriptorSets"

};

std::vector<std::string> vulkanOtherCalls
{
    "vkCmdExecuteCommands",
    "vkCmdResolveImage",
    "vkAllocateMemory"
};

std::vector<std::string> openglBlock
{
    "glFlush",
    "glFinish",
    "glBufferSubData",
    "glNamedBufferSubData",
    "glMapBuffer",
    "glMapNamedBuffer",
    "glMapNamedBufferRange",
    "glTexSubImage1D",
    "glTexSubImage2D",
    "glTexSubImage3D",
    "glWaitSync",
    "glClientWaitSync",
    "glReadPixels"
};

std::vector<std::string> openglShaderUseCalls
{
    "glActiveShaderProgram",
    "glAttachShader",
    "glCompileShader",
    "glCreateShader",
    "glCreateShaderProgramv",
    "glDeleteShader",
    "glDetachShader",
    "glGetAttachedShaders",
    "glGetShaderInfoLog",
    "glGetShaderiv",
    "glGetShaderPrecisionFormat",
    "glgetShaderSource",
    "glIsShader",
    "glReleaseShaderCompiler",
    "glShaderBinary",
    "glShaderSource",
    "glShaderStorageBlockBinding",
    "glUseProgram"
};

// *** Counter Categories ***

std::vector<std::string> cpu_tempCounters
{
    "CPU Temperature",
    "CPU temperature",
    "CPU Cluster 0 Temperature",
    "CPU Cluster 1 Temperature"
};

std::vector<std::string> gpu_tempCounters
{
    "GPU Temperature"
};

std::vector<std::string> cpu_loadCounters
{
    "CPU Core 0 Load",
    "CPU Core 1 Load",
    "CPU Core 2 Load",
    "CPU Core 3 Load",
    "CPU Core 4 Load",
    "CPU Core 5 Load",
    "CPU Core 6 Load",
    "CPU Core 7 Load",
    "User Cpu0 Usage",
    "User Cpu1 Usage"
};

std::vector<std::string> gpu_loadCounters
{
    "GPU Load"
};

std::vector<std::string> cpu_freqCounters
{
    "Freq Core 1",
    "Freq Core 2",
    "CPU Core 0 Frequency",
    "CPU Core 1 Frequency",
    "CPU Core 2 Frequency",
    "CPU Core 3 Frequency",
    "CPU Core 4 Frequency",
    "CPU Core 5 Frequency",
    "CPU Core 6 Frequency",
    "CPU Core 7 Frequency"
};

std::vector<std::string> fpsCounters
{
    "FPS"
};

std::vector<std::string> gpu_freqCounters
{
};

std::vector<std::string> powerCounters
{
    "Battery Power"
};

std::vector<std::string> fb_stallCounters
{
    "Core0 FB Stall Cycles",
    "Core1 FB Stall Cycles"
};

std::vector<std::string> total_cycleCounters
{
    "Core0 Total Cycles",
    "Core1 Total Cycles"
};

std::vector<std::string> tex_stallCounters
{
    "Core0 TexMap Stall Cycles",
    "Core1 TexMap Stall Cycles"
};

std::vector<std::string> num_instCounters
{
    "Core0 Instructions Number",
    "Core1 Instruction Number"
};

std::vector<std::string> num_pixelsCounters
{
    "Core0 Number of pixels",
    "Core1 Number of pixels"
};

std::vector<std::string> activityCounters
{
    "Core0 Scheduler Active Cycles",
    "Core0 Pipe Active Cycles",
    "Core0 Instr. Memory Active",
    "Core0 Register File Active",
    "Core0 TextureMap Active"
};


namespace lpgpu2 {
namespace db {

/// @brief               Class constructor, no work is performed here.
/// @param    vDbAdapter Reference to the database adapter used to interact with the LPGPU2Database.
LPGPU2FeedbackEnginePreProcessor::LPGPU2FeedbackEnginePreProcessor(LPGPU2DatabaseAdapter &vDbAdapter)
    : m_dbAdapter(vDbAdapter)
{
}

/// @brief Class destructor. No work if performed here.
LPGPU2FeedbackEnginePreProcessor::~LPGPU2FeedbackEnginePreProcessor()
{
}

/// @brief  Returns the error message stored in case any method call fails.
/// @returns gtString   The error messaged stored in the class.
const gtString& LPGPU2FeedbackEnginePreProcessor::GetErrorMessage() const
{
    return m_errorMessage;
}

/// @brief Entry point for generating the data necessary for the feedback analysis engine.
///        Basically inserts all the data required to perform the actual analysis in the database.
/// @return status  success = All the data was inserted successfully,
///                 failure = An attempt to insert the data was made but something went wrong in
///                           at least one of the insertions.
status LPGPU2FeedbackEnginePreProcessor::Run()
{
    auto bReturn = success;

    // Delete previous call and counter categories
    m_dbAdapter.DeleteCallCategories();
    m_dbAdapter.DeleteCounterCategories();

    bReturn &= InsertCallsCategories();
    bReturn &= InsertCounterCategories();    

    return bReturn;
}
 
status LPGPU2FeedbackEnginePreProcessor::InsertCallsCategories()
{
    status bReturn = success;

    m_dbAdapter.BeginTransaction();        
    bReturn &= InsertFrameDelimitersCalls();
    bReturn &= InsertDrawTypeCalls();
    bReturn &= InsertBindTypeCalls();
    bReturn &= InsertWaitTypeCalls();
    bReturn &= InsertSetupTypeCalls();
    bReturn &= InsertErrorTypeCalls();
    bReturn &= InsertCompileTypeCalls();
    bReturn &= InsertComputeTypeCalls();
    bReturn &= InsertDataTransferTypeCalls();
    bReturn &= InsertStateTypeCalls();
    bReturn &= InsertFrameTypeCalls();
    bReturn &= InsertTextureTypeCalls();
    bReturn &= InsertPipelineTypeCalls();
    bReturn &= InsertDescriptorTypeCalls();
    bReturn &= InsertBufferTypeCalls();
    bReturn &= InsertMemoryTypeCalls();
    bReturn &= InsertBlockTypeCalls();
    bReturn &= InsertShaderTypeCalls();
    bReturn &= InsertOtherTypeCalls();    
    m_dbAdapter.EndTransaction();

    return bReturn;
}

status LPGPU2FeedbackEnginePreProcessor::InsertCounterCategories()
{
    // This replaces the aliasDictionary on the LUAW6Bindings.h

    status bReturn = success;

    m_dbAdapter.BeginTransaction(); 
    bReturn &= InsertCounterType(cpu_tempCounters, CPU_TEMP_CATEGORY);
    bReturn &= InsertCounterType(gpu_tempCounters, GPU_TEMP);    
    bReturn &= InsertCounterType(cpu_loadCounters, CPU_LOAD);
    bReturn &= InsertCounterType(gpu_loadCounters, GPU_LOAD);
    bReturn &= InsertCounterType(cpu_freqCounters, CPU_FREQ);
    bReturn &= InsertCounterType(fpsCounters, FPS);
    bReturn &= InsertCounterType(gpu_freqCounters, GPU_FREQ);
    bReturn &= InsertCounterType(powerCounters, POWER);
    bReturn &= InsertCounterType(fb_stallCounters, FB_STALLS);
    bReturn &= InsertCounterType(total_cycleCounters, TOTAL_CYCLES);
    bReturn &= InsertCounterType(tex_stallCounters, TEX_STALLS);
    bReturn &= InsertCounterType(num_instCounters, NUM_INST);
    bReturn &= InsertCounterType(num_pixelsCounters, NUM_PIXELS);
    bReturn &= InsertCounterType(activityCounters, ACTIVITY);
    m_dbAdapter.EndTransaction(); 

    return bReturn;
}

status LPGPU2FeedbackEnginePreProcessor::InsertCallsType(std::vector<std::string> &calls, gtString category, gtString api)
{
    status bReturn = success;

    // NOTE: please do not use uint since this is not compatible with Visual Studio
    for (size_t i = 0; (i < calls.size()); i++)
    {
        gtString callName;
        callName.fromASCIIString(calls[i].c_str());
        bReturn &= m_dbAdapter.InsertCallCategory({ callName, category, api });
    }
    return bReturn;
}

status LPGPU2FeedbackEnginePreProcessor::InsertCounterType(std::vector<std::string> &counter, gtString category)
{
    status bReturn = success;

    for (size_t i = 0; (i < counter.size()); i++)
    {
        gtString counterName;
        counterName.fromASCIIString(counter[i].c_str());
        bReturn &= m_dbAdapter.InsertCounterCategory({ counterName, category});
    }
    return bReturn;
}

status LPGPU2FeedbackEnginePreProcessor::InsertFrameDelimitersCalls()
{
    status bReturn = success;

    bReturn &= InsertCallsType(openclFrameDelimiters, FRAME_DELIMITER_CATEGORY, OPENCL_API);
    bReturn &= InsertCallsType(vulkanFrameDelimiters, FRAME_DELIMITER_CATEGORY, VULKAN_API);
    bReturn &= InsertCallsType(openglFrameDelimiters, FRAME_DELIMITER_CATEGORY, OPENGL_API);
    bReturn &= InsertCallsType(eglFrameDelimiters, FRAME_DELIMITER_CATEGORY, OPENGLES_API);

    return bReturn;
}

status LPGPU2FeedbackEnginePreProcessor::InsertDrawTypeCalls()
{
    status bReturn = success;

    bReturn &= InsertCallsType(openclDrawTypeCalls, DRAW_CATEGORY, OPENCL_API);
    bReturn &= InsertCallsType(vulkanDrawTypeCalls, DRAW_CATEGORY, VULKAN_API);
    bReturn &= InsertCallsType(openglDrawTypeCalls, DRAW_CATEGORY, OPENGL_API);

    return bReturn;
}

status LPGPU2FeedbackEnginePreProcessor::InsertBindTypeCalls()
{
    status bReturn = success;

    bReturn &= InsertCallsType(openclBindTypeCalls, BIND_CATEGORY, OPENCL_API);
    bReturn &= InsertCallsType(vulkanBindTypeCalls, BIND_CATEGORY, VULKAN_API);
    bReturn &= InsertCallsType(openglBindTypeCalls, BIND_CATEGORY, OPENGL_API);
    bReturn &= InsertCallsType(eglBindTypeCalls, BIND_CATEGORY, OPENGLES_API);

    return bReturn;
}

status LPGPU2FeedbackEnginePreProcessor::InsertWaitTypeCalls()
{
    status bReturn = success;

    bReturn &= InsertCallsType(openclWaitTypeCalls, WAIT_CATEGORY, OPENCL_API);
    bReturn &= InsertCallsType(vulkanWaitTypeCalls, WAIT_CATEGORY, VULKAN_API);
    bReturn &= InsertCallsType(openglWaitTypeCalls, WAIT_CATEGORY, OPENGL_API);
    bReturn &= InsertCallsType(eglWaitTypeCalls, WAIT_CATEGORY, OPENGLES_API);

    return bReturn;
}

status LPGPU2FeedbackEnginePreProcessor::InsertSetupTypeCalls()
{
    status bReturn = success;

    bReturn &= InsertCallsType(openclSetupTypeCalls, SETUP_CATEGORY, OPENCL_API);
    bReturn &= InsertCallsType(vulkanSetupTypeCalls, SETUP_CATEGORY, VULKAN_API);
    bReturn &= InsertCallsType(openglSetupTypeCalls, SETUP_CATEGORY, OPENGL_API);
    bReturn &= InsertCallsType(eglSetupTypeCalls, SETUP_CATEGORY, OPENGLES_API);

    return bReturn;
}

status LPGPU2FeedbackEnginePreProcessor::InsertErrorTypeCalls()
{
    status bReturn = success;

    bReturn &= InsertCallsType(openclErrorTypeCalls, ERROR_CATEGORY, OPENCL_API);
    bReturn &= InsertCallsType(vulkanErrorTypeCalls, ERROR_CATEGORY, VULKAN_API);
    bReturn &= InsertCallsType(openglErrorTypeCalls, ERROR_CATEGORY, OPENGL_API);
    bReturn &= InsertCallsType(eglErrorTypeCalls, ERROR_CATEGORY, OPENGLES_API);

    return bReturn;
}

status LPGPU2FeedbackEnginePreProcessor::InsertCompileTypeCalls()
{
    return InsertCallsType(openclCompileFunctions, COMPILE_CATEGORY, OPENCL_API);
}

status LPGPU2FeedbackEnginePreProcessor::InsertComputeTypeCalls()
{
    return InsertCallsType(openclComputeFunctions, COMPUTE_CATEGORY, OPENCL_API);
}

status LPGPU2FeedbackEnginePreProcessor::InsertDataTransferTypeCalls()
{
    return InsertCallsType(openclDataTransferFunctions, DATA_TRANSFER_CATEGORY, OPENCL_API);
}

status LPGPU2FeedbackEnginePreProcessor::InsertStateTypeCalls()
{
    return InsertCallsType(openglStateCalls, STATE_CATEGORY, OPENGL_API);
}

status LPGPU2FeedbackEnginePreProcessor::InsertFrameTypeCalls()
{
    return InsertCallsType(openglFrameCalls, FRAME_CATEGORY, OPENGL_API);
}

status LPGPU2FeedbackEnginePreProcessor::InsertTextureTypeCalls()
{
    return InsertCallsType(openglTextureCalls, TEXTURE_CATEGORY, OPENGL_API);
}

status LPGPU2FeedbackEnginePreProcessor::InsertPipelineTypeCalls()
{
    return InsertCallsType(vulkanPipelineCalls, PIPELINE_CATEGORY, VULKAN_API);
}

status LPGPU2FeedbackEnginePreProcessor::InsertDescriptorTypeCalls()
{
    return InsertCallsType(vulkanDescriptorCalls, DESCRIPTOR_CATEGORY, VULKAN_API);
}

status LPGPU2FeedbackEnginePreProcessor::InsertBufferTypeCalls()
{
    return InsertCallsType(openglBufferCalls, BUFFER_CATEGORY, OPENGL_API);
}

status LPGPU2FeedbackEnginePreProcessor::InsertMemoryTypeCalls()
{
    return InsertCallsType(openclMemAllocationAndSetupFunctions, MEMORY_ALLOCATION_CATEGORY, OPENCL_API);
}

status LPGPU2FeedbackEnginePreProcessor::InsertBlockTypeCalls()
{
    return InsertCallsType(openglBlock, BLOCK_CATEGORY, OPENGL_API);
}

status LPGPU2FeedbackEnginePreProcessor::InsertShaderTypeCalls()
{
    return InsertCallsType(openglShaderUseCalls, SHADER_CATEGORY, OPENGL_API);
}

status LPGPU2FeedbackEnginePreProcessor::InsertOtherTypeCalls()
{
    return InsertCallsType(vulkanOtherCalls, OTHER_CATEGORY, VULKAN_API);
}



} // namespace db
} // namespace lpgpu2

#ifndef _ICD_DETOUR_BYPASS_H_
#define _ICD_DETOUR_BYPASS_H_

#ifndef CL_USE_DEPRECATED_OPENCL_1_0_APIS
#define CL_USE_DEPRECATED_OPENCL_1_0_APIS
#endif

#ifndef CL_USE_DEPRECATED_OPENCL_1_1_APIS
#define CL_USE_DEPRECATED_OPENCL_1_1_APIS
#endif

#ifndef CL_USE_DEPRECATED_OPENCL_1_2_APIS
#define CL_USE_DEPRECATED_OPENCL_1_2_APIS
#endif

#ifndef CL_USE_DEPRECATED_OPENCL_2_0_APIS
#define CL_USE_DEPRECATED_OPENCL_2_0_APIS
#endif

// cl.h
#include <CL/cl.h>

// cl_gl.h and required files
#ifdef _WIN32
#include <windows.h>
#include <d3d9.h>
#include <d3d10_1.h>
#include <CL/cl_d3d10.h>
#include <CL/cl_d3d11.h>
#include <CL/cl_dx9_media_sharing.h>
#endif
#if !defined(__ANDROID__)
#include <GL/gl.h>
#else
#include <GLES/gl.h>
#endif
#include <CL/cl_gl.h>
#include <CL/cl_gl_ext.h>
#include <CL/cl_ext.h>
#include <CL/cl_egl.h>

CL_API_ENTRY cl_int CL_API_CALL
detour_bypass_clGetEventInfo(cl_event         event,
                             cl_event_info    param_name,
                             size_t           param_value_size,
                             void *           param_value,
                             size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_bypass_clGetCommandQueueInfo(cl_command_queue      command_queue,
                                    cl_command_queue_info param_name,
                                    size_t                param_value_size,
                                    void *                param_value,
                                    size_t *              param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_bypass_clGetDeviceInfo(
    cl_device_id    device,
    cl_device_info  param_name, 
    size_t          param_value_size, 
    void *          param_value,
    size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_bypass_clGetContextInfo(cl_context         context, 
                               cl_context_info    param_name, 
                               size_t             param_value_size, 
                               void *             param_value, 
                               size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;


                               CL_API_ENTRY cl_int CL_API_CALL
detour_bypass_clGetKernelInfo(cl_kernel       kernel,
                cl_kernel_info  param_name,
                size_t          param_value_size,
                void *          param_value,
                size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_bypass_clGetKernelArgInfo(cl_kernel       kernel,
                   cl_uint         arg_indx,
                   cl_kernel_arg_info  param_name,
                   size_t          param_value_size,
                   void *          param_value,
                   size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_2;

CL_API_ENTRY cl_int CL_API_CALL
detour_bypass_clGetKernelWorkGroupInfo(cl_kernel                  kernel,
                         cl_device_id               device,
                         cl_kernel_work_group_info  param_name,
                         size_t                     param_value_size,
                         void *                     param_value,
                         size_t *                   param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_bypass_clRetainEvent(cl_event event) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_bypass_clReleaseEvent(cl_event event) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_bypass_clGetEventProfilingInfo(cl_event            event,
                        cl_profiling_info   param_name,
                        size_t              param_value_size,
                        void *              param_value,
                        size_t *            param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

#endif // _ICD_DETOUR_BYPASS_
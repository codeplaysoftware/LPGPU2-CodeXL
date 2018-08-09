#ifndef _ICD_DETOUR_H_
#define _ICD_DETOUR_H_

// cl.h
#include <CL/cl.h>

#include "icd_dispatch.h"

// Detour function typedefs. Identical to the normal CL function types with
// the addition of the 'next' func pointer.

// Platform APIs
typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clGetPlatformIDs)(
    KHRpfn_clGetPlatformIDs next,
                 cl_uint          num_entries,
                 cl_platform_id * platforms,
                 cl_uint *        num_platforms) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clGetPlatformInfo)(
    KHRpfn_clGetPlatformInfo next,
    cl_platform_id   platform, 
    cl_platform_info param_name,
    size_t           param_value_size, 
    void *           param_value,
    size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

// Device APIs
typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clGetDeviceIDs)(
    KHRpfn_clGetDeviceIDs next,
    cl_platform_id   platform,
    cl_device_type   device_type, 
    cl_uint          num_entries, 
    cl_device_id *   devices, 
    cl_uint *        num_devices) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clGetDeviceInfo)(
    KHRpfn_clGetDeviceInfo next,
    cl_device_id    device,
    cl_device_info  param_name, 
    size_t          param_value_size, 
    void *          param_value,
    size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clCreateSubDevices)(
    KHRpfn_clCreateSubDevices next,
    cl_device_id     in_device,
    const cl_device_partition_property * partition_properties,
    cl_uint          num_entries,
    cl_device_id *   out_devices,
    cl_uint *        num_devices);

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clRetainDevice)(
    KHRpfn_clRetainDevice next,
    cl_device_id     device) CL_API_SUFFIX__VERSION_1_2;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clReleaseDevice)(
    KHRpfn_clReleaseDevice next,
    cl_device_id     device) CL_API_SUFFIX__VERSION_1_2;

// Context APIs
typedef CL_API_ENTRY cl_context (CL_API_CALL *pfn_detour_clCreateContext)(
    KHRpfn_clCreateContext next,
    const cl_context_properties * properties,
    cl_uint                 num_devices,
    const cl_device_id *    devices,
    void (CL_CALLBACK *pfn_notify)(
    const char *, const void *, size_t, void *),
    void *                  user_data,
    cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_context (CL_API_CALL *pfn_detour_clCreateContextFromType)(
    KHRpfn_clCreateContextFromType next,
    const cl_context_properties * properties,
    cl_device_type          device_type,
    void (CL_CALLBACK *pfn_notify)(
    const char *, const void *, size_t, void *),
    void *                  user_data,
    cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clRetainContext)(
    KHRpfn_clRetainContext next,
    cl_context context) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clReleaseContext)(
    KHRpfn_clReleaseContext next,
    cl_context context) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clGetContextInfo)(
    KHRpfn_clGetContextInfo next,
    cl_context         context, 
    cl_context_info    param_name, 
    size_t             param_value_size, 
    void *             param_value, 
    size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

// Command Queue APIs
typedef CL_API_ENTRY cl_command_queue (CL_API_CALL *pfn_detour_clCreateCommandQueue)(
    KHRpfn_clCreateCommandQueue next,
    cl_context                     context, 
    cl_device_id                   device, 
    cl_command_queue_properties    properties,
    cl_int *                       errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_command_queue (CL_API_CALL *pfn_detour_clCreateCommandQueueWithProperties)(
    KHRpfn_clCreateCommandQueueWithProperties next,
    cl_context                  /* context */,
    cl_device_id                /* device */,
    const cl_queue_properties * /* properties */,
    cl_int *                    /* errcode_ret */) CL_API_SUFFIX__VERSION_2_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clRetainCommandQueue)(
    KHRpfn_clRetainCommandQueue next,
    cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clReleaseCommandQueue)(
    KHRpfn_clReleaseCommandQueue next,
    cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clGetCommandQueueInfo)(
    KHRpfn_clGetCommandQueueInfo next,
    cl_command_queue      command_queue,
    cl_command_queue_info param_name,
    size_t                param_value_size,
    void *                param_value,
    size_t *              param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

// Memory Object APIs
typedef CL_API_ENTRY cl_mem (CL_API_CALL *pfn_detour_clCreateBuffer)(
    KHRpfn_clCreateBuffer next,
    cl_context   context,
    cl_mem_flags flags,
    size_t       size,
    void *       host_ptr,
    cl_int *     errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_mem (CL_API_CALL *pfn_detour_clCreateImage)(
    KHRpfn_clCreateImage next,
    cl_context              context,
    cl_mem_flags            flags,
    const cl_image_format * image_format,
    const cl_image_desc *   image_desc,
    void *                  host_ptr,
    cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_2;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clRetainMemObject)(
    KHRpfn_clRetainMemObject next,cl_mem memobj) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clReleaseMemObject)(
    KHRpfn_clReleaseMemObject next,cl_mem memobj) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clGetSupportedImageFormats)(
    KHRpfn_clGetSupportedImageFormats next,
    cl_context           context,
    cl_mem_flags         flags,
    cl_mem_object_type   image_type,
    cl_uint              num_entries,
    cl_image_format *    image_formats,
    cl_uint *            num_image_formats) CL_API_SUFFIX__VERSION_1_0;
                                    
typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clGetMemObjectInfo)(
    KHRpfn_clGetMemObjectInfo next,
    cl_mem           memobj,
    cl_mem_info      param_name, 
    size_t           param_value_size,
    void *           param_value,
    size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clGetImageInfo)(
    KHRpfn_clGetImageInfo next,
    cl_mem           image,
    cl_image_info    param_name, 
    size_t           param_value_size,
    void *           param_value,
    size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_mem (CL_API_CALL *pfn_detour_clCreatePipe)(
    KHRpfn_clCreatePipe next,
    cl_context                 /* context */,
    cl_mem_flags               /* flags */,
    cl_uint                    /* pipe_packet_size */,
    cl_uint                    /* pipe_max_packets */,
    const cl_pipe_properties * /* properties */,
    cl_int *                   /* errcode_ret */) CL_API_SUFFIX__VERSION_2_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clGetPipeInfo)(
    KHRpfn_clGetPipeInfo next,
    cl_mem       /* pipe */,
    cl_pipe_info /* param_name */,
    size_t       /* param_value_size */,
    void *       /* param_value */,
    size_t *     /* param_value_size_ret */) CL_API_SUFFIX__VERSION_2_0;

typedef CL_API_ENTRY void * (CL_API_CALL *pfn_detour_clSVMAlloc)(
    KHRpfn_clSVMAlloc next,
    cl_context       /* context */,
    cl_svm_mem_flags /* flags */,
    size_t           /* size */,
    unsigned int     /* alignment */) CL_API_SUFFIX__VERSION_2_0;

typedef CL_API_ENTRY void (CL_API_CALL *pfn_detour_clSVMFree)(
    KHRpfn_clSVMFree next,
    cl_context /* context */,
    void *     /* svm_pointer */) CL_API_SUFFIX__VERSION_2_0;

// Sampler APIs
typedef CL_API_ENTRY cl_sampler (CL_API_CALL *pfn_detour_clCreateSampler)(
    KHRpfn_clCreateSampler next,
    cl_context          context,
    cl_bool             normalized_coords, 
    cl_addressing_mode  addressing_mode, 
    cl_filter_mode      filter_mode,
    cl_int *            errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clRetainSampler)(
    KHRpfn_clRetainSampler next,cl_sampler sampler) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clReleaseSampler)(
    KHRpfn_clReleaseSampler next,cl_sampler sampler) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clGetSamplerInfo)(
    KHRpfn_clGetSamplerInfo next,
    cl_sampler         sampler,
    cl_sampler_info    param_name,
    size_t             param_value_size,
    void *             param_value,
    size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_sampler (CL_API_CALL *pfn_detour_clCreateSamplerWithProperties)(
    KHRpfn_clCreateSamplerWithProperties next,
    cl_context                    /* context */,
    const cl_sampler_properties * /* sampler_properties */,
    cl_int *                      /* errcode_ret */) CL_API_SUFFIX__VERSION_2_0;

// Program Object APIs
typedef CL_API_ENTRY cl_program (CL_API_CALL *pfn_detour_clCreateProgramWithSource)(
    KHRpfn_clCreateProgramWithSource next,
    cl_context        context,
    cl_uint           count,
    const char **     strings,
    const size_t *    lengths,
    cl_int *          errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_program (CL_API_CALL *pfn_detour_clCreateProgramWithBinary)(
    KHRpfn_clCreateProgramWithBinary next,
    cl_context                     context,
    cl_uint                        num_devices,
    const cl_device_id *           device_list,
    const size_t *                 lengths,
    const unsigned char **         binaries,
    cl_int *                       binary_status,
    cl_int *                       errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_program (CL_API_CALL *pfn_detour_clCreateProgramWithBuiltInKernels)(
    KHRpfn_clCreateProgramWithBuiltInKernels next,
    cl_context            context,
    cl_uint               num_devices,
    const cl_device_id *  device_list,
    const char *          kernel_names,
    cl_int *              errcode_ret) CL_API_SUFFIX__VERSION_1_2;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clRetainProgram)(
    KHRpfn_clRetainProgram next,cl_program program) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clReleaseProgram)(
    KHRpfn_clReleaseProgram next,cl_program program) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clBuildProgram)(
    KHRpfn_clBuildProgram next,
    cl_program           program,
    cl_uint              num_devices,
    const cl_device_id * device_list,
    const char *         options, 
    void (CL_CALLBACK *pfn_notify)(
    cl_program program, void * user_data),
    void *               user_data) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clCompileProgram)(
    KHRpfn_clCompileProgram next,
    cl_program           program,
    cl_uint              num_devices,
    const cl_device_id * device_list,
    const char *         options,
    cl_uint              num_input_headers,
    const cl_program *   input_headers,
    const char **        header_include_names,
    void (CL_CALLBACK *  pfn_notify)(
    cl_program program, void * user_data),
    void *               user_data) CL_API_SUFFIX__VERSION_1_2;

typedef CL_API_ENTRY cl_program (CL_API_CALL *pfn_detour_clLinkProgram)(
    KHRpfn_clLinkProgram next,
    cl_context           context,
    cl_uint              num_devices,
    const cl_device_id * device_list,
    const char *         options,
    cl_uint              num_input_programs,
    const cl_program *   input_programs,
    void (CL_CALLBACK *  pfn_notify)(
    cl_program program, void * user_data),
    void *               user_data,
    cl_int *             errcode_ret) CL_API_SUFFIX__VERSION_1_2;

/*typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clSetProgramSpecializationConstant)(
    KHRpfn_clSetProgramSpecializationConstant next,
    cl_program           program,
    cl_uint              spec_id,
    size_t               spec_size,
    const void*          spec_value) CL_API_SUFFIX__VERSION_2_2;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clSetProgramReleaseCallback)(
    KHRpfn_clSetProgramReleaseCallback next,
    cl_program           program,
    void (CL_CALLBACK *  pfn_notify)(
    cl_program program, void * user_data),
    void *               user_data) CL_API_SUFFIX__VERSION_2_2;*/

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clUnloadPlatformCompiler)(
    KHRpfn_clUnloadPlatformCompiler next,
    cl_platform_id     platform) CL_API_SUFFIX__VERSION_1_2;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clGetProgramInfo)(
    KHRpfn_clGetProgramInfo next,
    cl_program         program,
    cl_program_info    param_name,
    size_t             param_value_size,
    void *             param_value,
    size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clGetProgramBuildInfo)(
    KHRpfn_clGetProgramBuildInfo next,
    cl_program            program,
    cl_device_id          device,
    cl_program_build_info param_name,
    size_t                param_value_size,
    void *                param_value,
    size_t *              param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;
                            
// Kernel Object APIs
typedef CL_API_ENTRY cl_kernel (CL_API_CALL *pfn_detour_clCreateKernel)(
    KHRpfn_clCreateKernel next,
    cl_program      program,
    const char *    kernel_name,
    cl_int *        errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clCreateKernelsInProgram)(
    KHRpfn_clCreateKernelsInProgram next,
    cl_program     program,
    cl_uint        num_kernels,
    cl_kernel *    kernels,
    cl_uint *      num_kernels_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clRetainKernel)(
    KHRpfn_clRetainKernel next,cl_kernel    kernel) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clReleaseKernel)(
    KHRpfn_clReleaseKernel next,cl_kernel   kernel) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clSetKernelArg)(
    KHRpfn_clSetKernelArg next,
    cl_kernel    kernel,
    cl_uint      arg_index,
    size_t       arg_size,
    const void * arg_value) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clGetKernelInfo)(
    KHRpfn_clGetKernelInfo next,
    cl_kernel       kernel,
    cl_kernel_info  param_name,
    size_t          param_value_size,
    void *          param_value,
    size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clGetKernelArgInfo)(
    KHRpfn_clGetKernelArgInfo next,
    cl_kernel       kernel,
    cl_uint         arg_indx,
    cl_kernel_arg_info  param_name,
    size_t          param_value_size,
    void *          param_value,
    size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_2;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clGetKernelWorkGroupInfo)(
    KHRpfn_clGetKernelWorkGroupInfo next,
    cl_kernel                  kernel,
    cl_device_id               device,
    cl_kernel_work_group_info  param_name,
    size_t                     param_value_size,
    void *                     param_value,
    size_t *                   param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clSetKernelArgSVMPointer)(
    KHRpfn_clSetKernelArgSVMPointer next,
    cl_kernel    /* kernel */,
    cl_uint      /* arg_index */,
    const void * /* arg_value */) CL_API_SUFFIX__VERSION_2_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clSetKernelExecInfo)(
    KHRpfn_clSetKernelExecInfo next,
    cl_kernel            /* kernel */,
    cl_kernel_exec_info  /* param_name */,
    size_t               /* param_value_size */,
    const void *         /* param_value */) CL_API_SUFFIX__VERSION_2_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clGetKernelSubGroupInfoKHR)(
    KHRpfn_clGetKernelSubGroupInfoKHR next,
    cl_kernel                /* in_kernel */,
    cl_device_id             /*in_device*/,
    cl_kernel_sub_group_info /* param_name */,
    size_t                   /*input_value_size*/,
    const void *             /*input_value*/,
    size_t                   /*param_value_size*/,
    void*                    /*param_value*/,
    size_t*                  /*param_value_size_ret*/) CL_EXT_SUFFIX__VERSION_2_0;

// Event Object APIs
typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clWaitForEvents)(
    KHRpfn_clWaitForEvents next,
    cl_uint             num_events,
    const cl_event *    event_list) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clGetEventInfo)(
    KHRpfn_clGetEventInfo next,
    cl_event         event,
    cl_event_info    param_name,
    size_t           param_value_size,
    void *           param_value,
    size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;
                            
typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clRetainEvent)(
    KHRpfn_clRetainEvent next,cl_event event) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clReleaseEvent)(
    KHRpfn_clReleaseEvent next,cl_event event) CL_API_SUFFIX__VERSION_1_0;

// Profiling APIs
typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clGetEventProfilingInfo)(
    KHRpfn_clGetEventProfilingInfo next,
    cl_event            event,
    cl_profiling_info   param_name,
    size_t              param_value_size,
    void *              param_value,
    size_t *            param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;
                                
// Flush and Finish APIs
typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clFlush)(
    KHRpfn_clFlush next,cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clFinish)(
    KHRpfn_clFinish next,cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0;

// Enqueued Commands APIs
typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueReadBuffer)(
    KHRpfn_clEnqueueReadBuffer next,
    cl_command_queue    command_queue,
    cl_mem              buffer,
    cl_bool             blocking_read,
    size_t              offset,
    size_t              cb, 
    void *              ptr,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueReadBufferRect)(
    KHRpfn_clEnqueueReadBufferRect next,
    cl_command_queue    command_queue,
    cl_mem              buffer,
    cl_bool             blocking_read,
    const size_t *      buffer_origin,
    const size_t *      host_origin, 
    const size_t *      region,
    size_t              buffer_row_pitch,
    size_t              buffer_slice_pitch,
    size_t              host_row_pitch,
    size_t              host_slice_pitch,
    void *              ptr,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event) CL_API_SUFFIX__VERSION_1_1;
                            
typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueWriteBuffer)(
    KHRpfn_clEnqueueWriteBuffer next,
    cl_command_queue   command_queue, 
    cl_mem             buffer, 
    cl_bool            blocking_write, 
    size_t             offset, 
    size_t             cb, 
    const void *       ptr, 
    cl_uint            num_events_in_wait_list, 
    const cl_event *   event_wait_list, 
    cl_event *         event) CL_API_SUFFIX__VERSION_1_0;
                            
typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueWriteBufferRect)(
    KHRpfn_clEnqueueWriteBufferRect next,
    cl_command_queue    command_queue,
    cl_mem              buffer,
    cl_bool             blocking_read,
    const size_t *      buffer_origin,
    const size_t *      host_origin, 
    const size_t *      region,
    size_t              buffer_row_pitch,
    size_t              buffer_slice_pitch,
    size_t              host_row_pitch,
    size_t              host_slice_pitch,    
    const void *        ptr,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event) CL_API_SUFFIX__VERSION_1_1;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueFillBuffer)(
    KHRpfn_clEnqueueFillBuffer next,
    cl_command_queue   command_queue,
    cl_mem             buffer,
    const void *       pattern,
    size_t             pattern_size,
    size_t             offset,
    size_t             cb,
    cl_uint            num_events_in_wait_list,
    const cl_event *   event_wait_list,
    cl_event *         event) CL_API_SUFFIX__VERSION_1_2;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueCopyBuffer)(
    KHRpfn_clEnqueueCopyBuffer next,
    cl_command_queue    command_queue, 
    cl_mem              src_buffer,
    cl_mem              dst_buffer, 
    size_t              src_offset,
    size_t              dst_offset,
    size_t              cb, 
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event) CL_API_SUFFIX__VERSION_1_0;
                            
typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueCopyBufferRect)(
    KHRpfn_clEnqueueCopyBufferRect next,
    cl_command_queue    command_queue, 
    cl_mem              src_buffer,
    cl_mem              dst_buffer, 
    const size_t *      src_origin,
    const size_t *      dst_origin,
    const size_t *      region,
    size_t              src_row_pitch,
    size_t              src_slice_pitch,
    size_t              dst_row_pitch,
    size_t              dst_slice_pitch,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event) CL_API_SUFFIX__VERSION_1_1;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueReadImage)(
    KHRpfn_clEnqueueReadImage next,
    cl_command_queue     command_queue,
    cl_mem               image,
    cl_bool              blocking_read, 
    const size_t *       origin,
    const size_t *       region,
    size_t               row_pitch,
    size_t               slice_pitch, 
    void *               ptr,
    cl_uint              num_events_in_wait_list,
    const cl_event *     event_wait_list,
    cl_event *           event) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueWriteImage)(
    KHRpfn_clEnqueueWriteImage next,
    cl_command_queue    command_queue,
    cl_mem              image,
    cl_bool             blocking_write, 
    const size_t *      origin,
    const size_t *      region,
    size_t              input_row_pitch,
    size_t              input_slice_pitch, 
    const void *        ptr,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueFillImage)(
    KHRpfn_clEnqueueFillImage next,
    cl_command_queue   command_queue,
    cl_mem             image,
    const void *       fill_color,
    const size_t       origin[3],
    const size_t       region[3],
    cl_uint            num_events_in_wait_list,
    const cl_event *   event_wait_list,
    cl_event *         event) CL_API_SUFFIX__VERSION_1_2;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueCopyImage)(
    KHRpfn_clEnqueueCopyImage next,
    cl_command_queue     command_queue,
    cl_mem               src_image,
    cl_mem               dst_image, 
    const size_t *       src_origin,
    const size_t *       dst_origin,
    const size_t *       region, 
    cl_uint              num_events_in_wait_list,
    const cl_event *     event_wait_list,
    cl_event *           event) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueCopyImageToBuffer)(
    KHRpfn_clEnqueueCopyImageToBuffer next,
    cl_command_queue command_queue,
    cl_mem           src_image,
    cl_mem           dst_buffer, 
    const size_t *   src_origin,
    const size_t *   region, 
    size_t           dst_offset,
    cl_uint          num_events_in_wait_list,
    const cl_event * event_wait_list,
    cl_event *       event) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueCopyBufferToImage)(
    KHRpfn_clEnqueueCopyBufferToImage next,
    cl_command_queue command_queue,
    cl_mem           src_buffer,
    cl_mem           dst_image, 
    size_t           src_offset,
    const size_t *   dst_origin,
    const size_t *   region, 
    cl_uint          num_events_in_wait_list,
    const cl_event * event_wait_list,
    cl_event *       event) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY void * (CL_API_CALL *pfn_detour_clEnqueueMapBuffer)(
    KHRpfn_clEnqueueMapBuffer next,
    cl_command_queue command_queue,
    cl_mem           buffer,
    cl_bool          blocking_map, 
    cl_map_flags     map_flags,
    size_t           offset,
    size_t           cb,
    cl_uint          num_events_in_wait_list,
    const cl_event * event_wait_list,
    cl_event *       event,
    cl_int *         errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY void * (CL_API_CALL *pfn_detour_clEnqueueMapImage)(
    KHRpfn_clEnqueueMapImage next,
    cl_command_queue  command_queue,
    cl_mem            image, 
    cl_bool           blocking_map, 
    cl_map_flags      map_flags, 
    const size_t *    origin,
    const size_t *    region,
    size_t *          image_row_pitch,
    size_t *          image_slice_pitch,
    cl_uint           num_events_in_wait_list,
    const cl_event *  event_wait_list,
    cl_event *        event,
    cl_int *          errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueUnmapMemObject)(
    KHRpfn_clEnqueueUnmapMemObject next,
    cl_command_queue command_queue,
    cl_mem           memobj,
    void *           mapped_ptr,
    cl_uint          num_events_in_wait_list,
    const cl_event *  event_wait_list,
    cl_event *        event) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueMigrateMemObjects)(
    KHRpfn_clEnqueueMigrateMemObjects next,
    cl_command_queue       command_queue,
    cl_uint                num_mem_objects,
    const cl_mem *         mem_objects,
    cl_mem_migration_flags flags,
    cl_uint                num_events_in_wait_list,
    const cl_event *       event_wait_list,
    cl_event *             event) CL_API_SUFFIX__VERSION_1_2;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueNDRangeKernel)(
    KHRpfn_clEnqueueNDRangeKernel next,
    cl_command_queue command_queue,
    cl_kernel        kernel,
    cl_uint          work_dim,
    const size_t *   global_work_offset,
    const size_t *   global_work_size,
    const size_t *   local_work_size,
    cl_uint          num_events_in_wait_list,
    const cl_event * event_wait_list,
    cl_event *       event) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueTask)(
    KHRpfn_clEnqueueTask next,
    cl_command_queue  command_queue,
    cl_kernel         kernel,
    cl_uint           num_events_in_wait_list,
    const cl_event *  event_wait_list,
    cl_event *        event) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueNativeKernel)(
    KHRpfn_clEnqueueNativeKernel next,
    cl_command_queue  command_queue,
    void (CL_CALLBACK * user_func)(
        void *),
    void *            args,
    size_t            cb_args, 
    cl_uint           num_mem_objects,
    const cl_mem *    mem_list,
    const void **     args_mem_loc,
    cl_uint           num_events_in_wait_list,
    const cl_event *  event_wait_list,
    cl_event *        event) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueMarkerWithWaitList)(
    KHRpfn_clEnqueueMarkerWithWaitList next,
    cl_command_queue  command_queue,
    cl_uint           num_events_in_wait_list,
    const cl_event *  event_wait_list,
    cl_event *        event) CL_API_SUFFIX__VERSION_1_2;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueBarrierWithWaitList)(
    KHRpfn_clEnqueueBarrierWithWaitList next,
    cl_command_queue  command_queue,
    cl_uint           num_events_in_wait_list,
    const cl_event *  event_wait_list,
    cl_event *        event) CL_API_SUFFIX__VERSION_1_2;

typedef CL_API_ENTRY void * (CL_API_CALL *pfn_detour_clGetExtensionFunctionAddressForPlatform)(
    KHRpfn_clGetExtensionFunctionAddressForPlatform next,
    cl_platform_id platform,
    const char *   function_name) CL_API_SUFFIX__VERSION_1_2;

// Shared Virtual Memory APIs
typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueSVMFree)(
    KHRpfn_clEnqueueSVMFree next,
    cl_command_queue /* command_queue */,
    cl_uint          /* num_svm_pointers */,
    void **          /* svm_pointers */,
    void (CL_CALLBACK *pfn_free_func)(
        cl_command_queue /* queue */,
        cl_uint          /* num_svm_pointers */,
        void **          /* svm_pointers[] */,
        void *           /* user_data */),
    void *           /* user_data */,
    cl_uint          /* num_events_in_wait_list */,
    const cl_event * /* event_wait_list */,
    cl_event * /* event */) CL_API_SUFFIX__VERSION_2_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueSVMMemcpy)(
    KHRpfn_clEnqueueSVMMemcpy next,
    cl_command_queue /* command_queue */,
    cl_bool          /* blocking_copy */,
    void *           /* dst_ptr */,
    const void *     /* src_ptr */,
    size_t           /* size */,
    cl_uint          /* num_events_in_wait_list */,
    const cl_event * /* event_wait_list */,
    cl_event *       /* event */) CL_API_SUFFIX__VERSION_2_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueSVMMemFill)(
    KHRpfn_clEnqueueSVMMemFill next,
    cl_command_queue /* command_queue */,
    void *           /* svm_ptr */,
    const void *     /* pattern */,
    size_t           /* pattern_size */,
    size_t           /* size */,
    cl_uint          /* num_events_in_wait_list */,
    const cl_event * /* event_wait_list */,
    cl_event *       /* event */) CL_API_SUFFIX__VERSION_2_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueSVMMap)(
    KHRpfn_clEnqueueSVMMap next,
    cl_command_queue /* command_queue */,
    cl_bool          /* blocking_map */,
    cl_map_flags     /* map_flags */,
    void *           /* svm_ptr */,
    size_t           /* size */,
    cl_uint          /* num_events_in_wait_list */,
    const cl_event * /* event_wait_list */,
    cl_event *       /* event */) CL_API_SUFFIX__VERSION_2_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueSVMUnmap)(
    KHRpfn_clEnqueueSVMUnmap next,
    cl_command_queue /* command_queue */,
    void *           /* svm_ptr */,
    cl_uint          /* num_events_in_wait_list */,
    const cl_event * /* event_wait_list */,
    cl_event *       /* event */) CL_API_SUFFIX__VERSION_2_0;

// Deprecated APIs
typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clSetCommandQueueProperty)(
    KHRpfn_clSetCommandQueueProperty next,
    cl_command_queue              command_queue,
    cl_command_queue_properties   properties, 
    cl_bool                       enable,
    cl_command_queue_properties * old_properties) CL_EXT_SUFFIX__VERSION_1_0_DEPRECATED;

typedef CL_API_ENTRY cl_mem (CL_API_CALL *pfn_detour_clCreateImage2D)(
    KHRpfn_clCreateImage2D next,
    cl_context              context,
    cl_mem_flags            flags,
    const cl_image_format * image_format,
    size_t                  image_width,
    size_t                  image_height,
    size_t                  image_row_pitch, 
    void *                  host_ptr,
    cl_int *                errcode_ret) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED;
                        
typedef CL_API_ENTRY cl_mem (CL_API_CALL *pfn_detour_clCreateImage3D)(
    KHRpfn_clCreateImage3D next,
    cl_context              context,
    cl_mem_flags            flags,
    const cl_image_format * image_format,
    size_t                  image_width, 
    size_t                  image_height,
    size_t                  image_depth, 
    size_t                  image_row_pitch, 
    size_t                  image_slice_pitch, 
    void *                  host_ptr,
    cl_int *                errcode_ret) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clUnloadCompiler)(
    KHRpfn_clUnloadCompiler next) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueMarker)(
    KHRpfn_clEnqueueMarker next,
    cl_command_queue    command_queue,
    cl_event *          event) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueWaitForEvents)(
    KHRpfn_clEnqueueWaitForEvents next,
    cl_command_queue command_queue,
    cl_uint          num_events,
    const cl_event * event_list) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueBarrier)(
    KHRpfn_clEnqueueBarrier next,cl_command_queue command_queue) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED;

typedef CL_API_ENTRY void * (CL_API_CALL *pfn_detour_clGetExtensionFunctionAddress)(
    KHRpfn_clGetExtensionFunctionAddress next,const char *function_name) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED;

// GL and other APIs
typedef CL_API_ENTRY cl_mem (CL_API_CALL *pfn_detour_clCreateFromGLBuffer)(
    KHRpfn_clCreateFromGLBuffer next,
    cl_context    context,
    cl_mem_flags  flags,
    GLuint        bufobj,
    int *         errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_mem (CL_API_CALL *pfn_detour_clCreateFromGLTexture)(
    KHRpfn_clCreateFromGLTexture next,
    cl_context      context,
    cl_mem_flags    flags,
    cl_GLenum       target,
    cl_GLint        miplevel,
    cl_GLuint       texture,
    cl_int *        errcode_ret) CL_API_SUFFIX__VERSION_1_2;

typedef CL_API_ENTRY cl_mem (CL_API_CALL *pfn_detour_clCreateFromGLTexture2D)(
    KHRpfn_clCreateFromGLTexture2D next,
    cl_context      context,
    cl_mem_flags    flags,
    GLenum          target,
    GLint           miplevel,
    GLuint          texture,
    cl_int *        errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_mem (CL_API_CALL *pfn_detour_clCreateFromGLTexture3D)(
    KHRpfn_clCreateFromGLTexture3D next,
    cl_context      context,
    cl_mem_flags    flags,
    GLenum          target,
    GLint           miplevel,
    GLuint          texture,
    cl_int *        errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_mem (CL_API_CALL *pfn_detour_clCreateFromGLRenderbuffer)(
    KHRpfn_clCreateFromGLRenderbuffer next,
    cl_context           context,
    cl_mem_flags         flags,
    GLuint               renderbuffer,
    cl_int *             errcode_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clGetGLObjectInfo)(
    KHRpfn_clGetGLObjectInfo next,
    cl_mem               memobj,
    cl_gl_object_type *  gl_object_type,
    GLuint *             gl_object_name) CL_API_SUFFIX__VERSION_1_0;
                  
typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clGetGLTextureInfo)(
    KHRpfn_clGetGLTextureInfo next,
    cl_mem               memobj,
    cl_gl_texture_info   param_name,
    size_t               param_value_size,
    void *               param_value,
    size_t *             param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueAcquireGLObjects)(
    KHRpfn_clEnqueueAcquireGLObjects next,
    cl_command_queue     command_queue,
    cl_uint              num_objects,
    const cl_mem *       mem_objects,
    cl_uint              num_events_in_wait_list,
    const cl_event *     event_wait_list,
    cl_event *           event) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueReleaseGLObjects)(
    KHRpfn_clEnqueueReleaseGLObjects next,
    cl_command_queue     command_queue,
    cl_uint              num_objects,
    const cl_mem *       mem_objects,
    cl_uint              num_events_in_wait_list,
    const cl_event *     event_wait_list,
    cl_event *           event) CL_API_SUFFIX__VERSION_1_0;

/* cl_khr_gl_sharing */
typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clGetGLContextInfoKHR)(
    KHRpfn_clGetGLContextInfoKHR next,
    const cl_context_properties *properties,
    cl_gl_context_info param_name,
    size_t param_value_size,
    void *param_value,
    size_t *param_value_size_ret);

/* cl_khr_gl_event */
typedef CL_API_ENTRY cl_event (CL_API_CALL *pfn_detour_clCreateEventFromGLsyncKHR)(
    KHRpfn_clCreateEventFromGLsyncKHR next,
    cl_context context,
    cl_GLsync sync,
    cl_int *errcode_ret);

#if defined(_WIN32)
    // TODO
#else

/* cl_khr_d3d10_sharing */
typedef void *KHRpfn_clGetDeviceIDsFromD3D10KHR;
typedef void *KHRpfn_clCreateFromD3D10BufferKHR;
typedef void *KHRpfn_clCreateFromD3D10Texture2DKHR;
typedef void *KHRpfn_clCreateFromD3D10Texture3DKHR;
typedef void *KHRpfn_clEnqueueAcquireD3D10ObjectsKHR;
typedef void *KHRpfn_clEnqueueReleaseD3D10ObjectsKHR;

/* cl_khr_d3d11_sharing */
typedef void *KHRpfn_clGetDeviceIDsFromD3D11KHR;
typedef void *KHRpfn_clCreateFromD3D11BufferKHR;
typedef void *KHRpfn_clCreateFromD3D11Texture2DKHR;
typedef void *KHRpfn_clCreateFromD3D11Texture3DKHR;
typedef void *KHRpfn_clEnqueueAcquireD3D11ObjectsKHR;
typedef void *KHRpfn_clEnqueueReleaseD3D11ObjectsKHR;

/* cl_khr_dx9_media_sharing */
typedef void *KHRpfn_clCreateFromDX9MediaSurfaceKHR;
typedef void *KHRpfn_clEnqueueAcquireDX9MediaSurfacesKHR;
typedef void *KHRpfn_clEnqueueReleaseDX9MediaSurfacesKHR;
typedef void *KHRpfn_clGetDeviceIDsFromDX9MediaAdapterKHR;

#endif // _WIN32

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clSetEventCallback)(
    KHRpfn_clSetEventCallback next,
    cl_event            /* event */,
    cl_int              /* command_exec_callback_type */,
    void (CL_CALLBACK * /* pfn_notify */)(
    cl_event, cl_int, void *),
    void *              /* user_data */) CL_API_SUFFIX__VERSION_1_1;

typedef CL_API_ENTRY cl_mem (CL_API_CALL *pfn_detour_clCreateSubBuffer)(
    KHRpfn_clCreateSubBuffer next,
    cl_mem                   /* buffer */,
    cl_mem_flags             /* flags */,
    cl_buffer_create_type    /* buffer_create_type */,
    const void *             /* buffer_create_info */,
    cl_int *                 /* errcode_ret */) CL_API_SUFFIX__VERSION_1_1;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clSetMemObjectDestructorCallback)(
    KHRpfn_clSetMemObjectDestructorCallback next,
    cl_mem /* memobj */, 
    void (CL_CALLBACK * /*pfn_notify*/)(
    cl_mem /* memobj */, void* /*user_data*/), 
    void * /*user_data */ ) CL_API_SUFFIX__VERSION_1_1;

typedef CL_API_ENTRY cl_event (CL_API_CALL *pfn_detour_clCreateUserEvent)(
    KHRpfn_clCreateUserEvent next,
    cl_context    /* context */,
    cl_int *      /* errcode_ret */) CL_API_SUFFIX__VERSION_1_1;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clSetUserEventStatus)(
    KHRpfn_clSetUserEventStatus next,
    cl_event   /* event */,
    cl_int     /* execution_status */) CL_API_SUFFIX__VERSION_1_1;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clCreateSubDevicesEXT)(
    KHRpfn_clCreateSubDevicesEXT next,
    cl_device_id     in_device,
    const cl_device_partition_property_ext * partition_properties,
    cl_uint          num_entries,
    cl_device_id *   out_devices,
    cl_uint *        num_devices);

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clRetainDeviceEXT)(
    KHRpfn_clRetainDeviceEXT next,
    cl_device_id     device) CL_API_SUFFIX__VERSION_1_0;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clReleaseDeviceEXT)(
    KHRpfn_clReleaseDeviceEXT next,
    cl_device_id     device) CL_API_SUFFIX__VERSION_1_0;

/* cl_khr_egl_image */
typedef CL_API_ENTRY cl_mem (CL_API_CALL *pfn_detour_clCreateFromEGLImageKHR)(
    KHRpfn_clCreateFromEGLImageKHR next,
    cl_context context,
    CLeglDisplayKHR display,
    CLeglImageKHR image,
    cl_mem_flags flags,
    const cl_egl_image_properties_khr *properties,
    cl_int *errcode_ret);

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueAcquireEGLObjectsKHR)(
    KHRpfn_clEnqueueAcquireEGLObjectsKHR next,
    cl_command_queue command_queue,
    cl_uint num_objects,
    const cl_mem *mem_objects,
    cl_uint num_events_in_wait_list,
    const cl_event *event_wait_list,
    cl_event *event);

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueReleaseEGLObjectsKHR)(
    KHRpfn_clEnqueueReleaseEGLObjectsKHR next,
    cl_command_queue command_queue,
    cl_uint num_objects,
    const cl_mem *mem_objects,
    cl_uint num_events_in_wait_list,
    const cl_event *event_wait_list,
    cl_event *event);

/* cl_khr_egl_event */
typedef CL_API_ENTRY cl_event (CL_API_CALL *pfn_detour_clCreateEventFromEGLSyncKHR)(
    KHRpfn_clCreateEventFromEGLSyncKHR next,
    cl_context context,
    CLeglSyncKHR sync,
    CLeglDisplayKHR display,
    cl_int *errcode_ret);

/*typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clSetDefaultDeviceCommandQueue)(
    KHRpfn_clSetDefaultDeviceCommandQueue next,
    cl_context context,
    cl_device_id device,
    cl_command_queue command_queue) CL_API_SUFFIX__VERSION_2_1;

typedef CL_API_ENTRY cl_program (CL_API_CALL *pfn_detour_clCreateProgramWithIL)(
    KHRpfn_clCreateProgramWithIL next,
    cl_context context,
    const void * il,
    size_t length,
    cl_int * errcode_ret) CL_API_SUFFIX__VERSION_2_1;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clGetKernelSubGroupInfo )(
    KHRpfn_clGetKernelSubGroupInfo  next,
    cl_kernel kernel,
    cl_device_id device,
    cl_kernel_sub_group_info param_name,
    size_t input_value_size,
    const void * input_value,
    size_t param_value_size,
    void * param_value,
    size_t * param_value_size_ret) CL_API_SUFFIX__VERSION_2_1;

typedef CL_API_ENTRY cl_kernel (CL_API_CALL *pfn_detour_clCloneKernel)(
    KHRpfn_clCloneKernel next,
    cl_kernel source_kernel, 
    cl_int * errcode_ret) CL_API_SUFFIX__VERSION_2_1;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clEnqueueSVMMigrateMem)(
    KHRpfn_clEnqueueSVMMigrateMem next,
    cl_command_queue command_queue,
    cl_uint num_svm_pointers,
    const void ** svm_pointers,
    const size_t * sizes,
    cl_mem_migration_flags flags,
    cl_uint num_events_in_wait_list,
    const cl_event * event_wait_list,
    cl_event * event) CL_API_SUFFIX__VERSION_2_1;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clGetDeviceAndHostTimer)(
    KHRpfn_clGetDeviceAndHostTimer next,
    cl_device_id device,
    cl_ulong * device_timestamp,
    cl_ulong * host_timestamp) CL_API_SUFFIX__VERSION_2_1;

typedef CL_API_ENTRY cl_int (CL_API_CALL *pfn_detour_clGetHostTimer)(
    KHRpfn_clGetHostTimer next,
    cl_device_id device,
    cl_ulong * host_timestamp) CL_API_SUFFIX__VERSION_2_1; */

/*
 *    Detour setter functions:
 */
void detour_install_clGetPlatformIDs(pfn_detour_clGetPlatformIDs func);
void detour_install_clGetPlatformInfo(pfn_detour_clGetPlatformInfo func);
void detour_install_clGetDeviceIDs(pfn_detour_clGetDeviceIDs func);
void detour_install_clGetDeviceInfo(pfn_detour_clGetDeviceInfo func);
void detour_install_clCreateSubDevices(pfn_detour_clCreateSubDevices func);
void detour_install_clRetainDevice(pfn_detour_clRetainDevice func);
void detour_install_clReleaseDevice(pfn_detour_clReleaseDevice func);
void detour_install_clCreateContext(pfn_detour_clCreateContext func);
void detour_install_clCreateContextFromType(pfn_detour_clCreateContextFromType func);
void detour_install_clRetainContext(pfn_detour_clRetainContext func);
void detour_install_clReleaseContext(pfn_detour_clReleaseContext func);
void detour_install_clGetContextInfo(pfn_detour_clGetContextInfo func);
void detour_install_clCreateCommandQueue(pfn_detour_clCreateCommandQueue func);
void detour_install_clCreateCommandQueueWithProperties(pfn_detour_clCreateCommandQueueWithProperties func);
void detour_install_clRetainCommandQueue(pfn_detour_clRetainCommandQueue func);
void detour_install_clReleaseCommandQueue(pfn_detour_clReleaseCommandQueue func);
void detour_install_clGetCommandQueueInfo(pfn_detour_clGetCommandQueueInfo func);
void detour_install_clCreateBuffer(pfn_detour_clCreateBuffer func);
void detour_install_clCreateImage(pfn_detour_clCreateImage func);
void detour_install_clRetainMemObject(pfn_detour_clRetainMemObject func);
void detour_install_clReleaseMemObject(pfn_detour_clReleaseMemObject func);
void detour_install_clGetSupportedImageFormats(pfn_detour_clGetSupportedImageFormats func);
void detour_install_clGetImageInfo(pfn_detour_clGetImageInfo func);
void detour_install_clGetMemObjectInfo(pfn_detour_clGetMemObjectInfo func);
void detour_install_clCreatePipe(pfn_detour_clCreatePipe func);
void detour_install_clGetPipeInfo(pfn_detour_clGetPipeInfo func);
void detour_install_clSVMAlloc(pfn_detour_clSVMAlloc func);
void detour_install_clSVMFree(pfn_detour_clSVMFree func);
void detour_install_clCreateSampler(pfn_detour_clCreateSampler func);
void detour_install_clRetainSampler(pfn_detour_clRetainSampler func);
void detour_install_clReleaseSampler(pfn_detour_clReleaseSampler func);
void detour_install_clGetSamplerInfo(pfn_detour_clGetSamplerInfo func);
void detour_install_clCreateSamplerWithProperties(pfn_detour_clCreateSamplerWithProperties func);
void detour_install_clCreateProgramWithSource(pfn_detour_clCreateProgramWithSource func);
void detour_install_clCreateProgramWithBinary(pfn_detour_clCreateProgramWithBinary func);
void detour_install_clCreateProgramWithBuiltInKernels(pfn_detour_clCreateProgramWithBuiltInKernels func);
void detour_install_clRetainProgram(pfn_detour_clRetainProgram func);
void detour_install_clReleaseProgram(pfn_detour_clReleaseProgram func);
void detour_install_clBuildProgram(pfn_detour_clBuildProgram func);
void detour_install_clCompileProgram(pfn_detour_clCompileProgram func);
void detour_install_clLinkProgram(pfn_detour_clLinkProgram func);
//void detour_install_clSetProgramSpecializationConstant(pfn_detour_clSetProgramSpecializationConstant func);
//void detour_install_clSetProgramReleaseCallback(pfn_detour_clSetProgramReleaseCallback func);
void detour_install_clUnloadPlatformCompiler(pfn_detour_clUnloadPlatformCompiler func);
void detour_install_clGetProgramInfo(pfn_detour_clGetProgramInfo func);
void detour_install_clGetProgramBuildInfo(pfn_detour_clGetProgramBuildInfo func);
void detour_install_clCreateKernelsInProgram(pfn_detour_clCreateKernelsInProgram func);
void detour_install_clCreateKernel(pfn_detour_clCreateKernel func);
void detour_install_clRetainKernel(pfn_detour_clRetainKernel func);
void detour_install_clReleaseKernel(pfn_detour_clReleaseKernel func);
void detour_install_clSetKernelArg(pfn_detour_clSetKernelArg func);
void detour_install_clGetKernelInfo(pfn_detour_clGetKernelInfo func);
void detour_install_clGetKernelArgInfo(pfn_detour_clGetKernelArgInfo func);
void detour_install_clGetKernelWorkGroupInfo(pfn_detour_clGetKernelWorkGroupInfo func);
void detour_install_clSetKernelArgSVMPointer(pfn_detour_clSetKernelArgSVMPointer func);
void detour_install_clSetKernelExecInfo(pfn_detour_clSetKernelExecInfo func);
void detour_install_clGetKernelSubGroupInfoKHR(pfn_detour_clGetKernelSubGroupInfoKHR func);
void detour_install_clWaitForEvents(pfn_detour_clWaitForEvents func);
void detour_install_clGetEventInfo(pfn_detour_clGetEventInfo func);
void detour_install_clReleaseEvent(pfn_detour_clReleaseEvent func);
void detour_install_clRetainEvent(pfn_detour_clRetainEvent func);
void detour_install_clGetEventProfilingInfo(pfn_detour_clGetEventProfilingInfo func);
void detour_install_clFlush(pfn_detour_clFlush func);
void detour_install_clFinish(pfn_detour_clFinish func);
void detour_install_clEnqueueReadBuffer(pfn_detour_clEnqueueReadBuffer func);
void detour_install_clEnqueueReadBufferRect(pfn_detour_clEnqueueReadBufferRect func);
void detour_install_clEnqueueWriteBuffer(pfn_detour_clEnqueueWriteBuffer func);
void detour_install_clEnqueueWriteBufferRect(pfn_detour_clEnqueueWriteBufferRect func);
void detour_install_clEnqueueFillBuffer(pfn_detour_clEnqueueFillBuffer func);
void detour_install_clEnqueueCopyBuffer(pfn_detour_clEnqueueCopyBuffer func);
void detour_install_clEnqueueCopyBufferRect(pfn_detour_clEnqueueCopyBufferRect func);
void detour_install_clEnqueueReadImage(pfn_detour_clEnqueueReadImage func);
void detour_install_clEnqueueWriteImage(pfn_detour_clEnqueueWriteImage func);
void detour_install_clEnqueueFillImage(pfn_detour_clEnqueueFillImage func);
void detour_install_clEnqueueCopyImage(pfn_detour_clEnqueueCopyImage func);
void detour_install_clEnqueueCopyImageToBuffer(pfn_detour_clEnqueueCopyImageToBuffer func);
void detour_install_clEnqueueCopyBufferToImage(pfn_detour_clEnqueueCopyBufferToImage func);
void detour_install_clEnqueueMapBuffer(pfn_detour_clEnqueueMapBuffer func);
void detour_install_clEnqueueMapImage(pfn_detour_clEnqueueMapImage func);
void detour_install_clEnqueueUnmapMemObject(pfn_detour_clEnqueueUnmapMemObject func);
void detour_install_clEnqueueMigrateMemObjects(pfn_detour_clEnqueueMigrateMemObjects func);
void detour_install_clEnqueueNDRangeKernel(pfn_detour_clEnqueueNDRangeKernel func);
void detour_install_clEnqueueTask(pfn_detour_clEnqueueTask func);
void detour_install_clEnqueueNativeKernel(pfn_detour_clEnqueueNativeKernel func);
void detour_install_clEnqueueMarkerWithWaitList(pfn_detour_clEnqueueMarkerWithWaitList func);
void detour_install_clEnqueueBarrierWithWaitList(pfn_detour_clEnqueueBarrierWithWaitList func);
void detour_install_clGetExtensionFunctionAddressForPlatform(pfn_detour_clGetExtensionFunctionAddressForPlatform func);
void detour_install_clEnqueueSVMFree(pfn_detour_clEnqueueSVMFree func);
void detour_install_clEnqueueSVMMemcpy(pfn_detour_clEnqueueSVMMemcpy func);
void detour_install_clEnqueueSVMMemFill(pfn_detour_clEnqueueSVMMemFill func);
void detour_install_clEnqueueSVMMap(pfn_detour_clEnqueueSVMMap func);
void detour_install_clEnqueueSVMUnmap(pfn_detour_clEnqueueSVMUnmap func);
void detour_install_clSetCommandQueueProperty(pfn_detour_clSetCommandQueueProperty func);
void detour_install_clCreateImage2D(pfn_detour_clCreateImage2D func);
void detour_install_clCreateImage3D(pfn_detour_clCreateImage3D func);
void detour_install_clUnloadCompiler(pfn_detour_clUnloadCompiler func);
void detour_install_clEnqueueMarker(pfn_detour_clEnqueueMarker func);
void detour_install_clEnqueueWaitForEvents(pfn_detour_clEnqueueWaitForEvents func);
void detour_install_clEnqueueBarrier(pfn_detour_clEnqueueBarrier func);
void detour_install_clGetExtensionFunctionAddress(pfn_detour_clGetExtensionFunctionAddress func);
void detour_install_clCreateFromGLBuffer(pfn_detour_clCreateFromGLBuffer func);
void detour_install_clCreateFromGLTexture(pfn_detour_clCreateFromGLTexture func);
void detour_install_clCreateFromGLTexture2D(pfn_detour_clCreateFromGLTexture2D func);
void detour_install_clCreateFromGLTexture3D(pfn_detour_clCreateFromGLTexture3D func);
void detour_install_clCreateFromGLRenderbuffer(pfn_detour_clCreateFromGLRenderbuffer func);
void detour_install_clGetGLObjectInfo(pfn_detour_clGetGLObjectInfo func);
void detour_install_clGetGLTextureInfo(pfn_detour_clGetGLTextureInfo func);
void detour_install_clEnqueueAcquireGLObjects(pfn_detour_clEnqueueAcquireGLObjects func);
void detour_install_clEnqueueReleaseGLObjects(pfn_detour_clEnqueueReleaseGLObjects func);
void detour_install_clGetGLContextInfoKHR(pfn_detour_clGetGLContextInfoKHR func);
void detour_install_clCreateEventFromGLsyncKHR(pfn_detour_clCreateEventFromGLsyncKHR func);
void detour_install_clSetEventCallback(pfn_detour_clSetEventCallback func);
void detour_install_clCreateSubBuffer(pfn_detour_clCreateSubBuffer func);
void detour_install_clSetMemObjectDestructorCallback(pfn_detour_clSetMemObjectDestructorCallback func);
void detour_install_clCreateUserEvent(pfn_detour_clCreateUserEvent func);
void detour_install_clSetUserEventStatus(pfn_detour_clSetUserEventStatus func);
void detour_install_clCreateSubDevicesEXT(pfn_detour_clCreateSubDevicesEXT func);
void detour_install_clRetainDeviceEXT(pfn_detour_clRetainDeviceEXT func);
void detour_install_clReleaseDeviceEXT(pfn_detour_clReleaseDeviceEXT func);
void detour_install_clCreateFromEGLImageKHR(pfn_detour_clCreateFromEGLImageKHR func);
void detour_install_clEnqueueAcquireEGLObjectsKHR(pfn_detour_clEnqueueAcquireEGLObjectsKHR func);
void detour_install_clEnqueueReleaseEGLObjectsKHR(pfn_detour_clEnqueueReleaseEGLObjectsKHR func);
void detour_install_clCreateEventFromEGLSyncKHR(pfn_detour_clCreateEventFromEGLSyncKHR func);
//void detour_install_clSetDefaultDeviceCommandQueue(pfn_detour_clSetDefaultDeviceCommandQueue func);
//void detour_install_clCreateProgramWithIL(pfn_detour_clCreateProgramWithIL func);
//void detour_install_clGetKernelSubGroupInfo (pfn_detour_clGetKernelSubGroupInfo  func);
//void detour_install_clCloneKernel(pfn_detour_clCloneKernel func);
//void detour_install_clEnqueueSVMMigrateMem(pfn_detour_clEnqueueSVMMigrateMem func);
//void detour_install_clGetDeviceAndHostTimer(pfn_detour_clGetDeviceAndHostTimer func);
//void detour_install_clGetHostTimer(pfn_detour_clGetHostTimer func);


// Detour function declarations. These are called from icd_dispatch, and
// will in turn call their registered detour callbacks if they exist.

CL_API_ENTRY cl_int CL_API_CALL
detour_clGetPlatformIDs(KHRpfn_clGetPlatformIDs next,
                 cl_uint          num_entries,
                 cl_platform_id * platforms,
                 cl_uint *        num_platforms) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clGetPlatformInfo(KHRpfn_clGetPlatformInfo next,
    cl_platform_id   platform, 
    cl_platform_info param_name,
    size_t           param_value_size, 
    void *           param_value,
    size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clGetDeviceIDs(KHRpfn_clGetDeviceIDs next,
    cl_platform_id   platform,
    cl_device_type   device_type, 
    cl_uint          num_entries, 
    cl_device_id *   devices, 
    cl_uint *        num_devices) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clGetDeviceInfo(KHRpfn_clGetDeviceInfo next,
    cl_device_id    device,
    cl_device_info  param_name, 
    size_t          param_value_size, 
    void *          param_value,
    size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clCreateSubDevices(KHRpfn_clCreateSubDevices next,
    cl_device_id     in_device,
    const cl_device_partition_property * partition_properties,
    cl_uint          num_entries,
    cl_device_id *   out_devices,
    cl_uint *        num_devices);

CL_API_ENTRY cl_int CL_API_CALL
detour_clRetainDevice(KHRpfn_clRetainDevice next,
    cl_device_id     device) CL_API_SUFFIX__VERSION_1_2;

CL_API_ENTRY cl_int CL_API_CALL
detour_clReleaseDevice(KHRpfn_clReleaseDevice next,
    cl_device_id     device) CL_API_SUFFIX__VERSION_1_2;

CL_API_ENTRY cl_context CL_API_CALL
detour_clCreateContext(KHRpfn_clCreateContext next,
    const cl_context_properties * properties,
    cl_uint                 num_devices,
    const cl_device_id *    devices,
    void (CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *),
    void *                  user_data,
    cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_context CL_API_CALL
detour_clCreateContextFromType(KHRpfn_clCreateContextFromType next,
    const cl_context_properties * properties,
    cl_device_type          device_type,
    void (CL_CALLBACK *pfn_notify)(const char *, const void *, size_t, void *),
    void *                  user_data,
    cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clRetainContext(KHRpfn_clRetainContext next,
    cl_context context) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clReleaseContext(KHRpfn_clReleaseContext next,
    cl_context context) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clGetContextInfo(KHRpfn_clGetContextInfo next,
    cl_context         context, 
    cl_context_info    param_name, 
    size_t             param_value_size, 
    void *             param_value, 
    size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_command_queue CL_API_CALL
detour_clCreateCommandQueue(KHRpfn_clCreateCommandQueue next,
    cl_context                     context, 
    cl_device_id                   device, 
    cl_command_queue_properties    properties,
    cl_int *                       errcode_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_command_queue CL_API_CALL
detour_clCreateCommandQueueWithProperties(KHRpfn_clCreateCommandQueueWithProperties next,
    cl_context                  context,
    cl_device_id                device,
    const cl_queue_properties * properties,
    cl_int *                    errcode_ret) CL_API_SUFFIX__VERSION_2_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clRetainCommandQueue(KHRpfn_clRetainCommandQueue next,
    cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clReleaseCommandQueue(KHRpfn_clReleaseCommandQueue next,
    cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clGetCommandQueueInfo(KHRpfn_clGetCommandQueueInfo next,
    cl_command_queue      command_queue,
    cl_command_queue_info param_name,
    size_t                param_value_size,
    void *                param_value,
    size_t *              param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_mem CL_API_CALL
detour_clCreateBuffer(KHRpfn_clCreateBuffer next,
    cl_context   context,
    cl_mem_flags flags,
    size_t       size,
    void *       host_ptr,
    cl_int *     errcode_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_mem CL_API_CALL
detour_clCreateImage(KHRpfn_clCreateImage next,
    cl_context              context,
    cl_mem_flags            flags,
    const cl_image_format * image_format,
    const cl_image_desc *   image_desc,
    void *                  host_ptr,
    cl_int *                errcode_ret) CL_API_SUFFIX__VERSION_1_2;

CL_API_ENTRY cl_int CL_API_CALL
detour_clRetainMemObject(KHRpfn_clRetainMemObject next,
    cl_mem memobj) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clReleaseMemObject(KHRpfn_clReleaseMemObject next,
    cl_mem memobj) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clGetSupportedImageFormats(KHRpfn_clGetSupportedImageFormats next,
    cl_context           context,
    cl_mem_flags         flags,
    cl_mem_object_type   image_type,
    cl_uint              num_entries,
    cl_image_format *    image_formats,
    cl_uint *            num_image_formats) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clGetMemObjectInfo(KHRpfn_clGetMemObjectInfo next,
    cl_mem           memobj,
    cl_mem_info      param_name, 
    size_t           param_value_size,
    void *           param_value,
    size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clGetImageInfo(KHRpfn_clGetImageInfo next,
    cl_mem           image,
    cl_image_info    param_name, 
    size_t           param_value_size,
    void *           param_value,
    size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_mem CL_API_CALL
detour_clCreatePipe(KHRpfn_clCreatePipe next,
    cl_context                 context,
    cl_mem_flags               flags,
    cl_uint                    pipe_packet_size,
    cl_uint                    pipe_max_packets,
    const cl_pipe_properties * properties,
    cl_int *                   errcode_ret) CL_API_SUFFIX__VERSION_2_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clGetPipeInfo(KHRpfn_clGetPipeInfo next,
    cl_mem       pipe,
    cl_pipe_info param_name,
    size_t       param_value_size,
    void *       param_value,
    size_t *     param_value_size_ret) CL_API_SUFFIX__VERSION_2_0;

CL_API_ENTRY void * CL_API_CALL
detour_clSVMAlloc(KHRpfn_clSVMAlloc next,
    cl_context       context,
    cl_svm_mem_flags flags,
    size_t           size,
    unsigned int     alignment) CL_API_SUFFIX__VERSION_2_0;

CL_API_ENTRY void CL_API_CALL
detour_clSVMFree(KHRpfn_clSVMFree next,
    cl_context context,
    void *     svm_pointer) CL_API_SUFFIX__VERSION_2_0;

CL_API_ENTRY cl_sampler CL_API_CALL
detour_clCreateSampler(KHRpfn_clCreateSampler next,
    cl_context          context,
    cl_bool             normalized_coords, 
    cl_addressing_mode  addressing_mode, 
    cl_filter_mode      filter_mode,
    cl_int *            errcode_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clRetainSampler(KHRpfn_clRetainSampler next,
cl_sampler sampler) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clReleaseSampler(KHRpfn_clReleaseSampler next,
cl_sampler sampler) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clGetSamplerInfo(KHRpfn_clGetSamplerInfo next,
    cl_sampler         sampler,
    cl_sampler_info    param_name,
    size_t             param_value_size,
    void *             param_value,
    size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_sampler CL_API_CALL
detour_clCreateSamplerWithProperties(KHRpfn_clCreateSamplerWithProperties next,
    cl_context                    context,
    const cl_sampler_properties * sampler_properties,
    cl_int *                      errcode_ret) CL_API_SUFFIX__VERSION_2_0;

CL_API_ENTRY cl_program CL_API_CALL
detour_clCreateProgramWithSource(KHRpfn_clCreateProgramWithSource next,
    cl_context        context,
    cl_uint           count,
    const char **     strings,
    const size_t *    lengths,
    cl_int *          errcode_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_program CL_API_CALL
detour_clCreateProgramWithBinary(KHRpfn_clCreateProgramWithBinary next,
    cl_context                     context,
    cl_uint                        num_devices,
    const cl_device_id *           device_list,
    const size_t *                 lengths,
    const unsigned char **         binaries,
    cl_int *                       binary_status,
    cl_int *                       errcode_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_program CL_API_CALL
detour_clCreateProgramWithBuiltInKernels(KHRpfn_clCreateProgramWithBuiltInKernels next,
    cl_context            context,
    cl_uint               num_devices,
    const cl_device_id *  device_list,
    const char *          kernel_names,
    cl_int *              errcode_ret) CL_API_SUFFIX__VERSION_1_2;

CL_API_ENTRY cl_int CL_API_CALL
detour_clRetainProgram(KHRpfn_clRetainProgram next,
cl_program program) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clReleaseProgram(KHRpfn_clReleaseProgram next,
cl_program program) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clBuildProgram(KHRpfn_clBuildProgram next,
    cl_program           program,
    cl_uint              num_devices,
    const cl_device_id * device_list,
    const char *         options, 
    void (CL_CALLBACK *pfn_notify)(cl_program program, void * user_data),
    void *               user_data) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clCompileProgram(KHRpfn_clCompileProgram next,
    cl_program           program,
    cl_uint              num_devices,
    const cl_device_id * device_list,
    const char *         options,
    cl_uint              num_input_headers,
    const cl_program *   input_headers,
    const char **        header_include_names,
    void (CL_CALLBACK *  pfn_notify)(cl_program program, void * user_data),
    void *               user_data) CL_API_SUFFIX__VERSION_1_2;

CL_API_ENTRY cl_program CL_API_CALL
detour_clLinkProgram(KHRpfn_clLinkProgram next,
    cl_context           context,
    cl_uint              num_devices,
    const cl_device_id * device_list,
    const char *         options,
    cl_uint              num_input_programs,
    const cl_program *   input_programs,
    void (CL_CALLBACK *  pfn_notify)(cl_program program, void * user_data),
    void *               user_data,
    cl_int *             errcode_ret) CL_API_SUFFIX__VERSION_1_2;

/*CL_API_ENTRY cl_int CL_API_CALL
detour_clSetProgramSpecializationConstant(KHRpfn_clSetProgramSpecializationConstant next,
    cl_program           program,
    cl_uint              spec_id,
    size_t               spec_size,
    const void*          spec_value) CL_API_SUFFIX__VERSION_2_2;

CL_API_ENTRY cl_int CL_API_CALL
detour_clSetProgramReleaseCallback(KHRpfn_clSetProgramReleaseCallback next,
    cl_program           program,
    void (CL_CALLBACK *  pfn_notify)(cl_program program, void * user_data),
    void *               user_data) CL_API_SUFFIX__VERSION_2_2;*/

CL_API_ENTRY cl_int CL_API_CALL
detour_clUnloadPlatformCompiler(KHRpfn_clUnloadPlatformCompiler next,
    cl_platform_id     platform) CL_API_SUFFIX__VERSION_1_2;

CL_API_ENTRY cl_int CL_API_CALL
detour_clGetProgramInfo(KHRpfn_clGetProgramInfo next,
    cl_program         program,
    cl_program_info    param_name,
    size_t             param_value_size,
    void *             param_value,
    size_t *           param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clGetProgramBuildInfo(KHRpfn_clGetProgramBuildInfo next,
    cl_program            program,
    cl_device_id          device,
    cl_program_build_info param_name,
    size_t                param_value_size,
    void *                param_value,
    size_t *              param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_kernel CL_API_CALL
detour_clCreateKernel(KHRpfn_clCreateKernel next,
    cl_program      program,
    const char *    kernel_name,
    cl_int *        errcode_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clCreateKernelsInProgram(KHRpfn_clCreateKernelsInProgram next,
    cl_program     program,
    cl_uint        num_kernels,
    cl_kernel *    kernels,
    cl_uint *      num_kernels_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clRetainKernel(KHRpfn_clRetainKernel next,
cl_kernel    kernel) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clReleaseKernel(KHRpfn_clReleaseKernel next,
cl_kernel   kernel) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clSetKernelArg(KHRpfn_clSetKernelArg next,
    cl_kernel    kernel,
    cl_uint      arg_index,
    size_t       arg_size,
    const void * arg_value) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clGetKernelInfo(KHRpfn_clGetKernelInfo next,
    cl_kernel       kernel,
    cl_kernel_info  param_name,
    size_t          param_value_size,
    void *          param_value,
    size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clGetKernelArgInfo(KHRpfn_clGetKernelArgInfo next,
    cl_kernel       kernel,
    cl_uint         arg_indx,
    cl_kernel_arg_info  param_name,
    size_t          param_value_size,
    void *          param_value,
    size_t *        param_value_size_ret) CL_API_SUFFIX__VERSION_1_2;

CL_API_ENTRY cl_int CL_API_CALL
detour_clGetKernelWorkGroupInfo(KHRpfn_clGetKernelWorkGroupInfo next,
    cl_kernel                  kernel,
    cl_device_id               device,
    cl_kernel_work_group_info  param_name,
    size_t                     param_value_size,
    void *                     param_value,
    size_t *                   param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clSetKernelArgSVMPointer(KHRpfn_clSetKernelArgSVMPointer next,
    cl_kernel    kernel,
    cl_uint      arg_index,
    const void * arg_value) CL_API_SUFFIX__VERSION_2_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clSetKernelExecInfo(KHRpfn_clSetKernelExecInfo next,
    cl_kernel            kernel,
    cl_kernel_exec_info  param_name,
    size_t               param_value_size,
    const void *         param_value) CL_API_SUFFIX__VERSION_2_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clGetKernelSubGroupInfoKHR(KHRpfn_clGetKernelSubGroupInfoKHR next,
    cl_kernel                in_kernel,
    cl_device_id             /*in_device*/,
    cl_kernel_sub_group_info param_name,
    size_t                   /*input_value_size*/,
    const void *             /*input_value*/,
    size_t                   /*param_value_size*/,
    void*                    /*param_value*/,
    size_t*                  /*param_value_size_ret*/) CL_EXT_SUFFIX__VERSION_2_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clWaitForEvents(KHRpfn_clWaitForEvents next,
    cl_uint             num_events,
    const cl_event *    event_list) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clGetEventInfo(KHRpfn_clGetEventInfo next,
    cl_event         event,
    cl_event_info    param_name,
    size_t           param_value_size,
    void *           param_value,
    size_t *         param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clRetainEvent(KHRpfn_clRetainEvent next,
cl_event event) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clReleaseEvent(KHRpfn_clReleaseEvent next,
cl_event event) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clGetEventProfilingInfo(KHRpfn_clGetEventProfilingInfo next,
    cl_event            event,
    cl_profiling_info   param_name,
    size_t              param_value_size,
    void *              param_value,
    size_t *            param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clFlush(KHRpfn_clFlush next,
    cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clFinish(KHRpfn_clFinish next,
    cl_command_queue command_queue) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueReadBuffer(KHRpfn_clEnqueueReadBuffer next,
    cl_command_queue    command_queue,
    cl_mem              buffer,
    cl_bool             blocking_read,
    size_t              offset,
    size_t              cb, 
    void *              ptr,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueReadBufferRect(KHRpfn_clEnqueueReadBufferRect next,
    cl_command_queue    command_queue,
    cl_mem              buffer,
    cl_bool             blocking_read,
    const size_t *      buffer_origin,
    const size_t *      host_origin, 
    const size_t *      region,
    size_t              buffer_row_pitch,
    size_t              buffer_slice_pitch,
    size_t              host_row_pitch,
    size_t              host_slice_pitch,
    void *              ptr,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event) CL_API_SUFFIX__VERSION_1_1;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueWriteBuffer(KHRpfn_clEnqueueWriteBuffer next,
    cl_command_queue   command_queue, 
    cl_mem             buffer, 
    cl_bool            blocking_write, 
    size_t             offset, 
    size_t             cb, 
    const void *       ptr, 
    cl_uint            num_events_in_wait_list, 
    const cl_event *   event_wait_list, 
    cl_event *         event) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueWriteBufferRect(KHRpfn_clEnqueueWriteBufferRect next,
    cl_command_queue    command_queue,
    cl_mem              buffer,
    cl_bool             blocking_read,
    const size_t *      buffer_origin,
    const size_t *      host_origin, 
    const size_t *      region,
    size_t              buffer_row_pitch,
    size_t              buffer_slice_pitch,
    size_t              host_row_pitch,
    size_t              host_slice_pitch,    
    const void *        ptr,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event) CL_API_SUFFIX__VERSION_1_1;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueFillBuffer(KHRpfn_clEnqueueFillBuffer next,
    cl_command_queue   command_queue,
    cl_mem             buffer,
    const void *       pattern,
    size_t             pattern_size,
    size_t             offset,
    size_t             cb,
    cl_uint            num_events_in_wait_list,
    const cl_event *   event_wait_list,
    cl_event *         event) CL_API_SUFFIX__VERSION_1_2;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueCopyBuffer(KHRpfn_clEnqueueCopyBuffer next,
    cl_command_queue    command_queue, 
    cl_mem              src_buffer,
    cl_mem              dst_buffer, 
    size_t              src_offset,
    size_t              dst_offset,
    size_t              cb, 
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueCopyBufferRect(KHRpfn_clEnqueueCopyBufferRect next,
    cl_command_queue    command_queue, 
    cl_mem              src_buffer,
    cl_mem              dst_buffer, 
    const size_t *      src_origin,
    const size_t *      dst_origin,
    const size_t *      region,
    size_t              src_row_pitch,
    size_t              src_slice_pitch,
    size_t              dst_row_pitch,
    size_t              dst_slice_pitch,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event) CL_API_SUFFIX__VERSION_1_1;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueReadImage(KHRpfn_clEnqueueReadImage next,
    cl_command_queue     command_queue,
    cl_mem               image,
    cl_bool              blocking_read, 
    const size_t *       origin,
    const size_t *       region,
    size_t               row_pitch,
    size_t               slice_pitch, 
    void *               ptr,
    cl_uint              num_events_in_wait_list,
    const cl_event *     event_wait_list,
    cl_event *           event) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueWriteImage(KHRpfn_clEnqueueWriteImage next,
    cl_command_queue    command_queue,
    cl_mem              image,
    cl_bool             blocking_write, 
    const size_t *      origin,
    const size_t *      region,
    size_t              input_row_pitch,
    size_t              input_slice_pitch, 
    const void *        ptr,
    cl_uint             num_events_in_wait_list,
    const cl_event *    event_wait_list,
    cl_event *          event) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueFillImage(KHRpfn_clEnqueueFillImage next,
    cl_command_queue   command_queue,
    cl_mem             image,
    const void *       fill_color,
    const size_t       origin[3],
    const size_t       region[3],
    cl_uint            num_events_in_wait_list,
    const cl_event *   event_wait_list,
    cl_event *         event) CL_API_SUFFIX__VERSION_1_2;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueCopyImage(KHRpfn_clEnqueueCopyImage next,
    cl_command_queue     command_queue,
    cl_mem               src_image,
    cl_mem               dst_image, 
    const size_t *       src_origin,
    const size_t *       dst_origin,
    const size_t *       region, 
    cl_uint              num_events_in_wait_list,
    const cl_event *     event_wait_list,
    cl_event *           event) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueCopyImageToBuffer(KHRpfn_clEnqueueCopyImageToBuffer next,
    cl_command_queue command_queue,
    cl_mem           src_image,
    cl_mem           dst_buffer, 
    const size_t *   src_origin,
    const size_t *   region, 
    size_t           dst_offset,
    cl_uint          num_events_in_wait_list,
    const cl_event * event_wait_list,
    cl_event *       event) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueCopyBufferToImage(KHRpfn_clEnqueueCopyBufferToImage next,
    cl_command_queue command_queue,
    cl_mem           src_buffer,
    cl_mem           dst_image, 
    size_t           src_offset,
    const size_t *   dst_origin,
    const size_t *   region, 
    cl_uint          num_events_in_wait_list,
    const cl_event * event_wait_list,
    cl_event *       event) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY void * CL_API_CALL
detour_clEnqueueMapBuffer(KHRpfn_clEnqueueMapBuffer next,
    cl_command_queue command_queue,
    cl_mem           buffer,
    cl_bool          blocking_map, 
    cl_map_flags     map_flags,
    size_t           offset,
    size_t           cb,
    cl_uint          num_events_in_wait_list,
    const cl_event * event_wait_list,
    cl_event *       event,
    cl_int *         errcode_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY void * CL_API_CALL
detour_clEnqueueMapImage(KHRpfn_clEnqueueMapImage next,
    cl_command_queue  command_queue,
    cl_mem            image, 
    cl_bool           blocking_map, 
    cl_map_flags      map_flags, 
    const size_t *    origin,
    const size_t *    region,
    size_t *          image_row_pitch,
    size_t *          image_slice_pitch,
    cl_uint           num_events_in_wait_list,
    const cl_event *  event_wait_list,
    cl_event *        event,
    cl_int *          errcode_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueUnmapMemObject(KHRpfn_clEnqueueUnmapMemObject next,
    cl_command_queue command_queue,
    cl_mem           memobj,
    void *           mapped_ptr,
    cl_uint          num_events_in_wait_list,
    const cl_event *  event_wait_list,
    cl_event *        event) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueMigrateMemObjects(KHRpfn_clEnqueueMigrateMemObjects next,
    cl_command_queue       command_queue,
    cl_uint                num_mem_objects,
    const cl_mem *         mem_objects,
    cl_mem_migration_flags flags,
    cl_uint                num_events_in_wait_list,
    const cl_event *       event_wait_list,
    cl_event *             event) CL_API_SUFFIX__VERSION_1_2;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueNDRangeKernel(KHRpfn_clEnqueueNDRangeKernel next,
    cl_command_queue command_queue,
    cl_kernel        kernel,
    cl_uint          work_dim,
    const size_t *   global_work_offset,
    const size_t *   global_work_size,
    const size_t *   local_work_size,
    cl_uint          num_events_in_wait_list,
    const cl_event * event_wait_list,
    cl_event *       event) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueTask(KHRpfn_clEnqueueTask next,
    cl_command_queue  command_queue,
    cl_kernel         kernel,
    cl_uint           num_events_in_wait_list,
    const cl_event *  event_wait_list,
    cl_event *        event) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueNativeKernel(KHRpfn_clEnqueueNativeKernel next,
    cl_command_queue  command_queue,
    void (CL_CALLBACK * user_func)(void *),
    void *            args,
    size_t            cb_args, 
    cl_uint           num_mem_objects,
    const cl_mem *    mem_list,
    const void **     args_mem_loc,
    cl_uint           num_events_in_wait_list,
    const cl_event *  event_wait_list,
    cl_event *        event) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueMarkerWithWaitList(KHRpfn_clEnqueueMarkerWithWaitList next,
    cl_command_queue  command_queue,
    cl_uint           num_events_in_wait_list,
    const cl_event *  event_wait_list,
    cl_event *        event) CL_API_SUFFIX__VERSION_1_2;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueBarrierWithWaitList(KHRpfn_clEnqueueBarrierWithWaitList next,
    cl_command_queue  command_queue,
    cl_uint           num_events_in_wait_list,
    const cl_event *  event_wait_list,
    cl_event *        event) CL_API_SUFFIX__VERSION_1_2;

CL_API_ENTRY void CL_API_CALL
detour_clGetExtensionFunctionAddressForPlatform(KHRpfn_clGetExtensionFunctionAddressForPlatform next,
    cl_platform_id platform,
    const char *   function_name) CL_API_SUFFIX__VERSION_1_2;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueSVMFree(KHRpfn_clEnqueueSVMFree next,
    cl_command_queue command_queue,
    cl_uint          num_svm_pointers,
    void* svm_pointers[],
    void (CL_CALLBACK* pfn_free_func)(
        cl_command_queue queue,
        cl_uint num_svm_pointers,
        void* svm_pointers[],
        void* user_data),
    void* user_data,
    cl_uint          num_events_in_wait_list,
    const cl_event * event_wait_list,
    cl_event * event) CL_API_SUFFIX__VERSION_2_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueSVMMemcpy(KHRpfn_clEnqueueSVMMemcpy next,
    cl_command_queue command_queue,
    cl_bool          blocking_copy,
    void *           dst_ptr,
    const void *     src_ptr,
    size_t           size,
    cl_uint          num_events_in_wait_list,
    const cl_event * event_wait_list,
    cl_event *       event) CL_API_SUFFIX__VERSION_2_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueSVMMemFill(KHRpfn_clEnqueueSVMMemFill next,
    cl_command_queue command_queue,
    void *           svm_ptr,
    const void *     pattern,
    size_t           pattern_size,
    size_t           size,
    cl_uint          num_events_in_wait_list,
    const cl_event * event_wait_list,
    cl_event *       event) CL_API_SUFFIX__VERSION_2_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueSVMMap(KHRpfn_clEnqueueSVMMap next,
    cl_command_queue command_queue,
    cl_bool          blocking_map,
    cl_map_flags     map_flags,
    void *           svm_ptr,
    size_t           size,
    cl_uint          num_events_in_wait_list,
    const cl_event * event_wait_list,
    cl_event *       event) CL_API_SUFFIX__VERSION_2_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueSVMUnmap(KHRpfn_clEnqueueSVMUnmap next,
    cl_command_queue command_queue,
    void *           svm_ptr,
    cl_uint          num_events_in_wait_list,
    const cl_event * event_wait_list,
    cl_event *       event) CL_API_SUFFIX__VERSION_2_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clSetCommandQueueProperty(KHRpfn_clSetCommandQueueProperty next,
    cl_command_queue              command_queue,
    cl_command_queue_properties   properties, 
    cl_bool                       enable,
    cl_command_queue_properties * old_properties) CL_EXT_SUFFIX__VERSION_1_0_DEPRECATED;

CL_API_ENTRY cl_mem CL_API_CALL
detour_clCreateImage2D(KHRpfn_clCreateImage2D next,
    cl_context              context,
    cl_mem_flags            flags,
    const cl_image_format * image_format,
    size_t                  image_width,
    size_t                  image_height,
    size_t                  image_row_pitch, 
    void *                  host_ptr,
    cl_int *                errcode_ret) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED;

CL_API_ENTRY cl_mem CL_API_CALL
detour_clCreateImage3D(KHRpfn_clCreateImage3D next,
    cl_context              context,
    cl_mem_flags            flags,
    const cl_image_format * image_format,
    size_t                  image_width, 
    size_t                  image_height,
    size_t                  image_depth, 
    size_t                  image_row_pitch, 
    size_t                  image_slice_pitch, 
    void *                  host_ptr,
    cl_int *                errcode_ret) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED;

CL_API_ENTRY cl_int CL_API_CALL
detour_clUnloadCompiler(KHRpfn_clUnloadCompiler next) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueMarker(KHRpfn_clEnqueueMarker next,
    cl_command_queue    command_queue,
    cl_event *          event) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueWaitForEvents(KHRpfn_clEnqueueWaitForEvents next,
    cl_command_queue command_queue,
    cl_uint          num_events,
    const cl_event * event_list) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueBarrier(KHRpfn_clEnqueueBarrier next,
cl_command_queue command_queue) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED;

CL_API_ENTRY void CL_API_CALL
detour_clGetExtensionFunctionAddress(KHRpfn_clGetExtensionFunctionAddress next,
const char *function_name) CL_EXT_SUFFIX__VERSION_1_1_DEPRECATED;

CL_API_ENTRY cl_mem CL_API_CALL
detour_clCreateFromGLBuffer(KHRpfn_clCreateFromGLBuffer next,
    cl_context    context,
    cl_mem_flags  flags,
    GLuint        bufobj,
    int *         errcode_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_mem CL_API_CALL
detour_clCreateFromGLTexture(KHRpfn_clCreateFromGLTexture next,
    cl_context      context,
    cl_mem_flags    flags,
    cl_GLenum       target,
    cl_GLint        miplevel,
    cl_GLuint       texture,
    cl_int *        errcode_ret) CL_API_SUFFIX__VERSION_1_2;

CL_API_ENTRY cl_mem CL_API_CALL
detour_clCreateFromGLTexture2D(KHRpfn_clCreateFromGLTexture2D next,
    cl_context      context,
    cl_mem_flags    flags,
    GLenum          target,
    GLint           miplevel,
    GLuint          texture,
    cl_int *        errcode_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_mem CL_API_CALL
detour_clCreateFromGLTexture3D(KHRpfn_clCreateFromGLTexture3D next,
    cl_context      context,
    cl_mem_flags    flags,
    GLenum          target,
    GLint           miplevel,
    GLuint          texture,
    cl_int *        errcode_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_mem CL_API_CALL
detour_clCreateFromGLRenderbuffer(KHRpfn_clCreateFromGLRenderbuffer next,
    cl_context           context,
    cl_mem_flags         flags,
    GLuint               renderbuffer,
    cl_int *             errcode_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clGetGLObjectInfo(KHRpfn_clGetGLObjectInfo next,
    cl_mem               memobj,
    cl_gl_object_type *  gl_object_type,
    GLuint *             gl_object_name) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clGetGLTextureInfo(KHRpfn_clGetGLTextureInfo next,
    cl_mem               memobj,
    cl_gl_texture_info   param_name,
    size_t               param_value_size,
    void *               param_value,
    size_t *             param_value_size_ret) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueAcquireGLObjects(KHRpfn_clEnqueueAcquireGLObjects next,
    cl_command_queue     command_queue,
    cl_uint              num_objects,
    const cl_mem *       mem_objects,
    cl_uint              num_events_in_wait_list,
    const cl_event *     event_wait_list,
    cl_event *           event) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueReleaseGLObjects(KHRpfn_clEnqueueReleaseGLObjects next,
    cl_command_queue     command_queue,
    cl_uint              num_objects,
    const cl_mem *       mem_objects,
    cl_uint              num_events_in_wait_list,
    const cl_event *     event_wait_list,
    cl_event *           event) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clGetGLContextInfoKHR(KHRpfn_clGetGLContextInfoKHR next,
    const cl_context_properties *properties,
    cl_gl_context_info param_name,
    size_t param_value_size,
    void *param_value,
    size_t *param_value_size_ret);

CL_API_ENTRY cl_event CL_API_CALL
detour_clCreateEventFromGLsyncKHR(KHRpfn_clCreateEventFromGLsyncKHR next,
    cl_context context,
    cl_GLsync sync,
    cl_int *errcode_ret);

CL_API_ENTRY cl_int CL_API_CALL
detour_clSetEventCallback(KHRpfn_clSetEventCallback next,
    cl_event            event,
    cl_int              command_exec_callback_type,
    void (CL_CALLBACK * pfn_notify)(cl_event, cl_int, void *),
    void *              user_data) CL_API_SUFFIX__VERSION_1_1;

CL_API_ENTRY cl_mem CL_API_CALL
detour_clCreateSubBuffer(KHRpfn_clCreateSubBuffer next,
    cl_mem                   buffer,
    cl_mem_flags             flags,
    cl_buffer_create_type    buffer_create_type,
    const void *             buffer_create_info,
    cl_int *                 errcode_ret) CL_API_SUFFIX__VERSION_1_1;

CL_API_ENTRY cl_int CL_API_CALL
detour_clSetMemObjectDestructorCallback(KHRpfn_clSetMemObjectDestructorCallback next,
    cl_mem memobj, 
    void (CL_CALLBACK * /*pfn_notify*/)( cl_mem memobj, void* /*user_data*/), 
    void * /*user_data */) CL_API_SUFFIX__VERSION_1_1;

CL_API_ENTRY cl_event CL_API_CALL
detour_clCreateUserEvent(KHRpfn_clCreateUserEvent next,
    cl_context    context,
    cl_int *      errcode_ret) CL_API_SUFFIX__VERSION_1_1;

CL_API_ENTRY cl_int CL_API_CALL
detour_clSetUserEventStatus(KHRpfn_clSetUserEventStatus next,
    cl_event   event,
    cl_int     execution_status) CL_API_SUFFIX__VERSION_1_1;

CL_API_ENTRY cl_int CL_API_CALL
detour_clCreateSubDevicesEXT(KHRpfn_clCreateSubDevicesEXT next,
    cl_device_id     in_device,
    const cl_device_partition_property_ext * partition_properties,
    cl_uint          num_entries,
    cl_device_id *   out_devices,
    cl_uint *        num_devices);

CL_API_ENTRY cl_int CL_API_CALL
detour_clRetainDeviceEXT(KHRpfn_clRetainDeviceEXT next,
    cl_device_id     device) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_int CL_API_CALL
detour_clReleaseDeviceEXT(KHRpfn_clReleaseDeviceEXT next,
    cl_device_id     device) CL_API_SUFFIX__VERSION_1_0;

CL_API_ENTRY cl_mem CL_API_CALL
detour_clCreateFromEGLImageKHR(KHRpfn_clCreateFromEGLImageKHR next,
    cl_context context,
    CLeglDisplayKHR display,
    CLeglImageKHR image,
    cl_mem_flags flags,
    const cl_egl_image_properties_khr *properties,
    cl_int *errcode_ret);

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueAcquireEGLObjectsKHR(KHRpfn_clEnqueueAcquireEGLObjectsKHR next,
    cl_command_queue command_queue,
    cl_uint num_objects,
    const cl_mem *mem_objects,
    cl_uint num_events_in_wait_list,
    const cl_event *event_wait_list,
    cl_event *event);

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueReleaseEGLObjectsKHR(KHRpfn_clEnqueueReleaseEGLObjectsKHR next,
    cl_command_queue command_queue,
    cl_uint num_objects,
    const cl_mem *mem_objects,
    cl_uint num_events_in_wait_list,
    const cl_event *event_wait_list,
    cl_event *event);

CL_API_ENTRY cl_event CL_API_CALL
detour_clCreateEventFromEGLSyncKHR(KHRpfn_clCreateEventFromEGLSyncKHR next,
    cl_context context,
    CLeglSyncKHR sync,
    CLeglDisplayKHR display,
    cl_int *errcode_ret);

/*CL_API_ENTRY cl_int CL_API_CALL
detour_clSetDefaultDeviceCommandQueue(KHRpfn_clSetDefaultDeviceCommandQueue next,
    cl_context context,
    cl_device_id device,
    cl_command_queue command_queue) CL_API_SUFFIX__VERSION_2_1;

CL_API_ENTRY cl_program CL_API_CALL
detour_clCreateProgramWithIL(KHRpfn_clCreateProgramWithIL next,
    cl_context context,
    const void * il,
    size_t length,
    cl_int * errcode_ret) CL_API_SUFFIX__VERSION_2_1;

CL_API_ENTRY cl_int CL_API_CALL
detour_clGetKernelSubGroupInfo (KHRpfn_clGetKernelSubGroupInfo  next,
    cl_kernel kernel,
    cl_device_id device,
    cl_kernel_sub_group_info param_name,
    size_t input_value_size,
    const void * input_value,
    size_t param_value_size,
    void * param_value,
    size_t * param_value_size_ret) CL_API_SUFFIX__VERSION_2_1;

CL_API_ENTRY cl_kernel CL_API_CALL
detour_clCloneKernel(KHRpfn_clCloneKernel next,
    cl_kernel source_kernel, 
    cl_int * errcode_ret) CL_API_SUFFIX__VERSION_2_1;

CL_API_ENTRY cl_int CL_API_CALL
detour_clEnqueueSVMMigrateMem(KHRpfn_clEnqueueSVMMigrateMem next,
    cl_command_queue command_queue,
    cl_uint num_svm_pointers,
    const void ** svm_pointers,
    const size_t * sizes,
    cl_mem_migration_flags flags,
    cl_uint num_events_in_wait_list,
    const cl_event * event_wait_list,
    cl_event * event) CL_API_SUFFIX__VERSION_2_1;

CL_API_ENTRY cl_int CL_API_CALL
detour_clGetDeviceAndHostTimer(KHRpfn_clGetDeviceAndHostTimer next,
    cl_device_id device,
    cl_ulong * device_timestamp,
    cl_ulong * host_timestamp) CL_API_SUFFIX__VERSION_2_1;

CL_API_ENTRY cl_int CL_API_CALL
detour_clGetHostTimer(KHRpfn_clGetHostTimer next,
    cl_device_id device,
    cl_ulong * host_timestamp) CL_API_SUFFIX__VERSION_2_1;*/

#if 1
#define DETOUR_CALL(name, next, ...) detour_##name(next, __VA_ARGS__)
#else
#define DETOUR_CALL(name, next, ...) next(__VA_ARGS__);
#endif

#endif // _ICD_DETOUR_H_H

/*
 * The MIT License
 *
 * Copyright (c) 2017 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */

#include <jni.h>
#include <string>
#include "CL/cl.h"
#include "common.h"
#include "clbench.h"
#include"shim2ify.h"

extern "C"
JNIEXPORT jstring JNICALL
Java_com_example_lpgpu2_opencltest_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Finished running OpenCLTest";

    cl_context context = 0;
    cl_command_queue commandQueue = 0;
    cl_program program = 0;
    cl_device_id device = 0;
    cl_kernel kernel = 0;
    int numberOfMemoryObjects = 3;
    cl_mem memoryObjects[3] = {0, 0, 0};
    cl_int errorNumber;
    clbench bench;

/*
    if (!createContext(&context))
    {
        cleanUpOpenCL(context, commandQueue, program, kernel, memoryObjects, numberOfMemoryObjects);
        hello = " Failed to create an OpenCL context. ";
        goto out;
    }

    if (!createCommandQueue(context, &commandQueue, &device))
    {
        cleanUpOpenCL(context, commandQueue, program, kernel, memoryObjects, numberOfMemoryObjects);
        hello = "Failed to create the OpenCL command queue. ";
        goto out;
    }

    if (!createProgram(context, device, "/data/local/tmp/hello_world_opencl.cl", &program))
    {
        cleanUpOpenCL(context, commandQueue, program, kernel, memoryObjects, numberOfMemoryObjects);
        hello = "Failed to create OpenCL program.";
        goto out;
    }

    kernel = clCreateKernel(program, "hello_world_opencl", &errorNumber);
    if (!checkSuccess(errorNumber))
    {
        cleanUpOpenCL(context, commandQueue, program, kernel, memoryObjects, numberOfMemoryObjects);
        hello = "Failed to create OpenCL kernel. ";
        goto out;
    }
*/
    if (bench.load_and_parse("/data/local/tmp/clbenchtest.cl") == CL_SUCCESS)
    {
        bench.compile();
        bench.run();
    }
    else
    {
        hello = "Failed to open /data/local/tmp/clbenchtest.cl ";
    }

out:
    return env->NewStringUTF(hello.c_str());
}

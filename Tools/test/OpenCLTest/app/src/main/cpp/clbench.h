/*
* The MIT License
*
* Copyright (c) 2017 Samsung Electronics Co., Ltd. All Rights Reserved
* For conditions of distribution and use, see the accompanying COPYING file.
*
*/

#ifndef OPENCLTEST_CLBENCH_H
#define OPENCLTEST_CLBENCH_H

#include "CL/cl.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <unistd.h>
#include <map>
#include <string>
#include <vector>

class clbench {

    std::map<std::string,int*> int_args;


    void register_arg(char const* arg, int* dest)
    {
        int_args[std::string(arg)]=dest;
    }


    std::string clsource;

    int err;
    int count = 20000;
    int global = 512*512;
    int local = 32;
    size_t global_s;
    size_t local_s;
    int in_size = 0;
    int tmp_size   = 0;
    int out_size = sizeof(float) * 1024*256;
    int repeat_inner = 1;
    int repeat_outer = 10;
    int delay_inner = 0;
    int delay_outer = 0;
    int unroll=1;
    int arg_in = 3;
    int arg_tmp = 3;
    int arg_out = 3;


    float* buffer_in;
    float* buffer_out;
    cl_platform_id platforms[1];
    cl_device_id devid;
    cl_mem in_mem;
    cl_mem out_mem;
    cl_mem tmp_mem;
    cl_context ctx;
    cl_command_queue cmds;
    cl_program prog;
    cl_kernel kernel;

public:

    clbench();
    ~clbench();
    int load_and_parse(char const* filename);
    void compile();
    void run();
};


#endif //OPENCLTEST_CLBENCH_H

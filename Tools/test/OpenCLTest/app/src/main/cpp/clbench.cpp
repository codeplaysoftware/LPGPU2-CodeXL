/*
 * The MIT License
 *
 * Copyright (c) 2017 Samsung Electronics Co., Ltd. All Rights Reserved
 * For conditions of distribution and use, see the accompanying COPYING file.
 *
 */

#include "clbench.h"
#include "lpgpu2_api.h"


using namespace std;

#define CL_CHECK(x) if ( ((x)) != CL_SUCCESS ) { printf("Error on line: %i\n ", __LINE__); exit(-1); }
#define CL_CHECK2(x) if ( (x) == NULL ) { printf("Error on line: %i \n ", __LINE__); exit(-1); }



clbench::clbench()
    {
        CL_CHECK(clGetPlatformIDs(1,platforms,NULL));
        CL_CHECK(clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_ALL, 1, &devid, NULL));
        CL_CHECK2(ctx = clCreateContext(0, 1, &devid, NULL, NULL, NULL));
        CL_CHECK2(cmds = clCreateCommandQueue(ctx, devid, 0, &err));
        register_arg("count",&count);
        register_arg("global",&global);
        register_arg("local",&local);
        register_arg("in_size",&in_size);
        register_arg("out_size",&out_size);
        register_arg("tmp_size",&out_size);
        register_arg("unroll",&unroll);
        register_arg("delay_inner",&delay_inner);
        register_arg("delay_outer",&delay_outer);
        register_arg("repeat_inner",&repeat_inner);
        register_arg("repeat_outer",&repeat_outer);
    }

clbench::~clbench()
    {
        if (arg_in != -1)
        {
            clReleaseMemObject(in_mem);
        }
        if (arg_tmp != -1)
        {
            clReleaseMemObject(tmp_mem);
        }
        if (arg_out != -1)
        {
            clReleaseMemObject(out_mem);
        }
        clReleaseProgram(prog);
        clReleaseKernel(kernel);
        clReleaseCommandQueue(cmds);
        clReleaseContext(ctx);
    }

int  clbench::load_and_parse(char const* filename)
    {
        FILE *f=fopen(filename,"rb");
        clsource=string();
        unroll=1;
        if (f == NULL)
        {
            return CL_INVALID_VALUE;
        }
        while(1)
        {
            char line[256];
            char* r=fgets(line,256,f);
            cout << line;
            if (r == NULL)
            {
                break;
            }
            if (line[0]=='@')
            {
                char* p=line+1;
                char* p2=p;
                while (*p2 != '=') p2++;
                *p2=0;
                p2++;
                auto it=int_args.find(string(p));
                if (it != int_args.end())
                {
                    *(it->second)=atoi(p2);
                }
            } else
            {
                for (int i=0;i<unroll;i++)
                {
                    clsource+=string(line);
                }
                unroll=1;
            }
        }
        cout << clsource;
        fclose(f);
        if (in_size != 0)
        {
            arg_tmp++;
            arg_out++;
            buffer_in=(float*)malloc(in_size);
        } else
        {
            arg_in = -1;
        }
        if (tmp_size != 0)
        {
            arg_out++;
        } else
        {
            arg_tmp = -1;
        }
        if (out_size == 0)
        {
            arg_out = -1;
        }
        buffer_out=(float*)malloc(out_size);
        return CL_SUCCESS;
    }

    void clbench::compile()
    {
        char const* src=clsource.c_str();
        CL_CHECK2(prog = clCreateProgramWithSource(ctx, 1, (const char **) &src , NULL, &err));
        err=clBuildProgram(prog, 0, NULL, NULL, NULL, NULL);
        if (err==CL_BUILD_PROGRAM_FAILURE)
        {
            size_t size;
            clGetProgramBuildInfo(prog, devid, CL_PROGRAM_BUILD_LOG, 0, NULL, &size);
            vector<char> log(size);

            clGetProgramBuildInfo(prog, devid, CL_PROGRAM_BUILD_LOG, size, &(log[0]), NULL);
            printf("%s\n", &(log[0]));
            exit(-1);
        }

        CL_CHECK2(kernel = clCreateKernel(prog, "testkernel", NULL));
    }

    void clbench::run()
    {
        cl_event event;
        cl_int error;
        if (arg_in != -1)
        {
            CL_CHECK2(in_mem = clCreateBuffer(ctx, CL_MEM_READ_ONLY, in_size, NULL, NULL));
            CL_CHECK(clSetKernelArg(kernel, arg_in, sizeof(cl_mem), &in_mem));
        }
        if (arg_tmp != -1)
        {
            CL_CHECK2(tmp_mem = clCreateBuffer(ctx, CL_MEM_READ_WRITE, tmp_size, NULL, NULL));
            CL_CHECK(clSetKernelArg(kernel, arg_tmp, sizeof(cl_mem), &tmp_mem));
        }
        if (arg_out != -1)
        {
            CL_CHECK2(out_mem = clCreateBuffer(ctx, CL_MEM_WRITE_ONLY, out_size, NULL, NULL));
            CL_CHECK(clSetKernelArg(kernel, arg_out, sizeof(cl_mem), &out_mem));
        }

        CL_CHECK(clSetKernelArg(kernel, 2, sizeof(int), &count));
        //CL_CHECK(clGetKernelWorkGroupInfo(kernel, devid, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL));
        for (int j=0;j<repeat_outer;j++)
        {
            CL_CHECK(clSetKernelArg(kernel, 0, sizeof(int), &j));
            // In Data Transfer missing!
            if (arg_in != -1)
            {
                CL_CHECK(clEnqueueWriteBuffer(cmds,in_mem,CL_TRUE,0,in_size,buffer_in,0,NULL,NULL));
            }
            for (int i=0;i<repeat_inner;i++)
            {
                CL_CHECK(clSetKernelArg(kernel, 1, sizeof(int), &i));
                global_s=global;
                local_s=local;
                err=clEnqueueNDRangeKernel(cmds, kernel, 1, NULL, &global_s, &local_s, 0, NULL, NULL);
                printf("err:%i\n",err);
                CL_CHECK(err);
                usleep(delay_inner*1000);
            }
            CL_CHECK(clEnqueueReadBuffer(cmds,out_mem,CL_TRUE,0,out_size,buffer_out,0,NULL,NULL));
            clFinish(cmds);
            lpgpu2_TerminateFrame();
            usleep(delay_outer*1000);
        }

    }



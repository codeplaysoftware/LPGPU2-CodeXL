@repeat_inner=1
@repeat_outer=1024
@delay_inner=0
@delay_outer=0
@in_size=16777216
@out_size=4
@global=1
@local=1
__kernel void testkernel(const int loop1, const int loop2, const int count,__global float* in, __global float* out) {
	out[get_global_id(0)]=in[get_global_id(0)];
}


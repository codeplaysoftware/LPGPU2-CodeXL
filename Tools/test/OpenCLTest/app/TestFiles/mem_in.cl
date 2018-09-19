@repeat_inner=256
@repeat_outer=2
@delay_inner=0
@in_size=16777216
@out_size=4
@global=4194304
@local=128
@count=0;
__kernel void testkernel(const int loop1, const int loop2, const int count,__global float* in, __global float* out) {
	float tmp=in[get_global_id(0)];
	if (count)
	{
		out[0]=tmp;
	}
}


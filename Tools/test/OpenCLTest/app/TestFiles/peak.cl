@repeat_inner=3
@repeat_outer=2
__kernel void testkernel(const int loop1, const int loop2, const int count,__global float* out) {
	float a,b,c,d;
	a=get_local_id(0); b=get_local_id(0)+1; c=get_local_id(0)+2; d=get_local_id(0)+3;
	for (int i=1;i<count;i+=4) {
      a*= i;
      b*= (i+1);
      c*= (i+2);
      d*= (i+3);
	};
	out[get_global_id(0)]=a*b*c*d;
}


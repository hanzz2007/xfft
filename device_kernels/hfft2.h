#include"hfft.h"

__global__ void d_hfft2x( float2* d_o, float2* d_i, int bat )
{											
	float2 c[2], temp;
	int tidx=blockIdx.x*blockDim.x+threadIdx.x;
	if(tidx>=bat) return;
	d_i+=(tidx<<1);
	d_o+=(tidx<<1);
	mLOAD2(c,d_i,1,)
	mFFT2(c,)
	mSTORE2(d_o,c,1,)
}
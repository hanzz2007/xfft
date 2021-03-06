#include"hfft.h"

#if SM==37
#define NUM_CTA_1024 15
#else
#define NUM_CTA_1024 8
#endif

__global__ void __launch_bounds__(64,NUM_CTA_1024) d_hfft1024x( float2* d_o, const float2* __restrict__ d_i, const float2* __restrict__ d_RF )
{															
	__shared__ float smem[16*97];								
	float2 c[16], RF[15], temp;									
	d_i+=((blockIdx.x<<10)+threadIdx.x);						
	d_o+=((blockIdx.x<<10)+threadIdx.x);						
	unsigned int lane=threadIdx.x&15;							
	unsigned int slot=threadIdx.x>>4;							
	float* sst=&smem[threadIdx.x];								
	float* sld=&smem[80*slot+lane];								
	RF[0]=d_RF[threadIdx.x];									
	mLOAD16(c,d_i,64,)											
	mCALRF16(RF)												
	mFFT16(c,)												
	mHMRF16(c,RF)												
	RF[0]=d_RF[lane<<4];										
	mPERMUTE4x4(sst,sld,c,80,320,16,0xf);							
	mCALRF4(RF)													
	mFFT4(&c[ 0],)											
	mFFT4(&c[ 4],)											
	mFFT4(&c[ 8],)											
	mFFT4(&c[12],)											
	mHMRF4(&c[ 0],RF)
	mHMRF4(&c[ 4],RF)
	mHMRF4(&c[ 8],RF)
	mHMRF4(&c[12],RF)												
	sst=&smem[15*(slot&1)+97*slot+lane];
	sld=&smem[15*(lane&1)+97*lane+17*slot];					
	mIPERMUTE4x4(sst,sld,c,388,17,1,0x7)							
	mFFT16(c,)												
	mISTORE16(d_o,c,64,)											
}
__global__ void __launch_bounds__(64,NUM_CTA_1024) d_hifft1024x( float2* d_o, const float2* __restrict__ d_i, const float2* __restrict__ d_RF )
{															
	__shared__ float smem[16*97];								
	float2 c[16], RF[15], temp;									
	d_i+=((blockIdx.x<<10)+threadIdx.x);						
	d_o+=((blockIdx.x<<10)+threadIdx.x);						
	unsigned int lane=threadIdx.x&15;							
	unsigned int slot=threadIdx.x>>4;							
	float* sst=&smem[threadIdx.x];								
	float* sld=&smem[80*slot+lane];								
	RF[0]=d_RF[threadIdx.x];
	RF[0].y=-RF[0].y;
	mLOAD16(c,d_i,64,)											
	mCALRF16(RF)												
	mFFT16(c,i)												
	mHMRF16(c,RF)												
	RF[0]=d_RF[lane<<4];
	RF[0].y=-RF[0].y;
	mPERMUTE4x4(sst,sld,c,69,276,17,0xf);							
	mCALRF4(RF)													
	mFFT4(&c[ 0],i)											
	mFFT4(&c[ 4],i)											
	mFFT4(&c[ 8],i)											
	mFFT4(&c[12],i)											
	mHMRF4(&c[ 0],RF)
	mHMRF4(&c[ 4],RF)
	mHMRF4(&c[ 8],RF)
	mHMRF4(&c[12],RF)													
	sst=&smem[15*(slot&1)+97*slot+lane];
	sld=&smem[15*(lane&1)+97*lane+17*slot];					
	mIPERMUTE4x4(sst,sld,c,388,17,1,0x7)							
	mFFT16(c,i)												
	mISTORE16(d_o,c,64,)											
}

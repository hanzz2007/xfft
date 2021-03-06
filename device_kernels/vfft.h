#ifndef __vfft_h__
#define __vfft_h__

#include"xfft.h"
#include"cuintrin.h"

__device__ __forceinline__ static float2 __fldg2( const float2* __restrict__ p )
{
	return *p;
}
__device__ __forceinline__ static float4 __fldg4( const float4* __restrict__ p )
{
	return *p;
}

#define mVMRF4(p,i,op){                                           \
    c[1]=d_##i##cmul(__fld##op##2((p)+0),c[1]);                   \
    d_##i##cmulx2(c[2],c[3],__fld##op##4(((const float4*)(p))+1));\
}

#define mVMRF4x2(p,i,op){                                                    \
    temp=__fld##op##2((p));                                                  \
    c[1]=d_##i##cmul(temp,c[1]);                                             \
    c[5]=d_##i##cmul(temp,c[5]);                                             \
    d_##i##cmul2x2(c[2],c[3],c[6],c[7],__fld##op##4(((const float4*)(p))+1));\
}

#define mVMRF8(p,i,op){                                           \
    c[1]=d_##i##cmul(__fld##op##2((p)),c[1]);                     \
    d_##i##cmulx2(c[2],c[3],__fld##op##4(((const float4*)(p))+1));\
    d_##i##cmulx2(c[4],c[5],__fld##op##4(((const float4*)(p))+2));\
    d_##i##cmulx2(c[6],c[7],__fld##op##4(((const float4*)(p))+3));\
}

#define mVMRF8x2(p,i,op){                                                      \
    temp=__fld##op##2((p));                                                    \
    c[1]=d_##i##cmul(temp,c[1]);                                               \
    c[9]=d_##i##cmul(temp,c[9]);                                               \
    d_##i##cmul2x2(c[2],c[3],c[10],c[11],__fld##op##4(((const float4*)(p))+1));\
    d_##i##cmul2x2(c[4],c[5],c[12],c[13],__fld##op##4(((const float4*)(p))+2));\
    d_##i##cmul2x2(c[6],c[7],c[14],c[15],__fld##op##4(((const float4*)(p))+3));\
}

#define mVMRF16(p,i,op){                                            \
    c[1]=d_##i##cmul(__fld##op##2((p)),c[1]);                       \
    d_##i##cmulx2(c[ 2],c[ 3],__fld##op##4(((const float4*)(p))+1));\
    d_##i##cmulx2(c[ 4],c[ 5],__fld##op##4(((const float4*)(p))+2));\
    d_##i##cmulx2(c[ 6],c[ 7],__fld##op##4(((const float4*)(p))+3));\
    d_##i##cmulx2(c[ 8],c[ 9],__fld##op##4(((const float4*)(p))+4));\
    d_##i##cmulx2(c[10],c[11],__fld##op##4(((const float4*)(p))+5));\
    d_##i##cmulx2(c[12],c[13],__fld##op##4(((const float4*)(p))+6));\
    d_##i##cmulx2(c[14],c[15],__fld##op##4(((const float4*)(p))+7));\
}

#define CUDA_VFFT_DECL(R,LB,dir,s,P)	__global__ void LB d_v##dir##fft##R##x_e##s( float2* d_o, float2* d_i P )
#define CUDA_UFFTX_DECL(R,LB,dir,n,s)	__global__ void LB d_u##dir##fft##R##x_x##n##e##s( float2* d_o, const float2* __restrict__ d_i, const float2* __restrict__ d_RF )
#define CUDA_VFFTX_DECL(R,LB,dir,n,s,P)	__global__ void LB d_v##dir##fft##R##x_x##n##e##s( float2* d_o, const float2* __restrict__ d_i P )

#define PRF ,const float2* __restrict__ d_RF

#define CUDA_VFFTX(R,dir) __global__ void d_v##dir##fft##R##x( float2* d_o, const float2* __restrict__ d_i )\
{                                                                                                           \
    float2 c[R], temp;                                                                                      \
    const unsigned int n=gridDim.x*blockDim.x;                                                              \
    d_i+=(blockIdx.y*R*n+blockIdx.x*blockDim.x+threadIdx.x);                                                \
    d_o+=(blockIdx.y*R*n+blockIdx.x*blockDim.x+threadIdx.x);                                                \
    mLOAD##R(c,d_i,n,)                                                                                      \
    mFFT##R(c,dir)                                                                                          \
    mISTORE##R(d_o,c,n,)                                                                                    \
}

CUDA_VFFTX( 2,)
CUDA_VFFTX( 4,)
CUDA_VFFTX( 8,)
CUDA_VFFTX(16,)
CUDA_VFFTX( 2,i)
CUDA_VFFTX( 4,i)
CUDA_VFFTX( 8,i)
CUDA_VFFTX(16,i)

#endif
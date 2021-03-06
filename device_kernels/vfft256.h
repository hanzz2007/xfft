#include"vfft.h"

#define CUDA_VFFT256(LB,dir,e,bdx,op) CUDA_VFFT_DECL(256,LB,dir,e,PRF)    \
{                                                                         \
    __shared__ float smem[256*bdx];                                       \
    float2 c[16], temp;                                                   \
    d_i+=((blockIdx.y*256+threadIdx.y)*(1<<e)+blockIdx.x*bdx+threadIdx.x);\
    d_o+=((blockIdx.y*256+threadIdx.y)*(1<<e)+blockIdx.x*bdx+threadIdx.x);\
    float* sst=&smem[16*bdx*threadIdx.y+threadIdx.x];                     \
    float* sld=&smem[   bdx*threadIdx.y+threadIdx.x];                     \
    mLOAD16(c,d_i,16*(1<<e),)                                             \
    mFFT16(c,dir)                                                         \
    mVMRF16(&d_RF[threadIdx.y<<4],dir,op)                                 \
    mPERMUTE(16,sst,sld,c,bdx,16*bdx,7)                                   \
    mFFT16(c,dir)                                                         \
    mISTORE16(d_o,c,16*(1<<e),)                                           \
}
#define CUDA_UFFT256X(LB,dir,n,e,bdx,op) CUDA_UFFTX_DECL(256,LB,dir,n,e)                   \
{                                                                                          \
    __shared__ float smem[256*bdx];                                                        \
    float2 c[16], temp;                                                                    \
    unsigned int slot=blockIdx.x/((1<<e)/bdx);                                             \
    unsigned int bidx=blockIdx.x&((1<<e)/bdx-1);                                           \
    size_t p=blockIdx.y*n*256*(1<<e)+slot*(1<<e)+threadIdx.y*n*(1<<e)+bidx*bdx+threadIdx.x;\
    d_i+=p; d_o+=p;                                                                        \
    slot+=threadIdx.y*n;                                                                   \
    float* sst=&smem[16*bdx*threadIdx.y+threadIdx.x];                                      \
    float* sld=&smem[   bdx*threadIdx.y+threadIdx.x];                                      \
    mLOAD16(c,d_i,16*n*(1<<e),)                                                            \
    mFFT16(c,dir)                                                                          \
    mVMRF16(&d_RF[slot<<4],dir,op)                                                         \
    mPERMUTE(16,sst,sld,c,bdx,16*bdx,7)                                                    \
    mFFT16(c,dir)                                                                          \
    mVMRF16(&d_RF[(slot&(n-1))<<8],dir,op)                                                 \
    mISTORE16(d_o,c,16*n*(1<<e),)                                                          \
}
#define CUDA_VFFT256X(LB,dir,n,e,bdx,op) CUDA_VFFTX_DECL(256,LB,dir,n,e,PRF)\
{                                                                           \
    __shared__ float smem[256*bdx];                                         \
    float2 c[16], temp;                                                     \
    unsigned int slot=blockIdx.x/((1<<e)/bdx);                              \
    unsigned int bidx=blockIdx.x&((1<<e)/bdx-1);                            \
    size_t p=blockIdx.y*256*n*(1<<e)+bidx*bdx+threadIdx.x;                  \
    d_i+=(p+slot*(1<<e)*256+threadIdx.y*(1<<e));                            \
    d_o+=(p+slot*(1<<e)+threadIdx.y*n*(1<<e));	                            \
    float* sst=&smem[16*bdx*threadIdx.y+threadIdx.x];                       \
    float* sld=&smem[   bdx*threadIdx.y+threadIdx.x];                       \
    mLOAD16(c,d_i,16*(1<<e),)                                               \
    mFFT16(c,dir)                                                           \
    mVMRF16(&d_RF[16*n*threadIdx.y],dir,op)                                 \
    mPERMUTE(16,sst,sld,c,bdx,16*bdx,7)                                     \
    mFFT16(c,dir)                                                           \
    mISTORE16(d_o,c,16*n*(1<<e),)                                           \
}

#if SM==37
#define NUM_CTA_V256 2
#else
#define NUM_CTA_V256 1
#endif

CUDA_VFFT256(LB(512,2),,1, 2,g)
CUDA_VFFT256(LB(512,2),,2, 4,g)
CUDA_VFFT256(LB(512,2),,3, 8,g)
CUDA_VFFT256(LB(512,2),,4,16,g)
CUDA_VFFT256(LB(512,2),, 5,32,u)
CUDA_VFFT256(LB(512,NUM_CTA_V256),, 6,32,u)
CUDA_VFFT256(LB(512,NUM_CTA_V256),, 7,32,u)
CUDA_VFFT256(LB(512,NUM_CTA_V256),, 8,32,u)
CUDA_VFFT256(LB(512,NUM_CTA_V256),, 9,32,u)
CUDA_VFFT256(LB(512,NUM_CTA_V256),,10,32,u)
CUDA_VFFT256(LB(512,NUM_CTA_V256),,11,32,u)
CUDA_VFFT256(LB(512,NUM_CTA_V256),,12,32,u)
CUDA_VFFT256(LB(512,NUM_CTA_V256),,13,32,u)
CUDA_VFFT256(LB(512,NUM_CTA_V256),,14,32,u)
CUDA_VFFT256(LB(512,NUM_CTA_V256),,15,32,u)
CUDA_VFFT256(LB(512,NUM_CTA_V256),,16,32,u)

CUDA_VFFT256(LB(512,2),i,1, 2,g)
CUDA_VFFT256(LB(512,2),i,2, 4,g)
CUDA_VFFT256(LB(512,2),i,3, 8,g)
CUDA_VFFT256(LB(512,2),i,4,16,g)
CUDA_VFFT256(LB(512,NUM_CTA_V256),i, 5,32,u)
CUDA_VFFT256(LB(512,NUM_CTA_V256),i, 6,32,u)
CUDA_VFFT256(LB(512,NUM_CTA_V256),i, 7,32,u)
CUDA_VFFT256(LB(512,NUM_CTA_V256),i, 8,32,u)
CUDA_VFFT256(LB(512,NUM_CTA_V256),i, 9,32,u)
CUDA_VFFT256(LB(512,NUM_CTA_V256),i,10,32,u)
CUDA_VFFT256(LB(512,NUM_CTA_V256),i,11,32,u)
CUDA_VFFT256(LB(512,NUM_CTA_V256),i,12,32,u)
CUDA_VFFT256(LB(512,NUM_CTA_V256),i,13,32,u)
CUDA_VFFT256(LB(512,NUM_CTA_V256),i,14,32,u)
CUDA_VFFT256(LB(512,NUM_CTA_V256),i,15,32,u)
CUDA_VFFT256(LB(512,NUM_CTA_V256),i,16,32,u)

CUDA_UFFT256X(,,128,1, 2,g)
CUDA_UFFT256X(,,128,2, 4,g)
CUDA_UFFT256X(,,128,3, 8,g)
CUDA_UFFT256X(,,128,4,16,g)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),,128, 5,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),,128, 6,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),,128, 7,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),,128, 8,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),,128, 9,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),,128,10,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),,128,11,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),,128,12,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),,128,13,32,u)

CUDA_UFFT256X(,i,128,1, 2,g)
CUDA_UFFT256X(,i,128,2, 4,g)
CUDA_UFFT256X(,i,128,3, 8,g)
CUDA_UFFT256X(,i,128,4,16,g)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),i,128, 5,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),i,128, 6,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),i,128, 7,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),i,128, 8,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),i,128, 9,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),i,128,10,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),i,128,11,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),i,128,12,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),i,128,13,32,u)

CUDA_UFFT256X(,,256,1, 2,g)
CUDA_UFFT256X(,,256,2, 4,g)
CUDA_UFFT256X(,,256,3, 8,g)
CUDA_UFFT256X(,,256,4,16,g)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),,256, 5,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),,256, 6,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),,256, 7,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),,256, 8,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),,256, 9,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),,256,10,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),,256,11,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),,256,12,32,u)

CUDA_UFFT256X(,i,256,1, 2,g)
CUDA_UFFT256X(,i,256,2, 4,g)
CUDA_UFFT256X(,i,256,3, 8,g)
CUDA_UFFT256X(,i,256,4,16,g)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),i,256, 5,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),i,256, 6,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),i,256, 7,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),i,256, 8,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),i,256, 9,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),i,256,10,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),i,256,11,32,u)
CUDA_UFFT256X(LB(512,NUM_CTA_V256),i,256,12,32,u)

CUDA_VFFT256X(,,256,1, 2,g)
CUDA_VFFT256X(,,256,2, 4,g)
CUDA_VFFT256X(,,256,3, 8,g)
CUDA_VFFT256X(,,256,4,16,g)
CUDA_VFFT256X(LB(512,NUM_CTA_V256),,256, 5,32,u)
CUDA_VFFT256X(LB(512,NUM_CTA_V256),,256, 6,32,u)
CUDA_VFFT256X(LB(512,NUM_CTA_V256),,256, 7,32,u)
CUDA_VFFT256X(LB(512,NUM_CTA_V256),,256, 8,32,u)
CUDA_VFFT256X(LB(512,NUM_CTA_V256),,256, 9,32,u)
CUDA_VFFT256X(LB(512,NUM_CTA_V256),,256,10,32,u)
CUDA_VFFT256X(LB(512,NUM_CTA_V256),,256,11,32,u)
CUDA_VFFT256X(LB(512,NUM_CTA_V256),,256,12,32,u)

CUDA_VFFT256X(,i,256,1, 2,g)
CUDA_VFFT256X(,i,256,2, 4,g)
CUDA_VFFT256X(,i,256,3, 8,g)
CUDA_VFFT256X(,i,256,4,16,g)
CUDA_VFFT256X(LB(512,NUM_CTA_V256),i,256, 5,32,u)
CUDA_VFFT256X(LB(512,NUM_CTA_V256),i,256, 6,32,u)
CUDA_VFFT256X(LB(512,NUM_CTA_V256),i,256, 7,32,u)
CUDA_VFFT256X(LB(512,NUM_CTA_V256),i,256, 8,32,u)
CUDA_VFFT256X(LB(512,NUM_CTA_V256),i,256, 9,32,u)
CUDA_VFFT256X(LB(512,NUM_CTA_V256),i,256,10,32,u)
CUDA_VFFT256X(LB(512,NUM_CTA_V256),i,256,11,32,u)
CUDA_VFFT256X(LB(512,NUM_CTA_V256),i,256,12,32,u)
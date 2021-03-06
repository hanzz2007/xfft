#include"hfft.h"
#include"cuintrin.h"

/*
64x2   : block(16, n)
64x4   : block(32, n)
64x8   : block(64, 1)
64x16  : block(32, 4)
64x32  : block(64, 4)
64x64  : block(64, 8)
64x128 : block(64, 8)
*/
__global__ void d_fft64x2x( float2* d_c, const float2* __restrict__ d_RF, int bat )
{											
	extern __shared__ float smem[];
	float2 c[8], RF[7], temp;
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	d_c+=(patch_id<<7)+threadIdx.x;
	unsigned int lane=threadIdx.x&7;
	unsigned int slot=threadIdx.x>>3;
	float* spx=&smem[144*patch_id+threadIdx.x];
	float* spy=&smem[144*patch_id+slot*72+lane];
	float* spz=&smem[144*patch_id+slot*72+9*lane];
	RF[0]=d_RF[lane];
	mLOAD2(&c[0],d_c+0*16,64,)
	mLOAD2(&c[2],d_c+1*16,64,)
	mLOAD2(&c[4],d_c+2*16,64,)
	mLOAD2(&c[6],d_c+3*16,64,)
	mCALRF8(RF)
	mFFT2(&c[0],)
	mFFT2(&c[2],)
	mFFT2(&c[4],)
	mFFT2(&c[6],)
	mISTORE2(spx+0*16,&c[0],72,.x)
	mISTORE2(spx+1*16,&c[2],72,.x)
	mISTORE2(spx+2*16,&c[4],72,.x)
	mISTORE2(spx+3*16,&c[6],72,.x)
	mLOAD8(c,spy,8,.x)
	mISTORE2(spx+0*16,&c[0],72,.y)
	mISTORE2(spx+1*16,&c[2],72,.y)
	mISTORE2(spx+2*16,&c[4],72,.y)
	mISTORE2(spx+3*16,&c[6],72,.y)

	mLOAD8(c,spy,8,.y)
	mFFT8(c,)
	mHMRF8(c,RF)
	mPERMUTE(8,spy,spz,c,9,1,0)
	mFFT8(c,)
	mISTORE8(spy,c,8,.x)
	mLOAD2(&c[0],spx+0*16,72,.x)
	mLOAD2(&c[2],spx+1*16,72,.x)
	mLOAD2(&c[4],spx+2*16,72,.x)
	mLOAD2(&c[6],spx+3*16,72,.x)
	mISTORE8(spy,c,8,.y)
	mLOAD2(&c[0],spx+0*16,72,.y)
	mLOAD2(&c[2],spx+1*16,72,.y)
	mLOAD2(&c[4],spx+2*16,72,.y)
	mLOAD2(&c[6],spx+3*16,72,.y)
	mSTORE2(d_c+0*16,&c[0],64,)
	mSTORE2(d_c+1*16,&c[2],64,)
	mSTORE2(d_c+2*16,&c[4],64,)
	mSTORE2(d_c+3*16,&c[6],64,)
}
__global__ void d_ifft64x2x( float2* d_c, const float2* __restrict__ d_RF, int bat )
{											
	extern __shared__ float smem[];
	float2 c[8], RF[7], temp;
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	d_c+=(patch_id<<7)+threadIdx.x;
	unsigned int lane=threadIdx.x&7;
	unsigned int slot=threadIdx.x>>3;
	float* spx=&smem[144*patch_id+threadIdx.x];
	float* spy=&smem[144*patch_id+slot*72+lane];
	float* spz=&smem[144*patch_id+slot*72+9*lane];
	RF[0]=d_RF[lane];
	RF[0].y=-RF[0].y;
	mLOAD2(&c[0],d_c+0*16,64,)
	mLOAD2(&c[2],d_c+1*16,64,)
	mLOAD2(&c[4],d_c+2*16,64,)
	mLOAD2(&c[6],d_c+3*16,64,)
	mCALRF8(RF)
	mFFT2(&c[0],)
	mFFT2(&c[2],)
	mFFT2(&c[4],)
	mFFT2(&c[6],)
	mISTORE2(spx+0*16,&c[0],72,.x)
	mISTORE2(spx+1*16,&c[2],72,.x)
	mISTORE2(spx+2*16,&c[4],72,.x)
	mISTORE2(spx+3*16,&c[6],72,.x)
	mLOAD8(c,spy,8,.x)
	mISTORE2(spx+0*16,&c[0],72,.y)
	mISTORE2(spx+1*16,&c[2],72,.y)
	mISTORE2(spx+2*16,&c[4],72,.y)
	mISTORE2(spx+3*16,&c[6],72,.y)

	mLOAD8(c,spy,8,.y)
	mFFT8(c,i)
	mHMRF8(c,RF)
	mPERMUTE(8,spy,spz,c,9,1,0)
	mFFT8(c,i)
	mISTORE8(spy,c,8,.x)
	mLOAD2(&c[0],spx+0*16,72,.x)
	mLOAD2(&c[2],spx+1*16,72,.x)
	mLOAD2(&c[4],spx+2*16,72,.x)
	mLOAD2(&c[6],spx+3*16,72,.x)
	mISTORE8(spy,c,8,.y)
	mLOAD2(&c[0],spx+0*16,72,.y)
	mLOAD2(&c[2],spx+1*16,72,.y)
	mLOAD2(&c[4],spx+2*16,72,.y)
	mLOAD2(&c[6],spx+3*16,72,.y)
	mSTORE2(d_c+0*16,&c[0],64,)
	mSTORE2(d_c+1*16,&c[2],64,)
	mSTORE2(d_c+2*16,&c[4],64,)
	mSTORE2(d_c+3*16,&c[6],64,)
}
__global__ void d_fft64x4x( float2* d_c, const float2* __restrict__ d_RF, int bat )
{											
	extern __shared__ float smem[];
	float2 c[8], RF[7], temp;
	unsigned int patch_id=(blockIdx.x*blockDim.y)+threadIdx.y;
	if(patch_id>=bat) return;
	d_c+=(patch_id<<8)+threadIdx.x;
	unsigned int lane=threadIdx.x&7;
	unsigned int slot=threadIdx.x>>3;
	float* spx=&smem[288*patch_id+threadIdx.x];
	float* spy=&smem[288*patch_id+slot*72+lane];
	float* spz=&smem[288*patch_id+slot*72+9*lane];
	RF[0]=d_RF[lane];
	mLOAD4x2(c,d_c,32,64,)
	mCALRF8(RF)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mISTORE4(spx+0*32,&c[0],72,.x)
	mISTORE4(spx+1*32,&c[4],72,.x)
	mLOAD8(c,spy,8,.x)
	mISTORE4(spx+0*32,&c[0],72,.y)
	mISTORE4(spx+1*32,&c[4],72,.y)
	mLOAD8(c,spy,8,.y)
	mFFT8(c,)
	mHMRF8(c,RF)
	mPERMUTE(8,spy,spz,c,9,1,0)
	mFFT8(c,)
	mPERMUTE_S8_L4x2(spy,spx,c,8,32,72,0)
	mSTORE4x2(d_c,c,32,64)
}
__global__ void d_ifft64x4x( float2* d_c, const float2* __restrict__ d_RF, int bat )
{											
	extern __shared__ float smem[];
	float2 c[8], RF[7], temp;
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	d_c+=(patch_id<<8)+threadIdx.x;
	unsigned int lane=threadIdx.x&7;
	unsigned int slot=threadIdx.x>>3;
	float* spx=&smem[288*patch_id+threadIdx.x];
	float* spy=&smem[288*patch_id+slot*72+lane];
	float* spz=&smem[288*patch_id+slot*72+9*lane];
	RF[0]=d_RF[lane];
	RF[0].y=-RF[0].y;
	mLOAD4x2(c,d_c,32,64,)
	mCALRF8(RF)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mISTORE4(spx+0*32,&c[0],72,.x)
	mISTORE4(spx+1*32,&c[4],72,.x)
	mLOAD8(c,spy,8,.x)	
	mISTORE4(spx+0*32,&c[0],72,.y)
	mISTORE4(spx+1*32,&c[4],72,.y)
	mLOAD8(c,spy,8,.y)
	mFFT8(c,i)
	mHMRF8(c,RF)
	mPERMUTE(8,spy,spz,c,9,1,0)
	mFFT8(c,i)
	mPERMUTE_S8_L4x2(spy,spx,c,8,32,72,0)
	mSTORE4x2(d_c,c,32,64)
}
__global__ void d_fft64x8x( float2* d_c, const float2* __restrict__ d_RF )
{											
	__shared__ float smem[8*72];
	float2 c[8], RF[7], temp;
	d_c+=(blockIdx.x<<9)+threadIdx.x;
	unsigned int lane=threadIdx.x&7;
	unsigned int slot=threadIdx.x>>3;
	float* spx=&smem[threadIdx.x];
	float* spy=&smem[72*slot+lane];
	float* spz=&smem[72*slot+9*lane];
	RF[0]=d_RF[lane];
	mLOAD8(c,d_c,64,)
	mCALRF8(RF)
	mFFT8(c,)
	mPERMUTE(8,spx,spy,c,72,8,0xf)
	mFFT8(c,)
	mHMRF8(c,RF)
	mPERMUTE(8,spy,spz,c,9,1,0xf)
	mFFT8(c,)
	mPERMUTE(8,spy,spx,c,8,72,0x7)
	mSTORE8(d_c,c,64,)
}
__global__ void d_ifft64x8x( float2* d_c, const float2* __restrict__ d_RF )
{											
	__shared__ float smem[8*72];
	float2 c[8], RF[7], temp;
	d_c+=(blockIdx.x<<9)+threadIdx.x;
	unsigned int lane=threadIdx.x&7;
	unsigned int slot=threadIdx.x>>3;
	float* spx=&smem[threadIdx.x];
	float* spy=&smem[72*slot+lane];
	float* spz=&smem[72*slot+9*lane];
	RF[0]=d_RF[lane];
	RF[0].y=-RF[0].y;
	mLOAD8(c,d_c,64,)
	mCALRF8(RF)
	mFFT8(c,i)
	mPERMUTE(8,spx,spy,c,72,8,0xf)
	mFFT8(c,i)
	mHMRF8(c,RF)
	mPERMUTE(8,spy,spz,c,9,1,0xf)
	mFFT8(c,i)
	mPERMUTE(8,spy,spx,c,8,72,0x7)
	mSTORE8(d_c,c,64,)
}
__global__ void d_fft64x16x( float2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[16*72];
	float2 c[8], RF[7], temp;
	d_c+=(blockIdx.x<<10)+(threadIdx.y<<6)+threadIdx.x;
	unsigned int lane=threadIdx.x&7;
	unsigned int slot=(threadIdx.y<<2)+(threadIdx.x>>3);
	float* spx=&smem[ 72*threadIdx.y+threadIdx.x];
	float* spy=&smem[288*threadIdx.y+threadIdx.x];
	float* spu=&smem[72*slot+lane];
	float* spv=&smem[72*slot+9*lane];
	RF[0]=d_RF[threadIdx.y<<2];
	mLOAD4(&c[0],d_c   ,256,)
	mLOAD4(&c[4],d_c+32,256,)
	mCALRF4(RF)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mHMRF4(&c[0],RF)
	mHMRF4(&c[4],RF)
	mPERMUTE4x2(spx,spy,c,32,288,32,72,0xf)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	RF[0]=d_RF[lane];
	mCALRF8(RF)
	mPERMUTE_S4x2_L8(spx,spu,c,32,288,8,0xf)
	mFFT8(c,)
	mHMRF8(c,RF)
	mPERMUTE(8,spu,spv,c,9,1,0xf)
	mFFT8(c,)
	mPERMUTE_S8_L4x2(spu,spx,c,8,32,288,0x7)
	mSTORE4(d_c   ,&c[0],256,)
	mSTORE4(d_c+32,&c[4],256,)
}
__global__ void d_ifft64x16x( float2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[16*72];
	float2 c[8], RF[7], temp;
	d_c+=(blockIdx.x<<10)+(threadIdx.y<<6)+threadIdx.x;
	unsigned int lane=threadIdx.x&7;
	unsigned int slot=(threadIdx.y<<2)+(threadIdx.x>>3);
	float* spx=&smem[ 72*threadIdx.y+threadIdx.x];
	float* spy=&smem[288*threadIdx.y+threadIdx.x];
	float* spu=&smem[72*slot+lane];
	float* spv=&smem[72*slot+9*lane];
	RF[0]=d_RF[threadIdx.y<<2];
	RF[0].y=-RF[0].y;
	mLOAD4(&c[0],d_c   ,256,)
	mLOAD4(&c[4],d_c+32,256,)
	mCALRF4(RF)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mHMRF4(&c[0],RF)
	mHMRF4(&c[4],RF)
	mPERMUTE4x2(spx,spy,c,32,288,32,72,0xf)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	RF[0]=d_RF[lane];
	RF[0].y=-RF[0].y;
	mCALRF8(RF)
	mPERMUTE_S4x2_L8(spx,spu,c,32,288,8,0xf)
	mFFT8(c,i)
	mHMRF8(c,RF)
	mPERMUTE(8,spu,spv,c,9,1,0xf)
	mFFT8(c,i)
	mPERMUTE_S8_L4x2(spu,spx,c,8,32,288,0x7)
	mSTORE4(d_c   ,&c[0],256,)
	mSTORE4(d_c+32,&c[4],256,)
}
__global__ void d_fft64x32x( float2* d_c, const float2* __restrict__ d_RF )
{											
	__shared__ float smem[32][65];
	float2 c[8], RF[7], temp;
	d_c+=((blockIdx.x<<11)+(threadIdx.y<<6)+threadIdx.x);
	unsigned int lane=threadIdx.x&31;
	unsigned int slot=(threadIdx.x>>5)+(threadIdx.y<<1);
	float* spx=&smem[  threadIdx.y][threadIdx.x];
	float* spy=&smem[4*threadIdx.y][threadIdx.x];
	float* spu=&smem[lane][  slot];
	float* spv=&smem[lane][8*slot];
	RF[0]=__fldu2(&d_RF[threadIdx.y<<1]);
	mLOAD8(c,d_c,256,)
	mCALRF8(RF)
	mFFT8(c,)
	mHMRF8(c,RF)
	mPERMUTE_S8_L4x2(spx,spy,c,260,1040,65,0xf)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mPERMUTE_S4x2_L8(spx,spu,c,260,520,8,0xf)
	RF[0]=d_RF[slot];
	mCALRF8(RF)
	mFFT8(c,)
	mHMRF8(c,RF)
	mPERMUTE(8,spu,spv,c,8,1,0xf)
	mFFT8(c,)
	mPERMUTE(8,spu,spx,c,8,260,0x7)
	mSTORE8(d_c,c,256,)
}
__global__ void d_ifft64x32x( float2* d_c, const float2* __restrict__ d_RF )
{											
	__shared__ float smem[32][65];
	float2 c[8], RF[7], temp;
	d_c+=((blockIdx.x<<11)+(threadIdx.y<<6)+threadIdx.x);
	unsigned int lane=threadIdx.x&31;
	unsigned int slot=(threadIdx.x>>5)+(threadIdx.y<<1);
	float* spx=&smem[  threadIdx.y][threadIdx.x];
	float* spy=&smem[4*threadIdx.y][threadIdx.x];
	float* spu=&smem[lane][  slot];
	float* spv=&smem[lane][8*slot];
	RF[0]=__fldu2(&d_RF[threadIdx.y<<1]);
	RF[0].y=-RF[0].y;
	mLOAD8(c,d_c,256,)
	mCALRF8(RF)
	mFFT8(c,i)
	mHMRF8(c,RF)
	mPERMUTE_S8_L4x2(spx,spy,c,260,1040,65,0xf)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mPERMUTE_S4x2_L8(spx,spu,c,260,520,8,0xf)
	RF[0]=d_RF[slot];
	RF[0].y=-RF[0].y;
	mCALRF8(RF)
	mFFT8(c,i)
	mHMRF8(c,RF)
	mPERMUTE(8,spu,spv,c,8,1,0xf)
	mFFT8(c,i)
	mPERMUTE(8,spu,spx,c,8,260,0x7)
	mSTORE8(d_c,c,256,)
}
__global__ void d_fft64x64x( float2* d_c, const float2* __restrict__ d_RF )
{											
	__shared__ float smem[64][65];
	float2 c[8], RF[7], temp;
	d_c+=(blockIdx.x<<12)+(threadIdx.y<<6)+threadIdx.x;	
	float* spx=&smem[  threadIdx.y][  threadIdx.x];
	float* spy=&smem[8*threadIdx.y][  threadIdx.x];
	float* spu=&smem[  threadIdx.x][  threadIdx.y];
	float* spv=&smem[  threadIdx.x][8*threadIdx.y];
	RF[0]=__fldu2(&d_RF[threadIdx.y]);
	mLOAD8(c,d_c,512,)
	mCALRF8(RF)
	mFFT8(c,)
	mHMRF8(c,RF)
	mPERMUTE(8,spx,spy,c,520,65,0xf)	
	mFFT8(c,)
	mPERMUTE(8,spy,spu,c,65,8,0xf)
	mFFT8(c,)
	mHMRF8(c,RF)
	mPERMUTE(8,spu,spv,c,8,1,0xf)
	mFFT8(c,)
	mPERMUTE(8,spu,spy,c,8,65,0x7)
	mSTORE8(d_c,c,512,)	
}
__global__ void d_ifft64x64x( float2* d_c, const float2* __restrict__ d_RF )
{											
	__shared__ float smem[64][65];
	float2 c[8], RF[7], temp;
	d_c+=(blockIdx.x<<12)+(threadIdx.y<<6)+threadIdx.x;	
	float* spx=&smem[  threadIdx.y][  threadIdx.x];
	float* spy=&smem[8*threadIdx.y][  threadIdx.x];
	float* spu=&smem[  threadIdx.x][  threadIdx.y];
	float* spv=&smem[  threadIdx.x][8*threadIdx.y];
	RF[0]=__fldu2(&d_RF[threadIdx.y]);
	RF[0].y=-RF[0].y;
	mLOAD8(c,d_c,512,)
	mCALRF8(RF)
	mFFT8(c,i)
	mHMRF8(c,RF)
	mPERMUTE(8,spx,spy,c,520,65,0xf)	
	mFFT8(c,i)
	mPERMUTE(8,spy,spu,c,65,8,0xf)
	mFFT8(c,i)
	mHMRF8(c,RF)
	mPERMUTE(8,spu,spv,c,8,1,0xf)
	mFFT8(c,i)
	mPERMUTE(8,spu,spy,c,8,65,0x7)
	mSTORE8(d_c,c,512,)	
}

#if SM!=37
#define LB_S64x128 __launch_bounds__(512,1)
#else
#define LB_S64x128 __launch_bounds__(512,3)
#endif

__global__ void LB_S64x128 d_fft64x128x( float2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[128*72];
	float2 c[16], RF[15], temp;
	unsigned int lane=threadIdx.x&7;
	unsigned int slot=(threadIdx.y<<3)+(threadIdx.x>>3);
	float* spx=&smem[ 72*threadIdx.y+threadIdx.x];
	float* spy=&smem[576*threadIdx.y+threadIdx.x];
	float* spu=&smem[72*slot+lane];
	float* spv=&smem[576*threadIdx.y+65*lane+8*(slot&7)];
	d_c+=(blockIdx.x<<13)+(threadIdx.y<<6)+threadIdx.x;
	RF[0]=d_RF[threadIdx.y];
	mLOAD16(c,d_c,512,)
	mCALRF16(RF)
	mFFT16(c,)
	mHMRF16(c,RF)
	RF[0]=d_RF[lane<<1];
	mPERMUTE_S16_L8x2(spx,spy,c,576,4608,72,0xf)
	mCALRF8(RF)
	mFFT8(&c[0],)
	mFFT8(&c[8],)
	mPERMUTE8x2(spx,spu,c,576,1152,4608,8,0xf)
	mFFT8(&c[0],)
	mFFT8(&c[8],)
	mHMRF8(&c[0],RF)
	mHMRF8(&c[8],RF)
	mPERMUTE8x2(spy,spv,c,4608,65,4608,1,0xf)
	mFFT8(&c[0],)
	mFFT8(&c[8],)
	mPERMUTE_S8x2_L16(spu,spx,c,4608,8,576,0x7)	
	mSTORE16(d_c,c,512,)
}
__global__ void LB_S64x128 d_ifft64x128x( float2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[128*72];
	float2 c[16], RF[15], temp;
	unsigned int lane=threadIdx.x&7;
	unsigned int slot=(threadIdx.y<<3)+(threadIdx.x>>3);
	float* spx=&smem[ 72*threadIdx.y+threadIdx.x];
	float* spy=&smem[576*threadIdx.y+threadIdx.x];
	float* spu=&smem[72*slot+lane];
	float* spv=&smem[576*threadIdx.y+65*lane+8*(slot&7)];
	d_c+=(blockIdx.x<<13)+(threadIdx.y<<6)+threadIdx.x;
	RF[0]=d_RF[threadIdx.y];
	RF[0].y=-RF[0].y;
	mLOAD16(c,d_c,512,)
	mCALRF16(RF)
	mFFT16(c,i)
	mHMRF16(c,RF)
	RF[0]=d_RF[lane<<1];
	RF[0].y=-RF[0].y;
	mPERMUTE_S16_L8x2(spx,spy,c,576,4608,72,0xf)
	mCALRF8(RF)
	mFFT8(&c[0],i)
	mFFT8(&c[8],i)
	mPERMUTE8x2(spx,spu,c,576,1152,4608,8,0xf)
	mFFT8(&c[0],i)
	mFFT8(&c[8],i)
	mHMRF8(&c[0],RF)
	mHMRF8(&c[8],RF)
	mPERMUTE8x2(spy,spv,c,4608,65,4608,1,0xf)
	mFFT8(&c[0],i)
	mFFT8(&c[8],i)
	mPERMUTE_S8x2_L16(spu,spx,c,4608,8,576,0x7)
	mSTORE16(d_c,c,512,)
}

//==============================================================================================================================================================

__global__ void d_mfft64x2x( ushort2* d_c, const float2* __restrict__ d_RF, int bat )
{											
	extern __shared__ float smem[];
	float2 c[8], RF[7], temp;
	ushort2 hc[8];
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	d_c+=(patch_id<<7)+threadIdx.x;
	unsigned int lane=threadIdx.x&7;
	unsigned int slot=threadIdx.x>>3;
	float* spx=&smem[144*patch_id+threadIdx.x];
	float* spy=&smem[144*patch_id+slot*72+lane];
	float* spz=&smem[144*patch_id+slot*72+9*lane];
	RF[0]=d_RF[lane];
	mLOAD2(&hc[0],d_c+0*16,64,)
	mLOAD2(&hc[2],d_c+1*16,64,)
	mLOAD2(&hc[4],d_c+2*16,64,)
	mLOAD2(&hc[6],d_c+3*16,64,)
	mH2Sx8(c,hc)
	mCALRF8(RF)
	mFFT2(&c[0],)
	mFFT2(&c[2],)
	mFFT2(&c[4],)
	mFFT2(&c[6],)
	mISTORE2(spx+0*16,&c[0],72,.x)
	mISTORE2(spx+1*16,&c[2],72,.x)
	mISTORE2(spx+2*16,&c[4],72,.x)
	mISTORE2(spx+3*16,&c[6],72,.x)
	mLOAD8(c,spy,8,.x)
	mISTORE2(spx+0*16,&c[0],72,.y)
	mISTORE2(spx+1*16,&c[2],72,.y)
	mISTORE2(spx+2*16,&c[4],72,.y)
	mISTORE2(spx+3*16,&c[6],72,.y)

	mLOAD8(c,spy,8,.y)
	mFFT8(c,)
	mHMRF8(c,RF)
	mPERMUTE(8,spy,spz,c,9,1,0)
	mFFT8(c,)
	mISTORE8(spy,c,8,.x)
	mLOAD2(&c[0],spx+0*16,72,.x)
	mLOAD2(&c[2],spx+1*16,72,.x)
	mLOAD2(&c[4],spx+2*16,72,.x)
	mLOAD2(&c[6],spx+3*16,72,.x)
	mISTORE8(spy,c,8,.y)
	mLOAD2(&c[0],spx+0*16,72,.y)
	mLOAD2(&c[2],spx+1*16,72,.y)
	mLOAD2(&c[4],spx+2*16,72,.y)
	mLOAD2(&c[6],spx+3*16,72,.y)
	mS2Hx8(hc,c)
	mSTORE2(d_c+0*16,&hc[0],64,)
	mSTORE2(d_c+1*16,&hc[2],64,)
	mSTORE2(d_c+2*16,&hc[4],64,)
	mSTORE2(d_c+3*16,&hc[6],64,)
}
__global__ void d_imfft64x2x( ushort2* d_c, const float2* __restrict__ d_RF, int bat )
{											
	extern __shared__ float smem[];
	float2 c[8], RF[7], temp;
	ushort2 hc[8];
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	d_c+=(patch_id<<7)+threadIdx.x;
	unsigned int lane=threadIdx.x&7;
	unsigned int slot=threadIdx.x>>3;
	float* spx=&smem[144*patch_id+threadIdx.x];
	float* spy=&smem[144*patch_id+slot*72+lane];
	float* spz=&smem[144*patch_id+slot*72+9*lane];
	RF[0]=d_RF[lane];
	RF[0].y=-RF[0].y;
	mLOAD2(&hc[0],d_c+0*16,64,)
	mLOAD2(&hc[2],d_c+1*16,64,)
	mLOAD2(&hc[4],d_c+2*16,64,)
	mLOAD2(&hc[6],d_c+3*16,64,)
	mH2Sx8(c,hc)
	mCALRF8(RF)
	mFFT2(&c[0],)
	mFFT2(&c[2],)
	mFFT2(&c[4],)
	mFFT2(&c[6],)
	mISTORE2(spx+0*16,&c[0],72,.x)
	mISTORE2(spx+1*16,&c[2],72,.x)
	mISTORE2(spx+2*16,&c[4],72,.x)
	mISTORE2(spx+3*16,&c[6],72,.x)
	mLOAD8(c,spy,8,.x)
	mISTORE2(spx+0*16,&c[0],72,.y)
	mISTORE2(spx+1*16,&c[2],72,.y)
	mISTORE2(spx+2*16,&c[4],72,.y)
	mISTORE2(spx+3*16,&c[6],72,.y)

	mLOAD8(c,spy,8,.y)
	mFFT8(c,i)
	mHMRF8(c,RF)
	mPERMUTE(8,spy,spz,c,9,1,0)
	mFFT8(c,i)
	mISTORE8(spy,c,8,.x)
	mLOAD2(&c[0],spx+0*16,72,.x)
	mLOAD2(&c[2],spx+1*16,72,.x)
	mLOAD2(&c[4],spx+2*16,72,.x)
	mLOAD2(&c[6],spx+3*16,72,.x)
	mISTORE8(spy,c,8,.y)
	mLOAD2(&c[0],spx+0*16,72,.y)
	mLOAD2(&c[2],spx+1*16,72,.y)
	mLOAD2(&c[4],spx+2*16,72,.y)
	mLOAD2(&c[6],spx+3*16,72,.y)
	mS2Hx8(hc,c)
	mSTORE2(d_c+0*16,&hc[0],64,)
	mSTORE2(d_c+1*16,&hc[2],64,)
	mSTORE2(d_c+2*16,&hc[4],64,)
	mSTORE2(d_c+3*16,&hc[6],64,)
}
__global__ void d_mfft64x4x( ushort2* d_c, const float2* __restrict__ d_RF, int bat )
{											
	extern __shared__ float smem[];
	float2 c[8], RF[7], temp;
	ushort2 hc[8];
	unsigned int patch_id=(blockIdx.x*blockDim.y)+threadIdx.y;
	if(patch_id>=bat) return;
	d_c+=(patch_id<<8)+threadIdx.x;
	unsigned int lane=threadIdx.x&7;
	unsigned int slot=threadIdx.x>>3;
	float* spx=&smem[288*patch_id+threadIdx.x];
	float* spy=&smem[288*patch_id+slot*72+lane];
	float* spz=&smem[288*patch_id+slot*72+9*lane];
	RF[0]=d_RF[lane];
	mLOAD4x2(hc,d_c,32,64,)
	mH2Sx8(c,hc)
	mCALRF8(RF)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mISTORE4(spx+0*32,&c[0],72,.x)
	mISTORE4(spx+1*32,&c[4],72,.x)
	mLOAD8(c,spy,8,.x)
	mISTORE4(spx+0*32,&c[0],72,.y)
	mISTORE4(spx+1*32,&c[4],72,.y)
	mLOAD8(c,spy,8,.y)
	mFFT8(c,)
	mHMRF8(c,RF)
	mPERMUTE(8,spy,spz,c,9,1,0)
	mFFT8(c,)
	mPERMUTE_S8_L4x2(spy,spx,c,8,32,72,0)
	mS2Hx8(hc,c)
	mSTORE4x2(d_c,hc,32,64)
}
__global__ void d_imfft64x4x( ushort2* d_c, const float2* __restrict__ d_RF, int bat )
{											
	extern __shared__ float smem[];
	float2 c[8], RF[7], temp;
	ushort2 hc[8];
	unsigned int patch_id=blockIdx.x*blockDim.y+threadIdx.y;
	if(patch_id>=bat) return;
	d_c+=(patch_id<<8)+threadIdx.x;
	unsigned int lane=threadIdx.x&7;
	unsigned int slot=threadIdx.x>>3;
	float* spx=&smem[288*patch_id+threadIdx.x];
	float* spy=&smem[288*patch_id+slot*72+lane];
	float* spz=&smem[288*patch_id+slot*72+9*lane];
	RF[0]=d_RF[lane];
	RF[0].y=-RF[0].y;
	mLOAD4x2(hc,d_c,32,64,)
	mH2Sx8(c,hc)
	mCALRF8(RF)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mISTORE4(spx+0*32,&c[0],72,.x)
	mISTORE4(spx+1*32,&c[4],72,.x)
	mLOAD8(c,spy,8,.x)	
	mISTORE4(spx+0*32,&c[0],72,.y)
	mISTORE4(spx+1*32,&c[4],72,.y)
	mLOAD8(c,spy,8,.y)
	mFFT8(c,i)
	mHMRF8(c,RF)
	mPERMUTE(8,spy,spz,c,9,1,0)
	mFFT8(c,i)
	mPERMUTE_S8_L4x2(spy,spx,c,8,32,72,0)
	mS2Hx8(hc,c)
	mSTORE4x2(d_c,hc,32,64)
}
__global__ void d_mfft64x8x( ushort2* d_c, const float2* __restrict__ d_RF )
{											
	__shared__ float smem[8*72];
	float2 c[8], RF[7], temp;
	ushort2 hc[8];
	d_c+=(blockIdx.x<<9)+threadIdx.x;
	unsigned int lane=threadIdx.x&7;
	unsigned int slot=threadIdx.x>>3;
	float* spx=&smem[threadIdx.x];
	float* spy=&smem[72*slot+lane];
	float* spz=&smem[72*slot+9*lane];
	RF[0]=d_RF[lane];
	mLOAD8(hc,d_c,64,)
	mH2Sx8(c,hc)
	mCALRF8(RF)
	mFFT8(c,)
	mPERMUTE(8,spx,spy,c,72,8,0xf)
	mFFT8(c,)
	mHMRF8(c,RF)
	mPERMUTE(8,spy,spz,c,9,1,0xf)
	mFFT8(c,)
	mPERMUTE(8,spy,spx,c,8,72,0x7)
	mS2Hx8(hc,c)
	mSTORE8(d_c,hc,64,)
}
__global__ void d_imfft64x8x( ushort2* d_c, const float2* __restrict__ d_RF )
{											
	__shared__ float smem[8*72];
	float2 c[8], RF[7], temp;
	ushort2 hc[8];
	d_c+=(blockIdx.x<<9)+threadIdx.x;
	unsigned int lane=threadIdx.x&7;
	unsigned int slot=threadIdx.x>>3;
	float* spx=&smem[threadIdx.x];
	float* spy=&smem[72*slot+lane];
	float* spz=&smem[72*slot+9*lane];
	RF[0]=d_RF[lane];
	RF[0].y=-RF[0].y;
	mLOAD8(hc,d_c,64,)
	mH2Sx8(c,hc)
	mCALRF8(RF)
	mFFT8(c,i)
	mPERMUTE(8,spx,spy,c,72,8,0xf)
	mFFT8(c,i)
	mHMRF8(c,RF)
	mPERMUTE(8,spy,spz,c,9,1,0xf)
	mFFT8(c,i)
	mPERMUTE(8,spy,spx,c,8,72,0x7)
	mS2Hx8(hc,c)
	mSTORE8(d_c,hc,64,)
}
__global__ void d_mfft64x16x( ushort2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[16*72];
	float2 c[8], RF[7], temp;
	ushort2 hc[8];
	d_c+=(blockIdx.x<<10)+(threadIdx.y<<6)+threadIdx.x;
	unsigned int lane=threadIdx.x&7;
	unsigned int slot=(threadIdx.y<<2)+(threadIdx.x>>3);
	float* spx=&smem[ 72*threadIdx.y+threadIdx.x];
	float* spy=&smem[288*threadIdx.y+threadIdx.x];
	float* spu=&smem[72*slot+lane];
	float* spv=&smem[72*slot+9*lane];
	RF[0]=d_RF[threadIdx.y<<2];
	mLOAD4(&hc[0],d_c   ,256,)
	mLOAD4(&hc[4],d_c+32,256,)
	mH2Sx8(c,hc)
	mCALRF4(RF)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mHMRF4(&c[0],RF)
	mHMRF4(&c[4],RF)
	mPERMUTE4x2(spx,spy,c,32,288,32,72,0xf)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	RF[0]=d_RF[lane];
	mCALRF8(RF)
	mPERMUTE_S4x2_L8(spx,spu,c,32,288,8,0xf)
	mFFT8(c,)
	mHMRF8(c,RF)
	mPERMUTE(8,spu,spv,c,9,1,0xf)
	mFFT8(c,)
	mPERMUTE_S8_L4x2(spu,spx,c,8,32,288,0x7)
	mS2Hx8(hc,c)
	mSTORE4(d_c   ,&hc[0],256,)
	mSTORE4(d_c+32,&hc[4],256,)
}
__global__ void d_imfft64x16x( ushort2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[16*72];
	float2 c[8], RF[7], temp;
	ushort2 hc[8];
	d_c+=(blockIdx.x<<10)+(threadIdx.y<<6)+threadIdx.x;
	unsigned int lane=threadIdx.x&7;
	unsigned int slot=(threadIdx.y<<2)+(threadIdx.x>>3);
	float* spx=&smem[ 72*threadIdx.y+threadIdx.x];
	float* spy=&smem[288*threadIdx.y+threadIdx.x];
	float* spu=&smem[72*slot+lane];
	float* spv=&smem[72*slot+9*lane];
	RF[0]=d_RF[threadIdx.y<<2];
	RF[0].y=-RF[0].y;
	mLOAD4(&hc[0],d_c   ,256,)
	mLOAD4(&hc[4],d_c+32,256,)
	mH2Sx8(c,hc)
	mCALRF4(RF)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mHMRF4(&c[0],RF)
	mHMRF4(&c[4],RF)
	mPERMUTE4x2(spx,spy,c,32,288,32,72,0xf)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	RF[0]=d_RF[lane];
	RF[0].y=-RF[0].y;
	mCALRF8(RF)
	mPERMUTE_S4x2_L8(spx,spu,c,32,288,8,0xf)
	mFFT8(c,i)
	mHMRF8(c,RF)
	mPERMUTE(8,spu,spv,c,9,1,0xf)
	mFFT8(c,i)
	mPERMUTE_S8_L4x2(spu,spx,c,8,32,288,0x7)
	mS2Hx8(hc,c)
	mSTORE4(d_c   ,&hc[0],256,)
	mSTORE4(d_c+32,&hc[4],256,)
}
__global__ void d_mfft64x32x( ushort2* d_c, const float2* __restrict__ d_RF )
{											
	__shared__ float smem[32][65];
	float2 c[8], RF[7], temp;
	ushort2 hc[8];
	d_c+=((blockIdx.x<<11)+(threadIdx.y<<6)+threadIdx.x);
	unsigned int lane=threadIdx.x&31;
	unsigned int slot=(threadIdx.x>>5)+(threadIdx.y<<1);
	float* spx=&smem[  threadIdx.y][threadIdx.x];
	float* spy=&smem[4*threadIdx.y][threadIdx.x];
	float* spu=&smem[lane][  slot];
	float* spv=&smem[lane][8*slot];
	RF[0]=__fldu2(&d_RF[threadIdx.y<<1]);
	mLOAD8(hc,d_c,256,)
	mH2Sx8(c,hc)
	mCALRF8(RF)
	mFFT8(c,)
	mHMRF8(c,RF)
	mPERMUTE_S8_L4x2(spx,spy,c,260,1040,65,0xf)
	mFFT4(&c[0],)
	mFFT4(&c[4],)
	mPERMUTE_S4x2_L8(spx,spu,c,260,520,8,0xf)
	RF[0]=d_RF[slot];
	mCALRF8(RF)
	mFFT8(c,)
	mHMRF8(c,RF)
	mPERMUTE(8,spu,spv,c,8,1,0xf)
	mFFT8(c,)
	mPERMUTE(8,spu,spx,c,8,260,0x7)
	mS2Hx8(hc,c)
	mSTORE8(d_c,hc,256,)
}
__global__ void d_imfft64x32x( ushort2* d_c, const float2* __restrict__ d_RF )
{											
	__shared__ float smem[32][65];
	float2 c[8], RF[7], temp;
	ushort2 hc[8];
	d_c+=((blockIdx.x<<11)+(threadIdx.y<<6)+threadIdx.x);
	unsigned int lane=threadIdx.x&31;
	unsigned int slot=(threadIdx.x>>5)+(threadIdx.y<<1);
	float* spx=&smem[  threadIdx.y][threadIdx.x];
	float* spy=&smem[4*threadIdx.y][threadIdx.x];
	float* spu=&smem[lane][  slot];
	float* spv=&smem[lane][8*slot];
	RF[0]=__fldu2(&d_RF[threadIdx.y<<1]);
	RF[0].y=-RF[0].y;
	mLOAD8(hc,d_c,256,)
	mH2Sx8(c,hc)
	mCALRF8(RF)
	mFFT8(c,i)
	mHMRF8(c,RF)
	mPERMUTE_S8_L4x2(spx,spy,c,260,1040,65,0xf)
	mFFT4(&c[0],i)
	mFFT4(&c[4],i)
	mPERMUTE_S4x2_L8(spx,spu,c,260,520,8,0xf)
	RF[0]=d_RF[slot];
	RF[0].y=-RF[0].y;
	mCALRF8(RF)
	mFFT8(c,i)
	mHMRF8(c,RF)
	mPERMUTE(8,spu,spv,c,8,1,0xf)
	mFFT8(c,i)
	mPERMUTE(8,spu,spx,c,8,260,0x7)
	mS2Hx8(hc,c)
	mSTORE8(d_c,hc,256,)
}
__global__ void d_mfft64x64x( ushort2* d_c, const float2* __restrict__ d_RF )
{											
	__shared__ float smem[64][65];
	float2 c[8], RF[7], temp;
	ushort2 hc[8];
	d_c+=(blockIdx.x<<12)+(threadIdx.y<<6)+threadIdx.x;	
	float* spx=&smem[  threadIdx.y][  threadIdx.x];
	float* spy=&smem[8*threadIdx.y][  threadIdx.x];
	float* spu=&smem[  threadIdx.x][  threadIdx.y];
	float* spv=&smem[  threadIdx.x][8*threadIdx.y];
	RF[0]=__fldu2(&d_RF[threadIdx.y]);
	mLOAD8(hc,d_c,512,)
	mH2Sx8(c,hc)
	mCALRF8(RF)
	mFFT8(c,)
	mHMRF8(c,RF)
	mPERMUTE(8,spx,spy,c,520,65,0xf)	
	mFFT8(c,)
	mPERMUTE(8,spy,spu,c,65,8,0xf)
	mFFT8(c,)
	mHMRF8(c,RF)
	mPERMUTE(8,spu,spv,c,8,1,0xf)
	mFFT8(c,)
	mPERMUTE(8,spu,spy,c,8,65,0x7)
	mS2Hx8(hc,c)
	mSTORE8(d_c,hc,512,)	
}
__global__ void d_imfft64x64x( ushort2* d_c, const float2* __restrict__ d_RF )
{											
	__shared__ float smem[64][65];
	float2 c[8], RF[7], temp;
	ushort2 hc[8];
	d_c+=(blockIdx.x<<12)+(threadIdx.y<<6)+threadIdx.x;	
	float* spx=&smem[  threadIdx.y][  threadIdx.x];
	float* spy=&smem[8*threadIdx.y][  threadIdx.x];
	float* spu=&smem[  threadIdx.x][  threadIdx.y];
	float* spv=&smem[  threadIdx.x][8*threadIdx.y];
	RF[0]=__fldu2(&d_RF[threadIdx.y]);
	RF[0].y=-RF[0].y;
	mLOAD8(hc,d_c,512,)
	mH2Sx8(c,hc)
	mCALRF8(RF)
	mFFT8(c,i)
	mHMRF8(c,RF)
	mPERMUTE(8,spx,spy,c,520,65,0xf)	
	mFFT8(c,i)
	mPERMUTE(8,spy,spu,c,65,8,0xf)
	mFFT8(c,i)
	mHMRF8(c,RF)
	mPERMUTE(8,spu,spv,c,8,1,0xf)
	mFFT8(c,i)
	mPERMUTE(8,spu,spy,c,8,65,0x7)
	mS2Hx8(hc,c)
	mSTORE8(d_c,hc,512,)	
}
__global__ void LB_S64x128 d_mfft64x128x( ushort2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[128*72];
	float2 c[16], RF[15], temp;
	ushort2 hc[16];
	unsigned int lane=threadIdx.x&7;
	unsigned int slot=(threadIdx.y<<3)+(threadIdx.x>>3);
	float* spx=&smem[ 72*threadIdx.y+threadIdx.x];
	float* spy=&smem[576*threadIdx.y+threadIdx.x];
	float* spu=&smem[72*slot+lane];
	float* spv=&smem[576*threadIdx.y+65*lane+8*(slot&7)];
	d_c+=(blockIdx.x<<13)+(threadIdx.y<<6)+threadIdx.x;
	RF[0]=d_RF[threadIdx.y];
	mLOAD16(hc,d_c,512,)
	mH2Sx16(c,hc)
	mCALRF16(RF)
	mFFT16(c,)
	mHMRF16(c,RF)
	RF[0]=d_RF[lane<<1];
	mPERMUTE_S16_L8x2(spx,spy,c,576,4608,72,0xf)
	mCALRF8(RF)
	mFFT8(&c[0],)
	mFFT8(&c[8],)
	mPERMUTE8x2(spx,spu,c,576,1152,4608,8,0xf)
	mFFT8(&c[0],)
	mFFT8(&c[8],)
	mHMRF8(&c[0],RF)
	mHMRF8(&c[8],RF)
	mPERMUTE8x2(spy,spv,c,4608,65,4608,1,0xf)
	mFFT8(&c[0],)
	mFFT8(&c[8],)
	mPERMUTE_S8x2_L16(spu,spx,c,4608,8,576,0x7)
	mS2Hx16(hc,c)
	mSTORE16(d_c,hc,512,)
}
__global__ void LB_S64x128 d_imfft64x128x( ushort2* d_c, const float2* __restrict__ d_RF )
{
	__shared__ float smem[128*72];
	float2 c[16], RF[15], temp;
	ushort2 hc[16];
	unsigned int lane=threadIdx.x&7;
	unsigned int slot=(threadIdx.y<<3)+(threadIdx.x>>3);
	float* spx=&smem[ 72*threadIdx.y+threadIdx.x];
	float* spy=&smem[576*threadIdx.y+threadIdx.x];
	float* spu=&smem[72*slot+lane];
	float* spv=&smem[576*threadIdx.y+65*lane+8*(slot&7)];
	d_c+=(blockIdx.x<<13)+(threadIdx.y<<6)+threadIdx.x;
	RF[0]=d_RF[threadIdx.y];
	RF[0].y=-RF[0].y;
	mLOAD16(hc,d_c,512,)
	mH2Sx16(c,hc)
	mCALRF16(RF)
	mFFT16(c,i)
	mHMRF16(c,RF)
	RF[0]=d_RF[lane<<1];
	RF[0].y=-RF[0].y;
	mPERMUTE_S16_L8x2(spx,spy,c,576,4608,72,0xf)
	mCALRF8(RF)
	mFFT8(&c[0],i)
	mFFT8(&c[8],i)
	mPERMUTE8x2(spx,spu,c,576,1152,4608,8,0xf)
	mFFT8(&c[0],i)
	mFFT8(&c[8],i)
	mHMRF8(&c[0],RF)
	mHMRF8(&c[8],RF)
	mPERMUTE8x2(spy,spv,c,4608,65,4608,1,0xf)
	mFFT8(&c[0],i)
	mFFT8(&c[8],i)
	mPERMUTE_S8x2_L16(spu,spx,c,4608,8,576,0x7)
	mS2Hx16(hc,c)
	mSTORE16(d_c,hc,512,)
}
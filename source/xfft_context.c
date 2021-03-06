#define _ALLOW_ITERATOR_DEBUG_LEVEL_MISMATCH
#pragma warning( disable:4996 )
#include"../include/xfft_context.h"

static const unsigned int long long kbin_sm20[]=
{
#if defined(_M_X64)||defined(_M_AMD64)||defined(__x86_64)||defined(_M_IA64)||defined(__LP64__)
#include"../include/dev/kbin64_sm20.h"
#else
#include"../include/dev/kbin32_sm20.h"
#endif
};
static const unsigned int long long kbin_sm21[]=
{
#if defined(_M_X64)||defined(_M_AMD64)||defined(__x86_64)||defined(_M_IA64)||defined(__LP64__)
#include"../include/dev/kbin64_sm21.h"
#else
#include"../include/dev/kbin32_sm21.h"
#endif
};
static const unsigned int long long kbin_sm30[]=
{
#if defined(_M_X64)||defined(_M_AMD64)||defined(__x86_64)||defined(_M_IA64)||defined(__LP64__)
#include"../include/dev/kbin64_sm30.h"
#else
#include"../include/dev/kbin32_sm30.h"
#endif
};
static const unsigned int long long kbin_sm32[]=
{
#if defined(_M_X64)||defined(_M_AMD64)||defined(__x86_64)||defined(_M_IA64)||defined(__LP64__)
#include"../include/dev/kbin64_sm32.h"
#else
#include"../include/dev/kbin32_sm32.h"
#endif
};
static const unsigned int long long kbin_sm35[]=
{
#if defined(_M_X64)||defined(_M_AMD64)||defined(__x86_64)||defined(_M_IA64)||defined(__LP64__)
#include"../include/dev/kbin64_sm35.h"
#else
#include"../include/dev/kbin32_sm35.h"
#endif
};
static const unsigned int long long kbin_sm37[]=
{
#if defined(_M_X64)||defined(_M_AMD64)||defined(__x86_64)||defined(_M_IA64)||defined(__LP64__)
#include"../include/dev/kbin64_sm37.h"
#else
#include"../include/dev/kbin32_sm37.h"
#endif
};
static const unsigned int long long kbin_sm50[]=
{
#if defined(_M_X64)||defined(_M_AMD64)||defined(__x86_64)||defined(_M_IA64)||defined(__LP64__)
#include"../include/dev/kbin64_sm50.h"
#else
#include"../include/dev/kbin32_sm50.h"
#endif
};
static const unsigned int long long kbin_sm52[]=
{
#if defined(_M_X64)||defined(_M_AMD64)||defined(__x86_64)||defined(_M_IA64)||defined(__LP64__)
#include"../include/dev/kbin64_sm52.h"
#else
#include"../include/dev/kbin32_sm52.h"
#endif
};
static const unsigned int long long kbin_sm53[]=
{
#if defined(_M_X64)||defined(_M_AMD64)||defined(__x86_64)||defined(_M_IA64)||defined(__LP64__)
#include"../include/dev/kbin64_sm53.h"
#else
#include"../include/dev/kbin32_sm53.h"
#endif
};

/****************************************************************************************************************************************************************
================================================================================================================================================================
/***************************************************************************************************************************************************************/

int xfft_context_create( xfft_context_t* const p_ctx )
{
	void* p_devbin;
	int vapi;	
	cuDriverGetVersion(&vapi);
	if(vapi<7000) 
		return XFFT_ERROR_INVALID_DRIVER;
	cuDevicePrimaryCtxRetain( &p_ctx->ctx, p_ctx->dev );
	cuDevicePrimaryCtxSetFlags( p_ctx->dev, CU_CTX_LMEM_RESIZE_TO_MAX );
	cuCtxPushCurrent( p_ctx->ctx );
	switch(p_ctx->arch)
	{
	case 20: p_devbin=(void*)kbin_sm20; break;
	case 21: p_devbin=(void*)kbin_sm21; break;
	case 30: p_devbin=(void*)kbin_sm30; break;
	case 32: p_devbin=(void*)kbin_sm32; break;
	case 35: p_devbin=(void*)kbin_sm35; break;
	case 37: p_devbin=(void*)kbin_sm37; break;
	case 50: p_devbin=(void*)kbin_sm50; break;
	case 52: p_devbin=(void*)kbin_sm52; break;
	case 53: p_devbin=(void*)kbin_sm53; break;
	}
	if(cuModuleLoadFatBinary( &p_ctx->module, p_devbin )!=CUDA_SUCCESS){
		cuDevicePrimaryCtxRelease(p_ctx->dev);
		p_ctx->ctx=NULL;
		return XFFT_ERROR_OUT_OF_MEMORY;
	}
	cuDeviceGetAttribute( &p_ctx->alignment	, CU_DEVICE_ATTRIBUTE_TEXTURE_ALIGNMENT	, p_ctx->dev );
	cuDeviceGetAttribute( &p_ctx->max_nblk_x, CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_X	, p_ctx->dev );
	cuDeviceGetAttribute( &p_ctx->max_nblk_y, CU_DEVICE_ATTRIBUTE_MAX_GRID_DIM_Y	, p_ctx->dev );
	cuCtxPopCurrent(NULL);
	return XFFT_SUCCESS;
}
void xfft_context_release( xfft_context_t* const p_ctx )
{
	if( p_ctx->ctx!=NULL ){
		xfft_context_bind(p_ctx);
		cuModuleUnload(p_ctx->module);
		xfft_context_unbind();
		cuDevicePrimaryCtxRelease(p_ctx->dev);
		p_ctx->ctx=NULL;
	}
}
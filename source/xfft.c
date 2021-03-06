#include"../include/xfft.h"
#include"../include/xfft_op.h"

static CUdevice		 *	g_devices=0;
static xfft_context_t*	g_pCtx=0;
static float2*			g_pTemp=0;
static int				g_dev_count;

#if	defined(_M_X64)||defined(_M_AMD64)||defined(__x86_64)||defined(_M_IA64)||defined(__LP64__)
	#define XFFT_MAX_HSIZE	(1<<16)
	#define XFFT_MAX_VSIZE	(1<<16)
	#define XFFT_MAX_SIZE	(1<<28)
#else
	#define XFFT_MAX_HSIZE	(1<<16)
	#define XFFT_MAX_VSIZE	(1<<16)
	#define XFFT_MAX_SIZE	(1<<27)
#endif

#define MAX_DEVICES 32

static int __get_devices( CUdevice* p_device, int* p_arch )
{	
	struct{ int x, y; } cc;
	CUdevice device;
	int i, n_devices, sm, n_valided;	
	cuDeviceGetCount( &n_devices );
	if( n_devices<=0 ) return -1;
	for( i=0, n_valided=0; i<n_devices; ++i )
	{
		cuDeviceGet( &device, i );
		cuDeviceGetAttribute( &cc.x, CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MAJOR, device );
		cuDeviceGetAttribute( &cc.y, CU_DEVICE_ATTRIBUTE_COMPUTE_CAPABILITY_MINOR, device );
		sm=10*cc.x+cc.y;
		if((sm<20)|(sm>=60)) continue;
		p_device[n_valided]=device;
		p_arch[n_valided]=sm;
		++n_valided;
	}
	return n_valided;
}

XFFTAPIENTRY xfftStatus_t XFFTAPICALL xfftInit()
{
	int cc[MAX_DEVICES], i, s;
	if(cuInit(0)!=CUDA_SUCCESS)
		return xfftErrorInvalidDriver;	
	if((g_devices=(CUdevice*)malloc(MAX_DEVICES*sizeof(CUdevice)))==0)
		return xfftErrorOutOfMemory;
	g_dev_count=__get_devices( &g_devices[0], cc );
	if(g_dev_count<=0) 
		return xfftErrorInvalidDevice;
	if((g_pCtx=(xfft_context_t*)calloc(g_dev_count,sizeof(xfft_context_t)))==0)
		return xfftErrorOutOfMemory;
	i=0;
	do{
		g_pCtx[i].dev=g_devices[i];
		g_pCtx[i].arch=cc[i];
		s=xfft_context_create( &g_pCtx[i] );
	}while(((++i)<g_dev_count)&(s==XFFT_SUCCESS));
	if(s!=XFFT_SUCCESS){
		while((--i)>0){ xfft_context_release( &g_pCtx[i-1] ); }
		free(g_pCtx); g_pCtx=0;
		return (xfftStatus_t)s;
	}
	if((g_pTemp=(float2*)malloc(1<<19))==0){
		free(g_pCtx); g_pCtx=0;
		return xfftErrorOutOfMemory;
	}
	return xfftSuccess;
}
XFFTAPIENTRY xfftStatus_t XFFTAPICALL xfftSetDevice( int idev )
{
	if((idev<0)|(idev>=g_dev_count))
		return xfftErrorInvalidValue;
	xfft_context_bind(&g_pCtx[idev]);
	return xfftSuccess;
}
XFFTAPIENTRY xfftStatus_t XFFTAPICALL xfftCreateOp1d( xfftOp* Op, int idev, int n, int bat )
{
	int s, max_bat;
	if((idev<0)|(idev>=g_dev_count)|(n<=1)|(bat<=0))
		return xfftErrorInvalidValue;
	max_bat=(n<=8192)?g_pCtx[idev].max_nblk_x:g_pCtx[idev].max_nblk_y;
	if((n>XFFT_MAX_HSIZE)|(bat>max_bat))
		return xfftErrorOutMaxSizeSupported;	
	if(((*Op)=(xfftOp)malloc(sizeof(xfftOp_t)))==0)
		return xfftErrorOutOfMemory;
	if((s=xfft_createOp_1d((xfftOp_t*)(*Op),&g_pCtx[idev],n,bat,g_pTemp))!=XFFT_SUCCESS){
		free((void*)Op);
	}
	return (xfftStatus_t)s;
}
XFFTAPIENTRY xfftStatus_t XFFTAPICALL xfftCreateOp2d( xfftOp* Op, int idev, int nx, int ny, int bat )
{
	int b, max_bat, s, dim;
	dim=(nx>1)+(ny>1);
	b=(idev<0)|(idev>=g_dev_count)|(dim==0)|(bat<=0);
	if(b) return xfftErrorInvalidValue;
	max_bat=(nx<=8192)?g_pCtx[idev].max_nblk_x:g_pCtx[idev].max_nblk_y;
	max_bat=(dim>1)?g_pCtx[idev].max_nblk_y:max_bat;
	b=(nx>XFFT_MAX_HSIZE)|(ny>XFFT_MAX_VSIZE)|((nx*ny)>XFFT_MAX_SIZE)|(bat>max_bat);
	if(b) return xfftErrorOutMaxSizeSupported;	
	if(((*Op)=(xfftOp)malloc(sizeof(xfftOp_t)))==0)
		return xfftErrorOutOfMemory;
	if(dim==2){
		s=xfft_createOp_2d((xfftOp_t*)(*Op),&g_pCtx[idev],nx,ny,bat,g_pTemp);
	} else {
		s=xfft_createOp_1d((xfftOp_t*)(*Op),&g_pCtx[idev],(nx>1)?nx:ny,bat,g_pTemp);
	}
	if(s!=XFFT_SUCCESS){
		free((void*)Op);
	}
	return (xfftStatus_t)s;
}
XFFTAPIENTRY xfftStatus_t XFFTAPICALL xfftCreateOp3d( xfftOp* Op, int idev, int nx, int ny, int nz, int bat )
{
	int dim, b, max_bat, s, dx, dy;
	dim=(nx>1)+(ny>1)+(nz>1);
	b=(idev<0)|(idev>=g_dev_count)|(dim==0)|(bat<=0);
	if(b) return xfftErrorInvalidValue;
	max_bat=(nx<=8192)?g_pCtx[idev].max_nblk_x:g_pCtx[idev].max_nblk_y;
	max_bat=(dim>1)?g_pCtx[idev].max_nblk_y:max_bat;
	b=(nx>XFFT_MAX_HSIZE)|(ny>XFFT_MAX_VSIZE)|(nz>XFFT_MAX_VSIZE)|((nx*ny*nz)>XFFT_MAX_SIZE)|(bat>max_bat);
	if(b) return xfftErrorOutMaxSizeSupported;
	if(((*Op)=(xfftOp)malloc(sizeof(xfftOp_t)))==0)
		return xfftErrorOutOfMemory;
	if(dim==3){
		s=xfft_createOp_3d((xfftOp_t*)(*Op),&g_pCtx[idev],nx,ny,nz,bat,g_pTemp);
	} else
	if(dim==2){
		dx=(nx>1)?nx:ny;
		dy=(nx>1)?((ny>1)?ny:nz):nz;
		s=xfft_createOp_2d((xfftOp_t*)(*Op),&g_pCtx[idev],dx,dy,bat,g_pTemp);
	} else {
		dx=(nx>1)?nx:((ny>1)?ny:nz);
		s=xfft_createOp_1d((xfftOp_t*)(*Op),&g_pCtx[idev],dx,bat,g_pTemp);
	}
	if(s!=XFFT_SUCCESS){ free((void*)Op); }
	return (xfftStatus_t)s;
}
XFFTAPIENTRY xfftStatus_t XFFTAPICALL xfftExec( xfftOp hOp, CUdeviceptr* p_dOut, CUdeviceptr d_src, CUdeviceptr d_aux, int dir, CUstream s )
{
	if((dir!=0)&&(dir!=1)) return xfftErrorInvalidValue;
	*p_dOut=xfft_exec((xfftOp_t*)hOp,d_src,d_aux,dir,s);
	return xfftSuccess;
}
XFFTAPIENTRY xfftStatus_t XFFTAPICALL xfftDestroy( xfftOp Op )
{
	xfft_release((xfftOp_t*)Op);
	return xfftSuccess;
}
XFFTAPIENTRY xfftStatus_t XFFTAPICALL xfftExit()
{
	if(g_pCtx)
	{
		int i;
		for( i=0; i<g_dev_count; ++i ){
			xfft_context_release( &g_pCtx[i] );
		}
		free(g_pCtx); g_pCtx=0;
		free(g_pTemp);
	}
	return xfftSuccess;
}
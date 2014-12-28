#pragma once

#include <optixpp_namespace.h>

#include <string>



optix::Buffer getOptixBuffer(optix::Context optix_ctx,float* data,int n)
{
	optix::Buffer buf=optix_ctx->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT, n );
	float* gpu_buf=(float*)buf->map();
	memcpy(gpu_buf,data,sizeof(float)*n);
	buf->unmap();
	return buf;
}

class Distribution1D
{
public:
	Distribution1D(float* data,int n)
	{
		assert(data&&n>0);

		float inv_n=1.f/n;
		cdf=new float[n+1];
		cdf[0]=0.f;
		for(int i=1;i<n+1;++i)
		{
			cdf[i]=cdf[i-1]+data[i-1]*inv_n;
		}
		intValue=cdf[n];

		assert(intValue>0.f);
		if(intValue==0.f)
		{
			for(int i=1;i<n+1;++i)
				cdf[i]=float(i)*inv_n;
		}
		else
		{
			float inv_int=1.f/intValue;
			for(int i=1;i<n+1;++i)
				cdf[i]*=inv_int;
		}	
	}
	~Distribution1D()
	{
		if(cdf) delete[] cdf;
	}

	// can use getOptixBuffer to get the Buffer object
	float* cdf;
	float intValue;
};


class Distribution2D
{
public:
	Distribution2D(optix::Context optix_ctx,float* data,int nu,int nv)
	{
		float* pmargin=new float[nv];

		pConditionalV=optix_ctx->createBuffer( RT_BUFFER_INPUT, RT_FORMAT_FLOAT, nu+1, nv );
		float* gpu_pcondv=(float*)pConditionalV->map();
		for(int v=0;v<nv;++v)
		{
			Distribution1D dist1(data+v*nu,nu);			
			memcpy(gpu_pcondv,dist1.cdf,sizeof(float)*(nu+1)); // cdf nu+1
			pmargin[v]=dist1.intValue;
			gpu_pcondv+=(nu+1);
		}
		pConditionalV->unmap();
		
		Distribution1D dist1(pmargin,nv);
		delete [] pmargin;
		pMarginal=getOptixBuffer(optix_ctx,dist1.cdf,nv+1);  // cdf nv+1
	}

	// CDFs
	optix::Buffer pConditionalV; // 2 dimensional
	optix::Buffer pMarginal; // 1 dimensional
};



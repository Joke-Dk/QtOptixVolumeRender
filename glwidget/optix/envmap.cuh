#ifndef _TRACY_ENVMAP_CUH_
#define _TRACY_ENVMAP_CUH_


rtTextureSampler<float4, 2>		envmap;
rtBuffer<int, 2>              pixelIsSampled;
rtDeclareVariable(float,		envmap_scale,,); // store explicitly for envmap eval
//rtDeclareVariable(Frame,		envmap_frame,,); // store explicitly for envmap eval

//rtDeclareVariable(uint, envmapWidth,,); // nu
//rtDeclareVariable(uint, envmapHeight,,); // nv

// sample by 2D distribution
#include "envmap_distr.cuh"
// sample by M-H
//#include "envmap_metro.cuh"



static __device__ __inline__ float2 spherical_uv(const float3& w)
{
	return make_float2(INV_TWOPI*sphericalPhi(w),INV_PI*sphericalTheta(w));
}

static __device__ __inline__ float3 envmapEvalL(const float3& W)
{
	float3 w=W;
	float2 uv=spherical_uv(w);
	return make_float3(tex2D(envmap,uv.x,uv.y))*1.f;
	//return make_float3(1.f)*envmapMapPdf( uv.x, uv.y);//test pdf
}

static __device__ __inline__ float3 envmapEvalLandPdf(const float3& W)
{
	float3 w=W;
	float theta=sphericalTheta(w),
		phi=sphericalPhi(w);
	float u=INV_TWOPI*phi,v=INV_PI*theta;

	float3 L=make_float3(tex2D(envmap,u,v));

	float sintheta=max(sinf(theta),0.01f); // check this!!!
	float pdf=envmapMapPdf(u,v);//(sintheta);

	return L/pdf;
}


#endif
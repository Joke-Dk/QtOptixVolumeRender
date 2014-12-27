#ifndef _TRACY_ENVMAP_CUH_
#define _TRACY_ENVMAP_CUH_


rtTextureSampler<float4, 2>		envmap;
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
}

static __device__ __inline__ float4 envmapEvalLandPdf(const float3& ref_p,const float3& W)
{
	float3 w=W;
	float theta=sphericalTheta(w),
		phi=sphericalPhi(w);
	float u=M_TWOPI*phi,v=INV_PI*theta;

	float3 L=make_float3(tex2D(envmap,u,v))*envmap_scale;

	float sintheta=max(sinf(theta),0.01f); // check this!!!
	float pdf=envmapMapPdf(u,v)/(2.f*M_PIf*M_PIf*sintheta);

	return make_float4(L,pdf);
}


#endif
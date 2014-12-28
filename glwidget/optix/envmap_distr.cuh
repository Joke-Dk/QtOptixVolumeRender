#ifndef _TRACY_ENVMAP_DISTR_CUH_
#define _TRACY_ENVMAP_DISTR_CUH_




//////////////////////////////////////////////////////////////////////////
// importance sample envmap
rtBuffer<float, 1> cdfMarginal; // cdf(vi)  //  sum_j(vi,uj)
rtBuffer<float, 2> cdfConditional; // cdf(uj|vi)
rtDeclareVariable(uint, envmapWidth,,); // nu
rtDeclareVariable(uint, envmapHeight,,); // nv


// ----- sampling marginal cdf
#define ARRAY_GET(i) cdfMarginal[i]
// size of the pdf array, cdf array size is ARRAY_SIZE+1
#define ARRAY_SIZE envmapHeight
#define LOWER_BOUND_FUNC lower_bound
#define UPPER_BOUND_FUNC upper_bound

#include "discrete.cuh"

// return (sampled_u, pdf)
static __device__ __inline__ float2 sample_env_marginal(float u,uint* offset)
{
	// u must > 0, else max will handle it
	uint idx=UPPER_BOUND_FUNC(ARRAY_SIZE+1u,u);
	idx=max(idx,1u)-1u; // unsigned version for idx=max(0,idx-1)
	// cdf[0]=0, cdf[ARRAY_SIZE]=1.0
	// assert(u >= cdf[idx] && u < cdf[ix+1])

	// compute offset along CDF segment
	float du=(u-ARRAY_GET(idx))/(ARRAY_GET(idx+1)-ARRAY_GET(idx));

	*offset=idx;
	return make_float2(
		(idx+du)/ARRAY_SIZE,
		(ARRAY_GET(idx+1)-ARRAY_GET(idx))*ARRAY_SIZE // todo: check this
		);
}


// ----- sampling conditional cdf
#undef ARRAY_GET
#undef ARRAY_SIZE
#undef LOWER_BOUND_FUNC
#undef UPPER_BOUND_FUNC
#define ARRAY_GET(i) cdfConditional[make_uint2(i,v)]
#define ARRAY_SIZE envmapWidth
#define LOWER_BOUND_FUNC lower_bound
#define UPPER_BOUND_FUNC upper_bound


//#include "../core/discrete.cuh"

// return (sampled_u, pdf)
static __device__ __inline__ float2 sample_env_conditional(float u,uint v)
{
	// u must > 0, else max will handle it
	uint idx=UPPER_BOUND_FUNC(ARRAY_SIZE+1u,u,v);
	idx=max(idx,1u)-1u; // unsigned version for idx=max(0,idx-1)
	// cdf[0]=0, cdf[ARRAY_SIZE]=1.0
	// assert(u >= cdf[idx] && u < cdf[ix+1])

	// compute offset along CDF segment
	float du=(u-ARRAY_GET(idx))/(ARRAY_GET(idx+1)-ARRAY_GET(idx));

	return make_float2(
		(idx+du)/ARRAY_SIZE,
		(ARRAY_GET(idx+1)-ARRAY_GET(idx))*ARRAY_SIZE // todo: check this
		);
}


// return (u,v,pdf)
static __device__ __inline__ float3 sampleEnvmap( float3& vec3, const float u1, const float u2)
{
	//float u1=rnd(seed),u2=rnd(seed);
	uint iv;
	float2 v=sample_env_marginal(u2,&iv);
	float2 u=sample_env_conditional(u1,iv);
	float3 dir = spherical2cartesian( v.x*M_PIf, u.x*2.f*M_PIf);
	vec3 = make_float3(u.x,v.x,u.y);
	return dir;
}

static __device__ __inline__ float envmapMapPdf(float uu,float vv)
{
	uint u = clamp((uint)(uu * envmapWidth), 0u,	envmapWidth-1u);
	uint v = clamp((uint)(vv * envmapHeight), 0u,	envmapHeight-1u);

	// return product of { func_value[i]/func_int }
	// which is equivalent to (cdf[i+1]-cdf[i])*count
	// because cdf[i+1]=cdf[i]+func_value[i]/count/func_int
	return (cdfMarginal[v+1]-cdfMarginal[v])*envmapHeight
		*(ARRAY_GET(u+1)-ARRAY_GET(u))*envmapWidth;
}

#endif
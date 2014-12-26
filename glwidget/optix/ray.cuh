#ifndef _RAY_CUH_
#define _RAY_CUH_

#include <optix.h>
#include <optixu/optixu_math_namespace.h>
//#include "helpers.h"
#include "Sampler.h"
#include "random.h"

using namespace optix;

rtTextureSampler<float4, 2> envmap;
rtDeclareVariable(float,        isSingle, , );
struct PerRayData_pathtrace
{
	float3 result;
	float3 radiance;
	float3 attenuation;
	float3 origin;
	float3 direction;
	unsigned int seed;
	int depth;
	int countEmitted;
	int done;
	int inside;
};

struct PerRayData_pathtrace_shadow
{
	unsigned int seed;
	float3 origin;
	float3 direction;
	float3 attenuation;
};

// Scene wide
rtDeclareVariable(float,         scene_epsilon, , );
rtDeclareVariable(rtObject,      top_object, , );
rtBuffer<ParallelogramLight>     lights;
rtDeclareVariable(float,         light_em, , );

rtDeclareVariable(unsigned int,  pathtrace_ray_type, , );
rtDeclareVariable(unsigned int,  pathtrace_shadow_ray_type, , );
rtDeclareVariable(unsigned int,  rr_begin_depth, , );
rtDeclareVariable(unsigned int,  max_depth, , );

rtDeclareVariable(float3, geometric_normal, attribute geometric_normal, ); 
rtDeclareVariable(float3, shading_normal,   attribute shading_normal, ); 

rtDeclareVariable(PerRayData_pathtrace, current_prd, rtPayload, );

rtDeclareVariable(optix::Ray, ray,          rtCurrentRay, );
rtDeclareVariable(float,      t_hit,        rtIntersectionDistance, );

// For miss program
rtDeclareVariable(float3,       bg_color, , );
rtDeclareVariable(float3,        bad_color, , );
// For shadow program
rtDeclareVariable(PerRayData_pathtrace_shadow, current_prd_shadow, rtPayload, );

#endif
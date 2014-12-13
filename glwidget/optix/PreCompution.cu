#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include "volume.cuh"
rtDeclareVariable(uint3, gridIndex, rtLaunchIndex, );
rtDeclareVariable(int, numSampling, , );
rtBuffer<float4, 3>    gridBuffer;
static __device__ __inline__ float3 GetPosition( uint3 index)
{
	float3 p01 = P1-P0;
	return make_float3(p01.x/(float)(index_x-1)*(float)(index.x)+P0.x, p01.y/(float)(index_y-1)*(float)(index.y)+P0.y, p01.z/(float)(index_z-1)*(float)(index.z)+P0.z );
}

//-----------------------------------------------------------------------------
//
//  Main program
//
//-----------------------------------------------------------------------------
RT_PROGRAM void PreCompution()
{
	unsigned int seed = tea<16>(gridIndex.z*index_x*index_y + gridIndex.y*index_x + gridIndex.z, 0);
	float3 ray_origin = GetPosition( gridIndex);
	float3 ray_direction;
	Ray ray;
	PerRayData_pathtrace prd;
	prd.result = make_float3(0.f);
	prd.seed = seed;
	for(int i=0; i<numSampling; ++i)
	{
		ray_direction = uniformSphere( rnd(prd.seed), rnd(prd.seed), make_float3(1.f));
		ray = make_Ray(ray_origin, ray_direction, pathtrace_ray_type, scene_epsilon, RT_DEFAULT_MAX);
		prd.depth = 0;
		Ray ray = make_Ray(ray_origin, ray_direction, pathtrace_ray_type, scene_epsilon, RT_DEFAULT_MAX);
		rtTrace(top_object, ray, prd);
		prd.result += prd.radiance * prd.attenuation;
	}
	gridBuffer[ gridIndex] =  make_float4(prd.result / numSampling, 0.f);
}

//-----------------------------------------------------------------------------
//
//  Exception program
//
//-----------------------------------------------------------------------------

RT_PROGRAM void exception()
{
	gridBuffer[ gridIndex] =  make_float4( bad_color, 0.f);
}


//-----------------------------------------------------------------------------
//
//  Miss program
//
//-----------------------------------------------------------------------------

RT_PROGRAM void miss()
{
	current_prd.radiance = bg_color;
	current_prd.done = true;
}


rtTextureSampler<float4, 2> envmap;
RT_PROGRAM void envmap_miss()
{
	float theta = atan2f( ray.direction.x, ray.direction.z );
	float phi   = M_PIf * 0.5f -  acosf( ray.direction.y );
	float u     = (theta + M_PIf) * (0.5f * M_1_PIf);
	float v     = 0.5f * ( 1.0f + sin(phi) );
	//current_prd.radiance = bg_color;
	current_prd.radiance = make_float3( tex2D(envmap, u, v) )*1.f;
	
	current_prd.done = true;
	//current_prd.attenuation *= 0.1f;
}

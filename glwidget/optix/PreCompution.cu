#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include "volume.cuh"
rtDeclareVariable(uint, gridIndex, rtLaunchIndex, );
rtDeclareVariable(int, numSampling, , );
//rtBuffer<float4, 3>    gridBuffer;
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
	int maxDepth = 2;
	float3 p = GetPosition( i2xyz(gridIndex));
	float3 ray_direction, ray_origin;
	Ray ray;
	//prd.seed = seed;
	float3 result = make_float3(0.0f);
	unsigned int seed = tea<16>(gridIndex*gridIndex, 1);
	for(int i=0; i<numSampling; ++i)
	{
		PerRayData_pathtrace prd;
		prd.attenuation = make_float3(1.f);
		prd.result = make_float3(0.f);
		prd.radiance = make_float3(0.f);
		prd.countEmitted = true;
		prd.done = false;
		prd.inside = true;
		prd.depth = 0;
		prd.seed = seed;
		ray_direction = uniformSphere( rnd(prd.seed), rnd(prd.seed), make_float3(1.f, 0.f, 0.f));
		ray_origin = p;
		while(1)
		{
			ray = make_Ray(ray_origin, ray_direction, pathtrace_ray_type, scene_epsilon, RT_DEFAULT_MAX);
			rtTrace(top_object, ray, prd);
			if(prd.done ||(prd.depth >= maxDepth))
			{
				prd.result += prd.radiance * prd.attenuation;
				break;
			}
			prd.depth++;
			prd.result += prd.radiance * prd.attenuation;
			ray_origin = prd.origin;
			ray_direction = prd.direction;
		}
		result += prd.result;
		seed = prd.seed;
	}
	gridBuffer[ gridIndex] =  result/(float)numSampling;
}

//-----------------------------------------------------------------------------
//
//  Exception program
//
//-----------------------------------------------------------------------------

RT_PROGRAM void exception()
{
	gridBuffer[ gridIndex] =  make_float3(1.f,0.f,0.f);//bad_color;
}


//-----------------------------------------------------------------------------
//
//  Miss program
//
//-----------------------------------------------------------------------------




RT_PROGRAM void envmap_miss()
{
	float theta = atan2f( ray.direction.x, ray.direction.z );
	float phi   = M_PIf * 0.5f -  acosf( ray.direction.y );
	float u     = (theta + M_PIf) * (0.5f * M_1_PIf);
	float v     = 0.5f * ( 1.0f + sin(phi) );
	current_prd.radiance = bg_color*100.f;
	//current_prd.radiance = make_float3( tex2D(envmap, u, v) )*1.f;
	
	current_prd.done = true;
	//current_prd.attenuation *= 0.1f;
}

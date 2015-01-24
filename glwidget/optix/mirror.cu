#include "ray.cuh"
// -----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//
// Mirror program
//
//-----------------------------------------------------------------------------



RT_PROGRAM void mirror_fresnel()
{
  float3 world_shading_normal   = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, shading_normal ) );
  float3 world_geometric_normal = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, geometric_normal ) );

  float3 ffnormal = faceforward( world_shading_normal, -ray.direction, world_geometric_normal );

  float3 hitpoint = ray.origin + t_hit * ray.direction;
  current_prd.origin = hitpoint;
  current_prd.countEmitted = false;



  float rand_reflect = rnd(current_prd.seed);
  // check for external or internal reflection

  current_prd.direction = reflect(ray.direction, ffnormal);	

  current_prd.radiance = make_float3(0.0f,0.f,0.f);
}


RT_PROGRAM void shadow()
{
	current_prd_shadow.attenuation = make_float3(0.f);
	rtTerminateRay();
}
#include "ray.cuh"
// -----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//
// Glass program
//
//-----------------------------------------------------------------------------

//
// Dielectric surface shader
//
rtDeclareVariable(float3,       cutoff_color, , );
rtDeclareVariable(float,        fresnel_exponent, , );
rtDeclareVariable(float,        fresnel_minimum, , );
rtDeclareVariable(float,        fresnel_maximum, , );
rtDeclareVariable(float,        refraction_index, , );
rtDeclareVariable(int,          refraction_maxdepth, , );
rtDeclareVariable(int,          reflection_maxdepth, , );
rtDeclareVariable(float3,       refraction_color, , );
rtDeclareVariable(float3,       reflection_color, , );
rtDeclareVariable(float3,       extinction_constant, , );

rtDeclareVariable(float,    importance_cutoff, , );   
rtDeclareVariable(float3,        glass_color, , );
rtDeclareVariable(float,         index_of_refraction, , );


RT_PROGRAM void glass_fresnel()
{
  float3 world_shading_normal   = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, shading_normal ) );
  float3 world_geometric_normal = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, geometric_normal ) );

  float3 ffnormal = faceforward( world_shading_normal, -ray.direction, world_geometric_normal );

  float3 hitpoint = ray.origin + t_hit * ray.direction;
  current_prd.origin = hitpoint;
  current_prd.countEmitted = false;

  float iof;
  if (current_prd.inside) {
    // Shoot outgoing ray
    iof = 1.0f/index_of_refraction;
  } else {
    iof = index_of_refraction;
  }
  float3 t;
  refract(t, ray.direction, ffnormal, iof);
  float rand_reflect = rnd(current_prd.seed);
  // check for external or internal reflection
  float cos_theta = dot(ray.direction, ffnormal);
  if (cos_theta < 0.0f)
    cos_theta = -cos_theta;
  else
    cos_theta = dot(t, ffnormal);

  float reflection = fresnel_schlick(cos_theta, fresnel_exponent, fresnel_minimum, fresnel_maximum);

  if (rand_reflect<=reflection)
  {
	 //reflect
     current_prd.direction = reflect(ray.direction, ffnormal);	
  }
  else
  {
	  //refract
	  refract(current_prd.direction, ray.direction, ffnormal, iof);
	  
	  if (current_prd.inside) {
		// Compute Beer's law
		current_prd.attenuation = current_prd.attenuation * powf(glass_color, t_hit);
	  }
	  current_prd.inside = !current_prd.inside;
  }

  current_prd.radiance = make_float3(0.0f,0.f,0.f);
}


rtDeclareVariable(float3, shadow_attenuation, , );

RT_PROGRAM void glass_any_hit_shadow()
{
  float3 world_normal = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, shading_normal ) );
  float nDi = fabs(dot(world_normal, ray.direction));

  current_prd_shadow.attenuation *= 1-fresnel_schlick(nDi, 5, 1-shadow_attenuation, make_float3(1));

  rtIgnoreIntersection();
}
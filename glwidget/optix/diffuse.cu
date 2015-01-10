//#include "volume.cuh"
#include "ray.cuh"

rtDeclareVariable(float3,        diffuse_color, , );

RT_PROGRAM void diffuse()
{
	current_prd.insertedDiffuse = true;

	float3 world_shading_normal   = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, shading_normal ) );
	float3 world_geometric_normal = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, geometric_normal ) );

	float3 ffnormal = faceforward( world_shading_normal, -ray.direction, world_geometric_normal );

	float3 hitpoint = ray.origin + t_hit * ray.direction;
	current_prd.origin = hitpoint;

	float z1=rnd(current_prd.seed);
	float z2=rnd(current_prd.seed);
	float3 p;
	cosine_sample_hemisphere(z1, z2, p);
	float3 v1, v2;
	createONB(ffnormal, v1, v2);
	current_prd.direction = v1 * p.x + v2 * p.y + ffnormal * p.z;
	//float3 normal_color = (normalize(world_shading_normal)*0.5f + 0.5f)*0.9;
	current_prd.attenuation = current_prd.attenuation * diffuse_color*0.8f; // use the diffuse_color as the diffuse response

	//////////////////////////////////////////////////////////////////////////
	// Fast preview moduel
	//current_prd.attenuation *= dot(current_prd.direction, ffnormal);
	if (current_prd.depth >5)
		current_prd.done = 1;
	
	current_prd.countEmitted = false;

	// Compute direct light...
	// Or shoot one...
	unsigned int num_lights = lights.size();
	float3 result = make_float3(0.0f);

	for(int i = 0; i < num_lights; ++i) 
	{
		ParallelogramLight light = lights[i];
		float z1 = rnd(current_prd.seed);
		float z2 = rnd(current_prd.seed);
		float3 light_pos = light.corner + light.v1 * z1 + light.v2 * z2;

		float Ldist = length(light_pos - hitpoint);
		float3 L = normalize(light_pos - hitpoint);
		float nDl = dot( ffnormal, L );
		float LnDl = dot( light.normal, L );
		float A = length(cross(light.v1, light.v2));

		// cast shadow ray
		if ( nDl > 0.0f && LnDl > 0.0f ) 
		{
			PerRayData_pathtrace_shadow shadow_prd;
			shadow_prd.attenuation = make_float3(1.f);
			Ray shadow_ray = make_Ray( current_prd.origin, L, pathtrace_shadow_ray_type, scene_epsilon, Ldist );
			shadow_prd.origin = current_prd.origin;
			shadow_prd.direction = L;
			shadow_prd.seed = current_prd.seed;
			rtTrace(top_object, shadow_ray, shadow_prd);

			float3 light_attenuation = shadow_prd.attenuation;
			if(fmaxf(light_attenuation) > 0.0f)
			{
				float weight=nDl * LnDl * A / (M_PIf*Ldist*Ldist);
				result += light.emission * weight*light_attenuation;
			}
		}
	}

	current_prd.radiance = result;
}


RT_PROGRAM void shadow()
{
	current_prd_shadow.attenuation = make_float3(0.f);
	rtTerminateRay();
}
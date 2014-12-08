#include "volume.cuh"

rtDeclareVariable(float3,        diffuse_color, , );

RT_PROGRAM void diffuse()
{
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
	float3 normal_color = (normalize(world_shading_normal)*0.5f + 0.5f)*0.9;
	current_prd.attenuation = current_prd.attenuation * diffuse_color*0.8f; // use the diffuse_color as the diffuse response
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
			Ray shadow_ray = make_Ray( hitpoint, L, pathtrace_shadow_ray_type, scene_epsilon, Ldist );
			//rtTrace(top_object, shadow_ray, shadow_prd);

			//dk
			float3 p=shadow_ray.origin;
			float3 dir=shadow_ray.direction;
			float l=0.f;
			float step=30.f;
			float rand0=rnd(current_prd.seed);
			l+=rand0*step;
			float tau = 0.f;
			tau+=get_density(p)*rand0;
			while (l<=300.f)
			{
				tau+=get_density(p+dir*l);
				l+=step;
			}
			tau*=step;
			shadow_prd.attenuation *=  exp(-sigma_t*tau);//make_float3(0.f);

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
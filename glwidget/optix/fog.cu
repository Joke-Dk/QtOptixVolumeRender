//#include "ray.cuh"
//#include "math.cuh"
#include "volume.cuh"
// -----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
//
// Fog program
//
//-----------------------------------------------------------------------------

rtDeclareVariable(float3,        glass_color, , );
rtDeclareVariable(float,         index_of_refraction, , );
rtDeclareVariable(float,        fresnel_exponent, , );
rtDeclareVariable(float,        fresnel_minimum, , );
rtDeclareVariable(float,        fresnel_maximum, , );
rtDeclareVariable(float,        isRayMarching, , );
rtDeclareVariable(float,        isPreCompution, , );



static __device__ __inline__ float woodcockTracking( const Ray& current_ray, float maxLength, float maxExtinction)
{
	float3 p = current_ray.origin;
	float3 dir = current_ray.direction;
	if (maxExtinction==0.f)
		return 100000000.f;

	float r = rnd(current_prd.seed);
	float d=-log(1.f-r)/maxExtinction;
	//*for inhomogeneous media
	int itimes = 0;
	float sigma;
	while (d < maxLength)
	{
		itimes++;
		if (itimes>300) break;
		sigma=maxExtinction*get_density(p+d*dir);
		r = rnd(current_prd.seed);
		if (r<sigma/maxExtinction)
			break;
		r = rnd(current_prd.seed);
		d-=log(1.f-r)/maxExtinction;
	}
	return d;
}

static __device__ __inline__ float woodcockTracking_shadow( const Ray& current_ray, float maxLength, float maxExtinction)
{
	float3 p = current_ray.origin;
	float3 dir = current_ray.direction;
	if (maxExtinction==0.f)
		return 100000000.f;

	float r = rnd(current_prd_shadow.seed);
	float d=-log(1.f-r)/maxExtinction;
	//*for inhomogeneous media
	int itimes = 0;
	float sigma;
	while (d < maxLength)
	{
		itimes++;
		if (itimes>300) break;
		sigma=maxExtinction*get_density(p+d*dir);
		r = rnd(current_prd_shadow.seed);
		if (r<sigma/maxExtinction)
			break;
		r = rnd(current_prd_shadow.seed);
		d-=log(1.f-r)/maxExtinction;
	}
	return d;
}


RT_PROGRAM void fog_shadow()
{
	float maxLength = 20.f;//ray.tmax;
	float d = woodcockTracking_shadow( ray, maxLength, sigma_t);
	if(d< maxLength-scene_epsilon)
	{
		current_prd_shadow.attenuation = make_float3(0.f);
	}
	else
	{
		current_prd_shadow.attenuation = make_float3(1.f);
	}
	//current_prd_shadow.attenuation = make_float3(0.f, 0.f, 1.f);
	rtTerminateRay();
}



RT_PROGRAM void fog__closest_hit_radiance()
{
	float3 result = make_float3(0.0f);
	float3 world_shading_normal   = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, shading_normal ) );
	float3 world_geometric_normal = normalize( rtTransformNormal( RT_OBJECT_TO_WORLD, geometric_normal ) );

	float3 ffnormal = faceforward( world_shading_normal, -ray.direction, world_geometric_normal );

	float3 hitpoint = ray.origin + t_hit * ray.direction;
	//current_prd.origin = hitpoint;
	current_prd.countEmitted = false;

	float iof;
	if (current_prd.inside) {
		// Shoot outgoing ray
		iof = 1.0f/index_of_refraction;
	} 
	else 
	{
		iof = index_of_refraction;
	}

	if (current_prd.inside) 
	{
		float d = woodcockTracking( ray, t_hit, sigma_t);//1000.f;//woodcockTracking(0.1f, r1);
		if (d>=t_hit)
		{
			current_prd.origin = hitpoint;
			refract(current_prd.direction, ray.direction, ffnormal, iof);
			current_prd.inside = !current_prd.inside;
		}
		else
		{
			if(isPreCompution<0.5f)
			{
				current_prd.origin = ray.origin+d*ray.direction;
			}
			else
			{
				current_prd.origin = ray.origin;
			}

			//////////////////////////////////////////////////////////////////
			//rtTerminateRay();
			if( isRayMarching>0.5f)
			{
				current_prd.done = true;
				current_prd.radiance = interpolation( current_prd.origin);
				current_prd.attenuation = make_float3(1.f);
				return;
			}
			float z1=rnd(current_prd.seed);
			float z2=rnd(current_prd.seed);
			//float3 p;
			//cosine_sample_hemisphere(z1, z2, p);
			//float3 v1, v2;
			//createONB(ray.direction, v1, v2);
			//current_prd.direction = v1 * p.x + v2 * p.y + ray.direction * p.z;
			current_prd.direction = SampleHG( z1, z2, ray.direction);
			current_prd.attenuation*=alpha_value;

			// Compute direct light...
			// Or shoot one...
			unsigned int num_lights = lights.size();

			for(int i = 0; i < num_lights; ++i) 
			{
				if(isPreCompution<0.5f&&isSingle>0.5f&&current_prd.depth>=2)
					break;
				if(isPreCompution>0.5f&&current_prd.depth>=1)
					break;
				ParallelogramLight light = lights[i];
				float z1 = rnd(current_prd.seed);
				float z2 = rnd(current_prd.seed);
				float3 light_pos = light.corner + light.v1 * z1 + light.v2 * z2;

				float Ldist = length(light_pos - current_prd.origin);
				float3 L = normalize(light_pos - current_prd.origin);
				float nDl = 1.f;//dot( ray.direction, L );
				//if(isPreCompution>0.5f) nDl = 1.f;
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
			if(isPreCompution>0.5f)
			{
				current_prd.origin = ray.origin+d*ray.direction;
			}
			//if(isPreCompution<0.5f&&isSingle>0.5f&&current_prd.depth>=3)
			//{
			//	current_prd.done = true;
			//}
			
		}
	}
	else
	{
		current_prd.origin = hitpoint;
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

		if (0)//(rand_reflect<reflection)
		{
			//reflect
			current_prd.direction = reflect(ray.direction, ffnormal);	
		}
		else
		{
			//refract
			refract(current_prd.direction, ray.direction, ffnormal, iof);
			current_prd.inside = !current_prd.inside;

		}
	}

	current_prd.radiance = result;

}



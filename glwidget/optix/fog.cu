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
rtDeclareVariable(int,        boundMaterial, , );
rtDeclareVariable(int,        MCWoodcock, , );//1-woodcocking 0-raymarching

//static __device__ __inline__ float woodcockTracking2( const Ray& current_ray, float maxLength, float maxExtinction)
//{
//	float3 p = current_ray.origin;
//	float3 dir = current_ray.direction;
//	if (maxExtinction==0.f)
//		return 100000000.f;
//	float r = rnd(current_prd.seed);
//	float d=-log(1.f-r)/maxExtinction;
//	//if d>maxLength_o: return maxLength+10.f
//	v=safeNormalize(dir)
//	l=0.f
//	span=0.3f
//	l+=rand0*span
//	len = 0.f
//	while len*span<=d && l<=maxLength
//		len+=get_density( p+v*l)
//		l+=span
//	len2 = 0.f
//	span2=0.015f
//	if len*span>d
//		l-=span
//		len-=get_density( p+v*l)
//		tt=0
//		while len*span+len2*span2<=d && tt<20
//			tt++
//			len2+=get_density( p+v*l)
//			l+=span2
//	return l
//}

static __device__ __inline__ float rayMarching( const Ray& current_ray, float maxLength, float maxExtinction)
{
	float3 p = current_ray.origin;
	float3 v = current_ray.direction;
	if (maxExtinction==0.f)
		return 100000000.f;
	float r1 = rnd(current_prd.seed);
	float d=-log(1.f-r1)/maxExtinction;

	float rand0 = rnd(current_prd.seed);
	float l=0.f;
	float span=0.3f;
	l+=rand0*span;
	float len = 0.f;
	len+=get_density( p)*rand0;
	while (len*span<=d && l<=maxLength)
	{
		len+=get_density( p+v*l);
		l+=span;
	}
	float len2 = 0.f;
	float span2=span/20.f;
	if (len*span>d)
	{
		l-=span;
		len-=get_density( p+v*l);
		int tt=0;
		while (len*span+len2*span2<=d && tt<20)
		{
			tt++;
			len2+=get_density( p+v*l);
			l+=span2;
		}
	}
	return l;
}

static __device__ __inline__ float getOpticDepth( const Ray& current_ray, float maxLength, float maxExtinction)
{
	//To get optic lenghth
	float3 p = current_ray.origin;
	float3 v = current_ray.direction;
	if (maxExtinction==0.f)
		return 100000000.f;
	//float r1 = rnd(current_prd_shadow.seed);
	//float d=-log(1.f-r1)/maxExtinction;

	float rand0 = rnd(current_prd_shadow.seed);
	float l=0.f;
	float span=0.5f;
	l+=rand0*span;
	float len = 0.f;
	len+=get_density( p)*rand0;
	while (l<maxLength)
	{
		len+=get_density( p+v*l);
		l+=span;
	}
	return len*span;
}


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
	switch(boundMaterial)
	{
	case 1://mirror shadow: solid
	case 3://diffuse shadow: solid
		current_prd_shadow.attenuation *= 0.f;
		//rtTerminateRay();
		break;
	case 2://glass in fog shadow: half solid
		current_prd_shadow.attenuation *= 0.6f;
		break;
	default:
	case 0://fog shadow
		float maxLength = ray.tmax;
		if(MCWoodcock)//woodcock-tracking shadow
		{
			float d = woodcockTracking_shadow( ray, maxLength, sigma_t);
			if(d< maxLength-scene_epsilon)
			{
				current_prd_shadow.attenuation *= 0.f;rtTerminateRay();
			}
			else
			{
				rtTerminateRay();
			}
		}
		else//ray-marching shadow
		{
			current_prd_shadow.attenuation *= exp(-sigma_t*getOpticDepth( ray, maxLength, sigma_t));
		}
		break;
	}
	rtIgnoreIntersection();
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
		float d;
		if(current_prd.insertedDiffuse)
			d = RT_DEFAULT_MAX;
		else if(MCWoodcock)
			d = woodcockTracking( ray, t_hit, sigma_t);//1000.f;//woodcockTracking(0.1f, r1);
		else
			d = rayMarching( ray, t_hit, sigma_t);
		if (d>=t_hit)
		{
			current_prd.origin = hitpoint;
			switch(boundMaterial)
			{
			case 0:
			default://fog
				refract(current_prd.direction, ray.direction, ffnormal, iof);
				current_prd.inside = !current_prd.inside;
				break;
			case 1://mirror
				current_prd.direction = reflect(ray.direction, ffnormal);	
				break;
			case 2://glass
				if (current_prd.inside2) 
				{
					// Shoot outgoing ray
					iof = 1.0f/index_of_refraction;
				} 
				else 
				{
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
				if (rand_reflect<reflection)
				{
					//reflect
					current_prd.direction = reflect(ray.direction, ffnormal);	
				}
				else
				{
					//refract
					refract(current_prd.direction, ray.direction, ffnormal, iof);
					current_prd.inside2 = !current_prd.inside2;
				}
				break;
			case 3://diffuse //To do!
				current_prd.insertedDiffuse = true;
				float u1 = rnd(current_prd.seed);
				float u2 = rnd(current_prd.seed);
				current_prd.direction = cosineHemisphere( u1, u2, ffnormal);
				current_prd.attenuation = current_prd.attenuation *0.95f; // use the diffuse_color as the diffuse response
				if (current_prd.depth >5)
					current_prd.done = 1;
				// Compute direct light...
				// Or shoot one...
				unsigned int num_lights = lights.size();

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
				break;
			}

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
				float nDl = 1.f;//dot( current_prd.direction, L );
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



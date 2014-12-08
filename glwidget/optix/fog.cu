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

RT_PROGRAM void fog_shadow()
{

	float d=200.f;
	float3 p=current_prd_shadow.origin;
	float3 dir=current_prd_shadow.direction;
	float l=0.f;
	float step=30.f;
	float rand0=0.f;
	l+=rand0*step;
	float tau = 0.f;
	tau+=get_density(p)*rand0;
	while (l<=d)
	{
		tau+=get_density(p+dir*l);
		l+=step;
	}
	tau*=step;
	//return tau//*$sigma_t.x 
	current_prd_shadow.attenuation *= make_float3(0.f);//exp(sigma_t*tau);//make_float3(0.f);
	//rtTerminateRay();
}
	
static __device__ __inline__ float woodcockTracking2( PerRayData_pathtrace& current_ray, float maxLength, float maxExtinction)
{
  if (maxExtinction==0.f)
		return 100000000.f;
	float r1 = rnd(current_prd.seed);
	float d=-log(1.f-r1)/maxExtinction;

	float rand0 = rnd(current_prd.seed);
	float3 p=current_prd.origin;
	float3 v=normalize(current_prd.direction);
	float l=0.f;
	float span=3.f;
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
		float r1=rnd(current_prd.seed);
		float d = woodcockTracking2(current_prd, t_hit, sigma_t);//1000.f;//woodcockTracking(0.1f, r1);
		if (d>t_hit)
		{
			current_prd.origin = hitpoint;
			refract(current_prd.direction, ray.direction, ffnormal, iof);
			current_prd.inside = !current_prd.inside;
		}
		else
		{
			current_prd.origin = ray.origin+d*ray.direction;
			float z1=rnd(current_prd.seed);
			float z2=rnd(current_prd.seed);
			float3 p;
			cosine_sample_hemisphere(z1, z2, p);
			float3 v1, v2;
			createONB(ray.direction, v1, v2);
			current_prd.direction = v1 * p.x + v2 * p.y + ray.direction * p.z;
			current_prd.attenuation*=alpha_value;
			
			// Compute direct light...
			// Or shoot one...
			unsigned int num_lights = lights.size();
			
			for(int i = 0; i < num_lights; ++i) 
			{
				ParallelogramLight light = lights[i];
				float z1 = rnd(current_prd.seed);
				float z2 = rnd(current_prd.seed);
				float3 light_pos = light.corner + light.v1 * z1 + light.v2 * z2;
				
				float Ldist = length(light_pos - current_prd.origin);
				float3 L = normalize(light_pos - current_prd.origin);
				float nDl = dot( current_prd.direction, L );
				float LnDl = dot( light.normal, L );
				float A = length(cross(light.v1, light.v2));
				
				// cast shadow ray
				if ( nDl > 0.0f && LnDl > 0.0f ) 
				{
					PerRayData_pathtrace_shadow shadow_prd;
					shadow_prd.attenuation = make_float3(1.f);
					//Ray shadow_ray = make_Ray( hitpoint, L, pathtrace_shadow_ray_type, scene_epsilon, Ldist );
					shadow_prd.origin = current_prd.origin;
					shadow_prd.direction = L;
					//rtTrace(top_object, shadow_ray, shadow_prd);
					
					float3 p=shadow_prd.origin;
					float3 dir=shadow_prd.direction;
					float l=0.f;
					float step=3.f;
					float rand0=rnd(current_prd.seed);
					l+=rand0*step;
					float tau = 0.f;
					tau+=get_density(p)*rand0;
					while (l<=500.f)
					{
						tau+=get_density(p+dir*l);
						l+=step;
					}
					tau*=step;
					shadow_prd.attenuation = make_float3(1.f)* exp(-sigma_t*tau);//make_float3(0.f);
					
					float3 light_attenuation = shadow_prd.attenuation;
					if(fmaxf(light_attenuation) > 0.0f)
					{
						float weight=nDl * LnDl * A / (M_PIf*Ldist*Ldist);
						result += light.emission * weight*light_attenuation;
					}
				}
			}
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
			//current_prd.expand_rad = 3.f;
			//current_prd.attenuation*=20.f;//
			current_prd.inside = !current_prd.inside;
			
		}
	}
	
	
	
	//current_prd.attenuation*=5.f;
	current_prd.radiance = result;
	//current_prd.radiance = make_float3(0.f,0.f,0.f);
}
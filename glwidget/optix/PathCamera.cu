#include "ray.cuh"
#include "envmap.cuh"
rtDeclareVariable(uint2,      launch_index, rtLaunchIndex, );

rtDeclareVariable(float3,        eye, , );
rtDeclareVariable(float3,        U, , );
rtDeclareVariable(float3,        V, , );
rtDeclareVariable(float3,        W, , );
rtDeclareVariable(unsigned int,  frame_number, , );
rtDeclareVariable(unsigned int,  sqrt_num_samples, , );
rtBuffer<float4, 2>              output_buffer;
rtDeclareVariable(float,  hasBackground, , );
rtDeclareVariable(float,  radianceMultipler, , );
//rtDeclareVariable(float,  hasHDR, , );


//-----------------------------------------------------------------------------
//
//  Camera program -- main ray tracing loop
//
//-----------------------------------------------------------------------------

RT_PROGRAM void pathtrace_camera()
{
	size_t2 screen = output_buffer.size();

	float2 inv_screen = 1.0f/make_float2(screen) * 2.f;
	float2 pixel = (make_float2(launch_index)) * inv_screen - 1.f;

	float2 jitter_scale = inv_screen / sqrt_num_samples;
	unsigned int samples_per_pixel = sqrt_num_samples*sqrt_num_samples;
	float3 result = make_float3(0.0f);

	unsigned int seed = tea<16>(screen.x*launch_index.y+launch_index.x, frame_number);
	do {
		unsigned int x = samples_per_pixel%sqrt_num_samples;
		unsigned int y = samples_per_pixel/sqrt_num_samples;
		float2 jitter = make_float2(x-rnd(seed), y-rnd(seed));
		float2 d = pixel + jitter*jitter_scale;
		float3 ray_origin = eye;
		float3 ray_direction = normalize(d.x*U + d.y*V + W);

		PerRayData_pathtrace prd;
		prd.result = make_float3(0.f);
		prd.attenuation = make_float3(1.f);
		prd.countEmitted = true;
		prd.done = false;
		prd.inside = false;
		prd.seed = seed;
		prd.depth = 0;

		for(;;) {
			Ray ray = make_Ray(ray_origin, ray_direction, pathtrace_ray_type, scene_epsilon, RT_DEFAULT_MAX);
			rtTrace(top_object, ray, prd);
			if(prd.done ||(prd.depth >= max_depth)) {
				prd.result += prd.radiance * prd.attenuation;
				break;
			}

			// RR
			if(prd.depth >= rr_begin_depth){
				float pcont = fmaxf(prd.attenuation);
				if(rnd(prd.seed) >= pcont)
					break;
				prd.attenuation /= pcont;
			}
			prd.depth++;
			prd.result += prd.radiance * prd.attenuation;
			ray_origin = prd.origin;
			ray_direction = prd.direction;
		} // eye ray

		result += prd.result;
		seed = prd.seed;
	} while (--samples_per_pixel);

	float3 pixel_color = result/(sqrt_num_samples*sqrt_num_samples);
	/*
	if(launch_index.x==0&&launch_index.y==0)
	{
	rtPrintf("[%f,%f,%f]",pixel_color.x,pixel_color.y,pixel_color.z);
	}*/

	pixel_color *= radianceMultipler;
	if (frame_number > 1)
	{
		float a = 1.0f / (float)frame_number;
		float b = ((float)frame_number - 1.0f) * a;
		float3 old_color = make_float3(output_buffer[launch_index]);
		output_buffer[launch_index] = make_float4(a * pixel_color + b * old_color, 0.0f);
	}
	else
	{
		output_buffer[launch_index] = make_float4(pixel_color, 0.0f);
	}
}


//-----------------------------------------------------------------------------
//
//  Exception program
//
//-----------------------------------------------------------------------------

RT_PROGRAM void exception()
{
	output_buffer[launch_index] = make_float4(bad_color, 0.0f);
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


RT_PROGRAM void envmap_miss()
{
	float theta = atan2f( ray.direction.x, ray.direction.z );
	float phi   = M_PIf * 0.5f -  acosf( ray.direction.y );
	float u     = (theta + M_PIf) * (0.5f * M_1_PIf);
	float v     = 0.5f * ( 1.0f + sin(phi) );
	current_prd.radiance = bg_color;
	if(hasBackground>0.5f || current_prd.depth>2)
	{
		current_prd.radiance = envmapEvalL(ray.direction);//make_float3( tex2D(envmap, u, v) )*1.f;
	}
	current_prd.done = true;
	//current_prd.attenuation *= 0.1f;
}

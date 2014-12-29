#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include "volume.cuh"
#include "MultiCompution.cuh"
#include "envmap.cuh"
rtDeclareVariable(uint, gridIndex, rtLaunchIndex, );
rtDeclareVariable(int, numSampling, , );
rtDeclareVariable(unsigned int,  numCompution, , );
//rtBuffer<float4, 3>    gridBuffer;
static __device__ __inline__ float3 GetPosition( int3 index)
{
	float3 p01 = P1-P0;
	return make_float3(p01.x/(float)(index_x-1)*(float)(index.x)+P0.x, p01.y/(float)(index_y-1)*(float)(index.y)+P0.y, p01.z/(float)(index_z-1)*(float)(index.z)+P0.z );
}

RT_PROGRAM void MultiCompution()
{
	if (curIterator==1)
	{
		//gridBuffer[gridIndex] *= GetDensity(gridIndex)*sigma_t;
		gridFluence[ gridIndex] = make_float3(1.f)*ee*dx*J_mean;
		return;
	}
	int i = gridIndex;
	float3 update_fluence = make_float3(0.f);
	int3 xyzIndex = i2xyz( i);
	int tmpI = xyzIndex.x+xyzIndex.y+xyzIndex.z;
	if( tmpI%2 == curIterator%2) return;

	float3 sigmaP = GetSigmaT0( i);
	float3 Dp = GetDp( i);

	if ( GetDensity( i) == 0.f)
	{
		//gridFluence[ gridIndex] = gridBuffer[i];
		//return;
		int countTmp = 0;
		for ( int i0=max( xyzIndex.x-1, 0); i0<=min(xyzIndex.x+1, index_x-1); ++i0)
			for (int j0=max( xyzIndex.y-1, 0); j0<=min(xyzIndex.y+1, index_y-1); ++j0)
				for (int k0=max( xyzIndex.z-1, 0); k0<=min(xyzIndex.z+1, index_z-1);++k0)
					if (GetDensity(xyz2i(i0, j0, k0))!=0.f && !(i0==xyzIndex.x && j0==xyzIndex.y && k0==xyzIndex.z))
					{
						countTmp++;
						float3 Ds = make_float3(1.f)/3.f/GetSigmaT2(i);//*2.5f;//2.5f
						update_fluence += 2.f*Ds*gridFluence[xyz2i( i0, j0, k0)]/(dx*make_float3(1.f)+2.f*Ds);
					}
		if(countTmp!=0)
			update_fluence /= float(countTmp);
		else
			return;
	}
	else
	{
		float3 Ds10 = (safeGetDp(i, -1, 0, 0)+Dp)/2.f;
		float3 Ds11 = (safeGetDp(i,  1, 0, 0)+Dp)/2.f;
		
		float3 Ds20 = (safeGetDp(i, 0, -1, 0)+Dp)/2.f;
		float3 Ds21 = (safeGetDp(i, 0,  1, 0)+Dp)/2.f;
		
		float3 Ds30 = (safeGetDp(i, 0, 0, -1)+Dp)/2.f;
		float3 Ds31 = (safeGetDp(i, 0, 0,  1)+Dp)/2.f;
		
		float3 fluence10 = safeGetFlue(  i, -1, 0,  0);
		float3 fluence11 = safeGetFlue(  i, 1,  0,  0);
		float3 fluence20 = safeGetFlue(  i, 0, -1,  0);
		float3 fluence21 = safeGetFlue(  i, 0,  1,  0);
		float3 fluence30 = safeGetFlue(  i, 0,  0, -1);
		float3 fluence31 = safeGetFlue(  i, 0,  0,  1);

		float3 numerator = GetDensity(i)*sigma_t*dx*dx*gridBuffer[i]+(Ds10*fluence10 + Ds11*fluence11 + Ds20*fluence20+Ds21*fluence21+Ds30*fluence30+Ds31*fluence31);
		float3 denominator = (1.f-alpha_value)*sigmaP*dx*dx+(Ds10+Ds11+Ds20+Ds21+Ds30+Ds31);
		//error_p = (numerator - $fluence[i]*denominator)/$dx/$dx
		update_fluence = numerator/denominator;
	}
	gridFluence[i] = max(weight*update_fluence+(1.f-weight)* gridFluence[i], make_float3(0.f,0.f,0.f));

}

//-----------------------------------------------------------------------------
//
//  Main program
//
//-----------------------------------------------------------------------------
RT_PROGRAM void PreCompution()
{
	//if(0)//if(curIterator>0)
	//{
	//	MultiCompution();
	//	return;
	//}

	int maxDepth = 1;
	float3 p = GetPosition( i2xyz(gridIndex));
	float3 ray_direction, ray_origin;
	Ray ray;
	//prd.seed = seed;
	float3 result = make_float3(0.0f);
	unsigned int seed = tea<16>(gridIndex*gridIndex, numCompution);
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

		float3 pdf=make_float3(1.f);
		if(1)//importance sampling
		{
			ray_direction = sampleEnvmap( pdf, rnd(prd.seed), rnd(prd.seed));
		}
		else//uniform sampling
		{
			ray_direction = uniformSphere( rnd(prd.seed), rnd(prd.seed), make_float3(1.f, 0.f, 0.f));
		}

		// [Debug] Record and plot the sampled point in environment map
		uint2 iuv = uv2iuv(make_float2(pdf));
		atomicAdd(&pixelIsSampled[iuv],1);
		// [Debug] Print the pdf value : assert( pdf nearly> 1)
		//if(gridIndex==10001)
		//{
		//	printf("%lf\n",pdf.z);
		//}

		ray_origin = p;
		//float3 envmapColor = envmapEvalLandPdf(ray_direction);//pdf.z;
		while(1)
		{
			ray = make_Ray(ray_origin, ray_direction, pathtrace_ray_type, scene_epsilon, RT_DEFAULT_MAX);
			rtTrace(top_object, ray, prd);
			if(prd.done ||(prd.depth >= maxDepth))
			{
				if(prd.done) prd.result = prd.radiance * prd.attenuation/pdf.z;
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
	result /= (float)numSampling;
	if (numCompution > 1)
	{
		float a = 1.0f / (float)numCompution;
		float b = ((float)numCompution - 1.0f) * a;
		float3 old_color = gridBuffer[gridIndex];
		gridBuffer[gridIndex] = a * result + b * old_color;
	}
	else
	{
		gridBuffer[gridIndex] = result;
	}
	//gridBuffer[ gridIndex] =  result/(float)numSampling;
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




//RT_PROGRAM void envmap_miss()
//{
//	float theta = atan2f( ray.direction.x, ray.direction.z );
//	float phi   = M_PIf * 0.5f -  acosf( ray.direction.y );
//	float u     = (theta + M_PIf) * (0.5f * M_1_PIf);
//	float v     = 0.5f * ( 1.0f + sin(phi) );
//	//current_prd.radiance = bg_color*100.f;
//	current_prd.radiance = make_float3( tex2D(envmap, u, v) )*1.f;
//	
//	current_prd.done = true;
//	//current_prd.attenuation *= 0.1f;
//}

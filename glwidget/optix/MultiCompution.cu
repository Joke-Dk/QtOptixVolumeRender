#include <optix.h>
#include <optixu/optixu_math_namespace.h>
#include "volume.cuh"
rtDeclareVariable(uint, gridIndex, rtLaunchIndex, );
//rtDeclareVariable(int, maxIterator, , );
rtDeclareVariable(int, curIterator, , );
rtDeclareVariable(float, ee, , );
rtDeclareVariable(float, J_mean, , );
rtDeclareVariable(float, dx, , );
rtDeclareVariable(float, weight, , );
rtBuffer<float3, 1>    gridFluence;

static __device__ __inline__ float3  GetSigmaT0( int i)	
{
	return volume_density[i]*sigma_t*make_float3(1.f);
}
static __device__ __inline__ float3  GetSigmaT1( int i)	
{
	return max(volume_density[i]*sigma_t, 1.f/pow(10.f, 3)/20.f)*make_float3(1.f);
}

static __device__ __inline__ float3  max( float3 a, float3 b)
{
	float3 ret;
	ret.x = max(a.x, b.x);
	ret.y = max(a.y, b.z);
	ret.z = max(a.y, b.z);
	return ret;
}
//static __device__ __inline__ bool compare(int3 a, int3 b)
//{
//
//}

static __device__ __inline__ bool safe_index(int3& xyz,  int addx, int addy, int addz)
{
	xyz += make_int3(addx, addy, addz);
	int3 minIndex = make_int3(0);
	int3 maxIndex = make_int3(index_x-1, index_y-1, index_z-1);
	if (xyz.x>index_x-1 || xyz.x<0)
		return 0;	
	if (xyz.y>index_y-1 || xyz.y<0)
		return 0;	
	if (xyz.z>index_z-1 || xyz.z<0)
		return 0;	
	if (min(xyz, minIndex)!= minIndex) return 0;
	if (max(xyz, maxIndex)!= maxIndex) return 0;
	return 1;
}

static __device__ __inline__ int safe_index(int i,  int addx, int addy, int addz)
{
	int3 xyz = i2xyz(i);
	if (safe_index(xyz, addx, addy, addz))
		return xyz2i(xyz);
	return -1;
}

static __device__ __inline__ float3 safeGetFlue(  int i,  int addx, int addy, int addz)
{
	int indexI =  safe_index( i, addx, addy, addz);
	if (indexI>0)
		return gridFluence[ indexI];
	return make_float3(0.f);
}



static __device__ __inline__ float3 D_flu(int indexI)
{
	
	float3 fluence10 = safeGetFlue( indexI, -1, 0,  0);
	float3 fluence11 = safeGetFlue( indexI, 1,  0,  0);
	float3 fluence20 = safeGetFlue( indexI, 0, -1,  0);
	float3 fluence21 = safeGetFlue( indexI, 0,  1,  0);
	float3 fluence30 = safeGetFlue( indexI, 0,  0, -1);
	float3 fluence31 = safeGetFlue( indexI, 0,  0,  1);
	float3 ret;
	float3 a1 = fluence11 -fluence10;
	float3 a2 = fluence21 -fluence20;
	float3 a3 = fluence31 -fluence30;

	ret.x = length(make_float3(a1.x, a2.x, a3.x)/2.f/dx);
	ret.y = length(make_float3(a1.y, a2.y, a3.y)/2.f/dx);
	ret.z = length(make_float3(a1.z, a2.z, a3.z)/2.f/dx);
	return ret;
}

static __device__ __inline__ float3 GetRp( int i)
{
	float3 x1 = max(D_flu(i), ee*J_mean*make_float3(1.f));
	float3 sigmaP = GetSigmaT0( i);
	float3 x2 = max(sigmaP*gridFluence[i], ee*J_mean*make_float3(1.f));
	return x1/x2;
}

static __device__ __inline__ float3 GetFr( float3 rp)
{
	float3 ret;
	ret.x = 2.f/(3.f + sqrt( 9.f+4.f*pow( rp.x, 2.f)));
	ret.y = 2.f/(3.f + sqrt( 9.f+4.f*pow( rp.y, 2.f)));
	ret.z = 2.f/(3.f + sqrt( 9.f+4.f*pow( rp.z, 2.f)));
	return ret;
}

static __device__ __inline__ float3 GetDp( int i)
{
	float3 sigmaP = GetSigmaT1( i);
	float3 rp = GetRp( i);
	float3 fr = GetFr( rp);
	return fr/sigmaP;
}

static __device__ __inline__ float3 safeGetDp(  int i,  int addx, int addy, int addz)
{
	int indexI =  safe_index( i, addx, addy, addz);
	if (indexI>0)
		return GetDp( indexI);
	return make_float3(0.f);
}

//-----------------------------------------------------------------------------
//
//  Main program
//
//-----------------------------------------------------------------------------
RT_PROGRAM void MultiCompution()
{
	if (curIterator==1)
	{
		gridFluence[ gridIndex] = make_float3(1.f)*ee*dx;
		return;
	}
	int i = gridIndex;
	float3 update_fluence = make_float3(0.f);
	int3 xyzIndex = i2xyz( i);
	int tmpI = xyzIndex.x+xyzIndex.y+xyzIndex.z;
	if( tmpI%2 == curIterator%2) return;

	float3 sigmaP = GetSigmaT0( i);
	float3 Dp = GetDp( i);

	if ( 0)//volume_density[ i] == 0.f)
	{

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

		float3 numerator = dx*dx*gridBuffer[i]+(Ds10*fluence10 + Ds11*fluence11 + Ds20*fluence20+Ds21*fluence21+Ds30*fluence30+Ds31*fluence31);
		float3 denominator = (1.f-alpha_value)*sigmaP*dx*dx+(Ds10+Ds11+Ds20+Ds21+Ds30+Ds31);
		//error_p = (numerator - $fluence[i]*denominator)/$dx/$dx
		update_fluence = numerator/denominator;
	}
	gridFluence[i] = max(weight*update_fluence+(1.f-weight)* gridFluence[i], make_float3(0.f,0.f,0.f));
}

//-----------------------------------------------------------------------------
//
//  Exception program
//
//-----------------------------------------------------------------------------

RT_PROGRAM void exception()
{
	gridFluence[ gridIndex] =  make_float3(0.f,1.f,0.f);//bad_color;
}



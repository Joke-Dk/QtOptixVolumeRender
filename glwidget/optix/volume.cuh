#ifndef _VOLUME_CUH_
#define _VOLUME_CUH_

#include "ray.cuh"

rtBuffer<float> volume_density;
rtDeclareVariable(float3,        P0, , );
rtDeclareVariable(float3,        P1, , );
rtDeclareVariable(int,        index_x, , );
rtDeclareVariable(int,        index_y, , );
rtDeclareVariable(int,        index_z, , );
rtDeclareVariable(float,        sigma_t, , );
rtDeclareVariable(float,        alpha_value, , );
rtDeclareVariable(float,        g, , );
rtDeclareVariable(int,        isCurve, , );

rtDeclareVariable(float,        CloudCover, , );
rtDeclareVariable(float,        CloudSharpness, , );
rtDeclareVariable(int,        SequenceCurID, , );


//Precomputation Intensity
rtBuffer<float3, 1>    gridBuffer;
rtBuffer<float3, 1>    gridFluence;

rtDeclareVariable(float,        isFLDSingle, , );

static __device__ __inline__ int3 i2xyz(int i)
{
	int3 xyz;
	xyz.x = i%(index_x);
	xyz.y = i/(index_x)%(index_y);
	xyz.z = i/(index_x)/(index_y);
	return xyz;
}

static __device__ __inline__ int xyz2i(int x, int y, int z)
{
	return z*index_x*index_y+ y*index_x+x;
}

static __device__ __inline__ int xyz2i(int3 xyz)
{
	return xyz.z*index_x*index_y+ xyz.y*index_x+xyz.x;
}

static __device__  __inline__ float CloudExpCurve0(float v)
{
	float c=v-CloudCover;
	if (c<0.f){c=0.f;}
	float cloudDensity = 1.f-pow(CloudSharpness, c);
	return cloudDensity;
}

static __device__  __inline__ float CloudExpCurve(float v)
{
	if (v>=0.1f&&SequenceCurID>140)
		return 0.f;
	if (v>=0.5f)
		return 1.f;
	else
		return 1.0f-pow(0.05f, 0.5f*pow(2.f*v, 2));
}

static __device__ __inline__ float GetDensity(int index)
{
	if(isCurve)
		return  CloudExpCurve(volume_density[index]);//*.densityMultiplier//*CloudExpCurve(res)
	return volume_density[index];
}

static __device__ __inline__ float get_density(float3 p)
{
	//return 1.f
	//pbrt
	//p = make_float3(p2.x, p2.z, p2.y)
	if (((p.x-P0.x)*(p.x-P1.x)<=0.f) && ((p.y-P0.y)*(p.y-P1.y)<=0.f) &&((p.z-P0.z)*(p.z-P1.z)<=0.f))
	{
		float x0f = (p.x-P0.x)/(P1.x-P0.x)*(float)(index_x-1);
		float y0f = (p.y-P0.y)/(P1.y-P0.y)*(float)(index_y-1);
		float z0f = (p.z-P0.z)/(P1.z-P0.z)*(float)(index_z-1);
		int x0 = int(x0f); x0f-=(float)x0;
		int y0 = int(y0f); y0f-=(float)y0;
		int z0 = int(z0f); z0f-=(float)z0;
		if (x0>=index_x-2) {x0=index_x-2;}
		if (y0>=index_y-2) {y0=index_y-2;}
		if (z0>=index_z-2) {z0=index_z-2;}
		if (x0<=0) {x0=0;}
		if (y0<=0) {y0=0;}
		if (z0<=0) {z0=0;}
		int x1=x0+1;
		int y1=y0+1;
		int z1=z0+1;

		
		float d000=volume_density[xyz2i(x0,y0,z0)];
		float d100=volume_density[xyz2i(x0,y0,z1)];
		float d010=volume_density[xyz2i(x0,y1,z0)];
		float d110=volume_density[xyz2i(x0,y1,z1)];
		float d001=volume_density[xyz2i(x1,y0,z0)];
		float d101=volume_density[xyz2i(x1,y0,z1)];
		float d011=volume_density[xyz2i(x1,y1,z0)];
		float d111=volume_density[xyz2i(x1,y1,z1)];
	
		//p>p0
		float3 w1=make_float3(x0f, y0f, z0f);
		//w1 = p-(P1-P0)*make_float3(float(x0), float(y0), float(z0))-P0
		float3 w0 = make_float3(1.f,1.f,1.f)-w1;
		//debug.watch(p,x0,y0,z0, w1, w0)
		//return 1.f
		float res = (((d000 * w0.x + d001 * w1.x) * w0.y +  (d010 * w0.x + d011 * w1.x) * w1.y) * w0.z +  ((d100 * w0.x + d101 * w1.x) * w0.y + (d110 * w0.x + d111 * w1.x) * w1.y) * w1.z); 
		if(isCurve)
			return  CloudExpCurve(res);
		return res;
	}
		return 0.0000f;
}


static __device__ __inline__ float3 interpolation(float3 p)
{
	//return 1.f
	//pbrt
	//p = make_float3(p2.x, p2.z, p2.y)
	if (((p.x-P0.x)*(p.x-P1.x)<=0.f) && ((p.y-P0.y)*(p.y-P1.y)<=0.f) &&((p.z-P0.z)*(p.z-P1.z)<=0.f))
	{
		float x0f = (p.x-P0.x)/(P1.x-P0.x)*(float)(index_x-1);
		float y0f = (p.y-P0.y)/(P1.y-P0.y)*(float)(index_y-1);
		float z0f = (p.z-P0.z)/(P1.z-P0.z)*(float)(index_z-1);
		int x0 = int(x0f); x0f-=(float)x0;
		int y0 = int(y0f); y0f-=(float)y0;
		int z0 = int(z0f); z0f-=(float)z0;
		if (x0>=index_x-2) {x0=index_x-2;}
		if (y0>=index_y-2) {y0=index_y-2;}
		if (z0>=index_z-2) {z0=index_z-2;}
		if (x0<=0) {x0=0;}
		if (y0<=0) {y0=0;}
		if (z0<=0) {z0=0;}
		int x1=x0+1;
		int y1=y0+1;
		int z1=z0+1;



		float3 d000=gridFluence[xyz2i(x0,y0,z0)];
		float3 d100=gridFluence[xyz2i(x0,y0,z1)];
		float3 d010=gridFluence[xyz2i(x0,y1,z0)];
		float3 d110=gridFluence[xyz2i(x0,y1,z1)];
		float3 d001=gridFluence[xyz2i(x1,y0,z0)];
		float3 d101=gridFluence[xyz2i(x1,y0,z1)];
		float3 d011=gridFluence[xyz2i(x1,y1,z0)];
		float3 d111=gridFluence[xyz2i(x1,y1,z1)];
		if(isFLDSingle>0.5f)
		{
			d000=gridBuffer[xyz2i(x0,y0,z0)];
			d100=gridBuffer[xyz2i(x0,y0,z1)];
			d010=gridBuffer[xyz2i(x0,y1,z0)];
			d110=gridBuffer[xyz2i(x0,y1,z1)];
			d001=gridBuffer[xyz2i(x1,y0,z0)];
			d101=gridBuffer[xyz2i(x1,y0,z1)];
			d011=gridBuffer[xyz2i(x1,y1,z0)];
			d111=gridBuffer[xyz2i(x1,y1,z1)];
		}
		else if(isFLDSingle<-0.5f)
		{
			d000+=gridBuffer[xyz2i(x0,y0,z0)];
			d100+=gridBuffer[xyz2i(x0,y0,z1)];
			d010+=gridBuffer[xyz2i(x0,y1,z0)];
			d110+=gridBuffer[xyz2i(x0,y1,z1)];
			d001+=gridBuffer[xyz2i(x1,y0,z0)];
			d101+=gridBuffer[xyz2i(x1,y0,z1)];
			d011+=gridBuffer[xyz2i(x1,y1,z0)];
			d111+=gridBuffer[xyz2i(x1,y1,z1)];
		}

	
		//p>p0
		float3 w1 = make_float3(x0f, y0f, z0f);
		//w1 = p-(P1-P0)*make_float3(float(x0), float(y0), float(z0))-P0
		float3 w0 = make_float3(1.f,1.f,1.f)-w1;
		//debug.watch(p,x0,y0,z0, w1, w0)
		//return 1.f
		float3 res = (((d000 * w0.x + d001 * w1.x) * w0.y +  (d010 * w0.x + d011 * w1.x) * w1.y) * w0.z +  ((d100 * w0.x + d101 * w1.x) * w0.y + (d110 * w0.x + d111 * w1.x) * w1.y) * w1.z); 
		return res;
	}
		return make_float3(0.f);
}

static __device__ __inline__ void localFrame(float3& T, float3& B, const float3& N2)
{
	float3 N=normalize(N2);
	if (abs(N.x) <= abs(N.y) && abs(N.x) <= abs(N.z))
		T = make_float3(0.f, -N.z, N.y);
	else if (abs(N.y) <= abs(N.x) && abs(N.y) <= abs(N.z))
		T = make_float3(-N.z, 0.f, N.x);
	else
		T = make_float3(-N.y, N.x, 0.f);
	T = normalize(T);
	B = cross(N, T);
	T = cross(B, N);
}

static __device__ __inline__ float3 SampleHG( const float u1, const float u2, const float3& N)
{
	float3 T,B;
	localFrame(T,B,N);
	
	float costheta;
	if (fabs(g) < 0.001f)
		costheta = 1.f - 2.f * u1;
	else 
	{
		float sqrTerm = (1.f - g * g) /(1.f - g + 2.f * g * u1);
		costheta = (1.f + g * g - sqrTerm * sqrTerm) / (2.f * g);
	}
	float sintheta = sqrt(max(0.f, 1.f-costheta*costheta));
	float phi = 2.f * M_PIf * u2;
	float x=cos(phi)*sintheta;
	float y=sin(phi)*sintheta;
	return x*T+y*B+costheta*N;
}

static __device__ __inline__ float3 uniformSphere( const float u1, const float u2, const float3& N)
{
	float3 T,B;
	localFrame(T,B,N);

	float z=1.f-2.f*u1;
	float r=sqrt(max(0.f,1.f-z*z));
	float phi=2.f*M_PIf*u2;
	float x=r*cos(phi);
	float y=r*sin(phi);
	return x*T+y*B+z*N;
}

static __device__ __inline__ float3 cosineHemisphere( const float u1, const float u2, const float3& N)
{
	float3 p;
	cosine_sample_hemisphere(u1, u2, p);
	float3 v1, v2;
	createONB(ray.direction, v1, v2);
	current_prd.direction = v1 * p.x + v2 * p.y + ray.direction * p.z;
	return v1 * p.x + v2 * p.y + ray.direction * p.z;
}


#endif
	
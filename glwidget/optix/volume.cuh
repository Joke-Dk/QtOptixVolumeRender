#include "ray.cuh"

rtBuffer<float> volume_density;
rtDeclareVariable(float3,        P0, , );
rtDeclareVariable(float3,        P1, , );
rtDeclareVariable(int,        index_x, , );
rtDeclareVariable(int,        index_y, , );
rtDeclareVariable(int,        index_z, , );
rtDeclareVariable(float,        sigma_t, , );
rtDeclareVariable(float,        alpha_value, , );


static __device__ __inline__ int xyz2i(int x, int y, int z)
{
	return z*index_x*index_y+ y*index_x+x;
}

static __device__ __inline__ float get_density(float3 p)
{
	//return 1.f
	//pbrt
	//p = make_float3(p2.x, p2.z, p2.y)
	if (((p.x-P0.x)*(p.x-P1.x)<0.f) && ((p.y-P0.y)*(p.y-P1.y)<0.f) &&((p.z-P0.z)*(p.z-P1.z)<0.f))
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
		return  res;//CloudExpCurve(res);//*.densityMultiplier//*CloudExpCurve(res)
	}
	else 
	{
		return 0.0000f;
	}
}

static __device__  __inline__ float CloudExpCurve(float v)
{
	float CloudCover = 0.1f;
	float CloudSharpness = 0.1f;
	float c=v-CloudCover;
	if (c<0.f){c=0.f;}
	float cloudDensity = 1.f-pow(CloudSharpness, c);
	return cloudDensity;
}


	
#pragma once
#define NOMINMAX
#include <optixu/optixu_math_namespace.h>
#include <string>


struct ParallelogramLight
{
	optix::float3 corner;
	optix::float3 v1, v2;
	optix::float3 normal;
	optix::float3 emission;
	bool textured;
};

static __inline__ std::string my_ptxpath( const std::string& base )
{
	//cout<<std::string("load  ./Debug/ptx/ray_tracer_optix_" + base + ".ptx")<<endl;
	return std::string("./Debug/ptx/glwidget_" + base + ".ptx");
}

static __device__ __inline__ void mapToDisk( optix::float2& sample )
{
	float phi, r;
	float a = 2.0f * sample.x - 1.0f;      // (a,b) is now on [-1,1]^2 
	float b = 2.0f * sample.y - 1.0f;      // 
	if (a > -b) {                           // reg 1 or 2 
		if (a > b) {                          // reg 1, also |a| > |b| 
			r = a;
			phi = (M_PIf/4.0f) * (b/a);
		} else {                              // reg 2, also |b| > |a| 
			r = b;
			phi = (M_PIf/4.0f) * (2.0f - a/b);
		}
	} else {                                // reg 3 or 4 
		if (a < b) {                          // reg 3, also |a|>=|b| && a!=0 
			r = -a;
			phi = (M_PIf/4.0f) * (4.0f + b/a);
		} else {                              // region 4, |b| >= |a|,  but 
			// a==0 and  b==0 could occur. 
			r = -b;
			phi = b != 0.0f ? (M_PIf/4.0f) * (6.0f - a/b) :
				0.0f;
		}
	}
	float u = r * cosf( phi );
	float v = r * sinf( phi );
	sample.x = u;
	sample.y = v;
}


// Uniformly sample the surface of a Parallelogram.  Return probability
// of the given sample
static
__device__ __inline__ void sampleParallelogram( const ParallelogramLight& light, 
											   const optix::float3& hit_point,
											   const optix::float2& sample,
											   optix::float3& w_in,
											   float& dist,
											   float& pdf )
{
	using namespace optix;

	float3 on_light = light.corner + sample.x*light.v1 + sample.y*light.v2;
	w_in = on_light - hit_point;
	float dist2 = dot( w_in, w_in );
	dist  = sqrt( dist2 ); 
	w_in /= dist;

	float3 normal = cross( light.v1, light.v2 );
	float area    = length( normal );
	normal /= area;
	float cosine  = -dot( normal, w_in );
	pdf = dist2 / ( area * cosine );
}


// Create ONB from normal.  Resulting W is Parallel to normal
static
__device__ __inline__ void createONB( const optix::float3& n,
									 optix::float3& U,
									 optix::float3& V,
									 optix::float3& W )
{
	using namespace optix;

	W = normalize( n );
	U = cross( W, make_float3( 0.0f, 1.0f, 0.0f ) );
	if ( fabsf( U.x) < 0.001f && fabsf( U.y ) < 0.001f && fabsf( U.z ) < 0.001f  )
		U = cross( W, make_float3( 1.0f, 0.0f, 0.0f ) );
	U = normalize( U );
	V = cross( W, U );
}

// Create ONB from normalalized vector
static
__device__ __inline__ void createONB( const optix::float3& n,
									 optix::float3& U,
									 optix::float3& V)
{
	using namespace optix;

	U = cross( n, make_float3( 0.0f, 1.0f, 0.0f ) );
	if ( dot(U, U) < 1.e-3f )
		U = cross( n, make_float3( 1.0f, 0.0f, 0.0f ) );
	U = normalize( U );
	V = cross( n, U );
}

// sample hemisphere with cosine density
static
__device__ __inline__ void sampleUnitHemisphere( const optix::float2& sample,
												const optix::float3& U,
												const optix::float3& V,
												const optix::float3& W,
												optix::float3& point )
{
	using namespace optix;

	float phi = 2.0f * M_PIf*sample.x;
	float r = sqrt( sample.y );
	float x = r * cos(phi);
	float y = r * sin(phi);
	float z = 1.0f - x*x -y*y;
	z = z > 0.0f ? sqrt(z) : 0.0f;

	point = x*U + y*V + z*W;
}

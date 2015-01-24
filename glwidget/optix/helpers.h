#pragma once
#define NOMINMAX
#include <optixu/optixu_math_namespace.h>
#include <string>

static __device__ __inline__ optix::float3 powf(optix::float3 a, float exp)
{
	return optix::make_float3(powf(a.x, exp), powf(a.y, exp), powf(a.z, exp));
}

// Convert a float3 in [0,1)^3 to a uchar4 in [0,255]^4 -- 4th channel is set to 255
#ifdef __CUDACC__
static __device__ __inline__ optix::uchar4 make_color(const optix::float3& c)
{
    return optix::make_uchar4( static_cast<unsigned char>(__saturatef(c.z)*255.99f),  /* B */
                               static_cast<unsigned char>(__saturatef(c.y)*255.99f),  /* G */
                               static_cast<unsigned char>(__saturatef(c.x)*255.99f),  /* R */
                               255u);                                                 /* A */
}
#endif

// Sample Phong lobe relative to U, V, W frame
static __host__ __device__ __inline__ optix::float3 sample_phong_lobe( optix::float2 sample, float exponent, 
                                                                optix::float3 U, optix::float3 V, optix::float3 W )
{
  const float power = expf( logf(sample.y)/(exponent+1.0f) );
  const float phi = sample.x * 2.0f * (float)M_PIf;
  const float scale = sqrtf(1.0f - power*power);
  
  const float x = cosf(phi)*scale;
  const float y = sinf(phi)*scale;
  const float z = power;

  return x*U + y*V + z*W;
}

// Sample Phong lobe relative to U, V, W frame
static __host__ __device__ __inline__ optix::float3 sample_phong_lobe( const optix::float2 &sample, float exponent, 
                                                                const optix::float3 &U, const optix::float3 &V, const optix::float3 &W, 
                                                                float &pdf, float &bdf_val )
{
  const float cos_theta = powf(sample.y, 1.0f/(exponent+1.0f) );

  const float phi = sample.x * 2.0f * M_PIf;
  const float sin_theta = sqrtf(1.0f - cos_theta*cos_theta);
  
  const float x = cosf(phi)*sin_theta;
  const float y = sinf(phi)*sin_theta;
  const float z = cos_theta;

  const float powered_cos = powf( cos_theta, exponent );
  pdf = (exponent+1.0f) / (2.0f*M_PIf) * powered_cos;
  bdf_val = (exponent+2.0f) / (2.0f*M_PIf) * powered_cos;  

  return x*U + y*V + z*W;
}

// Get Phong lobe PDF for local frame
static __host__ __device__ __inline__ float get_phong_lobe_pdf( float exponent, const optix::float3 &normal, const optix::float3 &dir_out, 
                                                         const optix::float3 &dir_in, float &bdf_val)
{  
  using namespace optix;

  float3 r = -reflect(dir_out, normal);
  const float cos_theta = fabs(dot(r, dir_in));
  const float powered_cos = powf(cos_theta, exponent );

  bdf_val = (exponent+2.0f) / (2.0f*M_PIf) * powered_cos;  
  return (exponent+1.0f) / (2.0f*M_PIf) * powered_cos;
}

// Create ONB from normal.  Resulting W is parallel to normal
static __host__ __device__ __inline__ void create_onb( const optix::float3& n, optix::float3& U, optix::float3& V, optix::float3& W )
{
  using namespace optix;

  W = normalize( n );
  U = cross( W, optix::make_float3( 0.0f, 1.0f, 0.0f ) );

  if ( fabs( U.x ) < 0.001f && fabs( U.y ) < 0.001f && fabs( U.z ) < 0.001f  )
    U = cross( W, make_float3( 1.0f, 0.0f, 0.0f ) );

  U = normalize( U );
  V = cross( W, U );
}

// Create ONB from normalized vector
static __device__ __inline__ void create_onb( const optix::float3& n, optix::float3& U, optix::float3& V)
{
  using namespace optix;

  U = cross( n, make_float3( 0.0f, 1.0f, 0.0f ) );

  if ( dot( U, U ) < 1e-3f )
    U = cross( n, make_float3( 1.0f, 0.0f, 0.0f ) );

  U = normalize( U );
  V = cross( n, U );
}

// Compute the origin ray differential for transfer
static __host__ __device__ __inline__ optix::float3 differential_transfer_origin(optix::float3 dPdx, optix::float3 dDdx, float t, optix::float3 direction, optix::float3 normal)
{
  float dtdx = -optix::dot((dPdx + t*dDdx), normal)/optix::dot(direction, normal);
  return (dPdx + t*dDdx)+dtdx*direction;
}

// Compute the direction ray differential for a pinhole camera
static __host__ __device__ __inline__ optix::float3 differential_generation_direction(optix::float3 d, optix::float3 basis)
{
  float dd = optix::dot(d,d);
  return (dd*basis-optix::dot(d,basis)*d)/(dd*sqrtf(dd));
}

// Compute the direction ray differential for reflection
static __host__ __device__ __inline__
optix::float3 differential_reflect_direction(optix::float3 dPdx, optix::float3 dDdx, optix::float3 dNdP, 
                                             optix::float3 D, optix::float3 N)
{
  using namespace optix;

  float3 dNdx = dNdP*dPdx;
  float dDNdx = dot(dDdx,N) + dot(D,dNdx);
  return dDdx - 2*(dot(D,N)*dNdx + dDNdx*N);
}

// Compute the direction ray differential for refraction
static __host__ __device__ __inline__ 
optix::float3 differential_refract_direction(optix::float3 dPdx, optix::float3 dDdx, optix::float3 dNdP, 
                                             optix::float3 D, optix::float3 N, float ior, optix::float3 T)
{
  using namespace optix;

  float eta;
  if(dot(D,N) > 0.f) {
    eta = ior;
    N = -N;
  } else {
    eta = 1.f / ior;
  }

  float3 dNdx = dNdP*dPdx;
  float mu = eta*dot(D,N)-dot(T,N);
  float TN = -sqrtf(1-eta*eta*(1-dot(D,N)*dot(D,N)));
  float dDNdx = dot(dDdx,N) + dot(D,dNdx);
  float dmudx = (eta - (eta*eta*dot(D,N))/TN)*dDNdx;
  return eta*dDdx - (mu*dNdx+dmudx*N);
}

// Color space conversions
static __host__ __device__ __inline__ optix::float3 Yxy2XYZ( const optix::float3& Yxy )
{
  return optix::make_float3(  Yxy.y * ( Yxy.x / Yxy.z ),
                              Yxy.x,
                              ( 1.0f - Yxy.y - Yxy.z ) * ( Yxy.x / Yxy.z ) );
}

static __host__ __device__ __inline__ optix::float3 XYZ2rgb( const optix::float3& xyz)
{
  const float R = optix::dot( xyz, optix::make_float3(  3.2410f, -1.5374f, -0.4986f ) );
  const float G = optix::dot( xyz, optix::make_float3( -0.9692f,  1.8760f,  0.0416f ) );
  const float B = optix::dot( xyz, optix::make_float3(  0.0556f, -0.2040f,  1.0570f ) );
  return optix::make_float3( R, G, B );
}

static __host__ __device__ __inline__ optix::float3 Yxy2rgb( optix::float3 Yxy )
{
  using namespace optix;

  // First convert to xyz
  float3 xyz = make_float3( Yxy.y * ( Yxy.x / Yxy.z ),
                            Yxy.x,
                            ( 1.0f - Yxy.y - Yxy.z ) * ( Yxy.x / Yxy.z ) );

  const float R = dot( xyz, make_float3(  3.2410f, -1.5374f, -0.4986f ) );
  const float G = dot( xyz, make_float3( -0.9692f,  1.8760f,  0.0416f ) );
  const float B = dot( xyz, make_float3(  0.0556f, -0.2040f,  1.0570f ) );
  return make_float3( R, G, B );
}

static __host__ __device__ __inline__ optix::float3 rgb2Yxy( optix::float3 rgb)
{
  using namespace optix;

  // convert to xyz
  const float X = dot( rgb, make_float3( 0.4124f, 0.3576f, 0.1805f ) );
  const float Y = dot( rgb, make_float3( 0.2126f, 0.7152f, 0.0722f ) );
  const float Z = dot( rgb, make_float3( 0.0193f, 0.1192f, 0.9505f ) );

  // convert xyz to Yxy
  return make_float3( Y, 
                      X / ( X + Y + Z ),
                      Y / ( X + Y + Z ) );
}

static __host__ __device__ __inline__ optix::float3 tonemap( const optix::float3 &hdr_value, float Y_log_av, float Y_max)
{
  using namespace optix;

  float3 val_Yxy = rgb2Yxy( hdr_value );
  
  float Y        = val_Yxy.x; // Y channel is luminance
  const float a = 0.04f;
  float Y_rel = a * Y / Y_log_av;
  float mapped_Y = Y_rel * (1.0f + Y_rel / (Y_max * Y_max)) / (1.0f + Y_rel);

  float3 mapped_Yxy = make_float3( mapped_Y, val_Yxy.y, val_Yxy.z ); 
  float3 mapped_rgb = Yxy2rgb( mapped_Yxy ); 

  return mapped_rgb;
}

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
	return std::string("Debug/ptx/glwidget_" + base + ".ptx");
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


template<unsigned int N>
static __host__ __device__ __inline__ unsigned int tea( unsigned int val0, unsigned int val1 )
{
	unsigned int v0 = val0;
	unsigned int v1 = val1;
	unsigned int s0 = 0;

	for( unsigned int n = 0; n < N; n++ )
	{
		s0 += 0x9e3779b9;
		v0 += ((v1<<4)+0xa341316c)^(v1+s0)^((v1>>5)+0xc8013ea4);
		v1 += ((v0<<4)+0xad90777d)^(v0+s0)^((v0>>5)+0x7e95761e);
	}

	return v0;
}

// Generate random unsigned int in [0, 2^24)
static __host__ __device__ __inline__ unsigned int lcg(unsigned int &prev)
{
	const unsigned int LCG_A = 1664525u;
	const unsigned int LCG_C = 1013904223u;
	prev = (LCG_A * prev + LCG_C);
	return prev & 0x00FFFFFF;
}

static __host__ __device__ __inline__ unsigned int lcg2(unsigned int &prev)
{
	prev = (prev*8121 + 28411)  % 134456;
	return prev;
}

// Generate random float in [0, 1)
static __host__ __device__ __inline__ float rnd(unsigned int &prev)
{
	return ((float) lcg(prev) / (float) 0x01000000);
}

// Multiply with carry
static __host__ __inline__ unsigned int mwc()
{
	static unsigned long long r[4];
	static unsigned long long carry;
	static bool init = false;
	if( !init ) {
		init = true;
		unsigned int seed = 7654321u, seed0, seed1, seed2, seed3;
		r[0] = seed0 = lcg2(seed);
		r[1] = seed1 = lcg2(seed0);
		r[2] = seed2 = lcg2(seed1);
		r[3] = seed3 = lcg2(seed2);
		carry = lcg2(seed3);
	}

	unsigned long long sum = 2111111111ull * r[3] +
		1492ull       * r[2] +
		1776ull       * r[1] +
		5115ull       * r[0] +
		1ull          * carry;
	r[3]   = r[2];
	r[2]   = r[1];
	r[1]   = r[0];
	r[0]   = static_cast<unsigned int>(sum);        // lower half
	carry  = static_cast<unsigned int>(sum >> 32);  // upper half
	return static_cast<unsigned int>(r[0]);
}

static __host__ __inline__ unsigned int random1u()
{
#if 0
	return rand();
#else
	return mwc();
#endif
}

static __host__ __inline__ optix::uint2 random2u()
{
	return optix::make_uint2(random1u(), random1u());
}

static __host__ __inline__ void fillRandBuffer( unsigned int *seeds, unsigned int N )
{
	for( unsigned int i=0; i<N; ++i ) 
		seeds[i] = mwc();
}

static __host__ __device__ __inline__ unsigned int rot_seed( unsigned int seed, unsigned int frame )
{
	return seed ^ frame;
}

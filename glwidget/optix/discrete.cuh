
// reference implementation
// http://en.cppreference.com/w/cpp/algorithm/lower_bound
// http://en.cppreference.com/w/cpp/algorithm/upper_bound

// first ele not less than key

const float M_TWOPI = 2.f*M_PIf;
const float INV_TWOPI = 1.f/2.f/M_PIf;
const float INV_PI = 1.f/M_PIf;

static __device__ __inline__ uint LOWER_BOUND_FUNC(int size,float key,uint v=0)
{
	int first = 0, it, count = size, step;

	while(count > 0)
	{
		step = count >> 1;
		it = first + step;
		if(ARRAY_GET(it) < key)
		{     
			first = ++it;          
			count -= step + 1;
		}
		else
		{
			count = step;
		}
	}
	return first;
}

// first ele larger than key
static __device__ __inline__ uint UPPER_BOUND_FUNC(uint size, float key,uint v=0)
{
	int first = 0, it, count = size, step;

	while(count > 0)
	{
		step = count >> 1;
		it = first + step;
		if(ARRAY_GET(it) > key)
		{
			count = step;
		}
		else{
			first = ++it;
			count -= step + 1;
		}
	}
	return first;
}

static __device__ __inline__ float sphericalTheta(const float3 &v) // 0 pi
{
	return acosf(clamp(v.z, -1.f, 1.f));
}
static __device__ __inline__ float sphericalPhi(const float3 &v) // 0 2pi
{
	float p = atan2f(v.y, v.x);
	return (p < 0.f) ? p + M_TWOPI : p;
}

static __device__ __inline__ float3 spherical2cartesian(float theta, float phi) 
{
	return optix::make_float3(sinf(theta) * cosf(phi),
		sinf(theta) * sinf(phi),
		cosf(theta));
}
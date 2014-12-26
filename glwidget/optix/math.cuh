#ifndef _MATH_CUH_
#define _MATH_CUH_

static __device__ inline float3 powf(float3 a, float exp)
{
  return make_float3(powf(a.x, exp), powf(a.y, exp), powf(a.z, exp));
}


#endif
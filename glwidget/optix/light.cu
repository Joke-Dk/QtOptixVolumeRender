#include "ray.cuh"

rtDeclareVariable(float3,        emission_color, , );

RT_PROGRAM void diffuseEmitter()
{
  current_prd.radiance = current_prd.countEmitted? emission_color : make_float3(1.f);
  current_prd.done = true;
}

RT_PROGRAM void diffuseEmitterShadow()
{
	//rtTerminateRay();
}
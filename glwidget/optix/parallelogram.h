#pragma once
#include "shape.h"

class Parallelogram: public Shape
{
public:
	Parallelogram(optix::Context& optixCtx, const optix::float3& anchor,
		const optix::float3& offset1,
		const optix::float3& offset2);
protected:
	void setup( optix::Context& optixCtx);
	static bool isSetuped;
	static optix::Program _ProgramBoundingBox;
	static optix::Program _ProgramIntersection;
	bool isGroups(){ return 0;}
};

optix::GeometryInstance createParallelogram( optix::Context& optixCtx, const optix::float3& anchor,
											const optix::float3& offset1,
											const optix::float3& offset2);
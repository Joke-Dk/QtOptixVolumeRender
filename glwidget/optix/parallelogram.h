#pragma once
#include "shape.h"

class Parallelogram: public Shape
{
public:
	Parallelogram(optix::Context& optixCtx, const optix::float3& anchor,
		const optix::float3& offset1,
		const optix::float3& offset2);
	static void setup( optix::Context& optixCtx);
	static bool isSetuped;
	static optix::Program p_ProgramBoundingBox;
	static optix::Program p_ProgramIntersection;
};


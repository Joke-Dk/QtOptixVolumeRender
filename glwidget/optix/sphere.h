#pragma once
#include "shape.h"

class Sphere: public Shape
{
public:
	Sphere(optix::Context& optixCtx, const optix::float3& p, float r);
protected:
	void setup( optix::Context& optixCtx);
	static bool isSetuped;
	static optix::Program _ProgramBoundingBox;
	static optix::Program _ProgramIntersection;
	bool isGroups(){ return 0;}
};


optix::GeometryInstance createSphere( optix::Context& optixCtx, const optix::float3& p, float r);
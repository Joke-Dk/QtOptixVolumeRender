#pragma once
#include "shape.h"

class TriangleMesh: public Shape
{
public:
	TriangleMesh(optix::Context& optixCtx, const std::string& path, const optix::Matrix4x4 m0, const optix::Material& material0);
protected:
	void setup( optix::Context& optixCtx);
	static bool isSetuped;
	static optix::Program _ProgramBoundingBox;
	static optix::Program _ProgramIntersection;
	bool isGroups(){ return 1;}
};


optix::GeometryInstance createObjloader( optix::Context& optixCtx, const std::string& path, const optix::Matrix4x4 m0, const optix::Material& material0);
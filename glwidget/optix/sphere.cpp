#include "sphere.h"

optix::Program Sphere::_ProgramBoundingBox;
optix::Program Sphere::_ProgramIntersection;
bool Sphere::isSetuped = false;

void Sphere::setup( optix::Context& optixCtx)
{
	if (!isSetuped)
	{
		_ProgramBoundingBox = optixCtx->createProgramFromPTXFile( my_ptxpath( "sphere.cu" ), "bounds" );
		_ProgramIntersection = optixCtx->createProgramFromPTXFile( my_ptxpath( "sphere.cu" ), "robust_intersect" );
		isSetuped = true;
	}
}

Sphere::Sphere(optix::Context& optixCtx, const optix::float3& p, float r)
{
	init( optixCtx);
	_geometry->setIntersectionProgram( _ProgramIntersection );
	_geometry->setBoundingBoxProgram( _ProgramBoundingBox );
	_geometry["sphere"]->setFloat( p.x, p.y, p.z, r );
	setGeometryInstance();
}

optix::GeometryInstance createSphere( optix::Context& optixCtx, const optix::float3& p, float r)
{
	return Sphere( optixCtx, p, r).getGeometryInstance();
}
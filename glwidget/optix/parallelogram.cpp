#include "parallelogram.h"

optix::Program Parallelogram::_ProgramBoundingBox;
optix::Program Parallelogram::_ProgramIntersection;
bool Parallelogram::isSetuped = false;

void Parallelogram::setup( optix::Context& optixCtx)
{
	if (!isSetuped)
	{
		_ProgramBoundingBox = optixCtx->createProgramFromPTXFile( my_ptxpath( "parallelogram.cu" ), "bounds" );
		_ProgramIntersection = optixCtx->createProgramFromPTXFile( my_ptxpath( "parallelogram.cu" ), "intersect" );
		isSetuped = true;
	}
}

Parallelogram::Parallelogram( optix::Context& optixCtx, const optix::float3& anchor,
															 const optix::float3& offset1,
															 const optix::float3& offset2)
{
	init( optixCtx);
	_geometry->setIntersectionProgram( _ProgramIntersection );
	_geometry->setBoundingBoxProgram( _ProgramBoundingBox );
	optix::float3 normal = normalize( cross( offset1, offset2 ) );
	float d = dot( normal, anchor );
	optix::float4 plane = make_float4( normal, d );

	optix::float3 v1 = offset1 / dot( offset1, offset1 );
	optix::float3 v2 = offset2 / dot( offset2, offset2 );

	_geometry["plane"]->setFloat( plane );
	_geometry["anchor"]->setFloat( anchor );
	_geometry["v1"]->setFloat( v1 );
	_geometry["v2"]->setFloat( v2 );

	setGeometryInstance();
}


optix::GeometryInstance createParallelogram( optix::Context& optixCtx, const optix::float3& anchor,
															 const optix::float3& offset1,
															 const optix::float3& offset2)
{
	return Parallelogram( optixCtx, anchor, offset1, offset2).getGeometryInstance();
}
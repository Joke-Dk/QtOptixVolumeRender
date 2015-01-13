#include "parallelogram.h"

optix::Program Parallelogram::p_ProgramBoundingBox;
optix::Program Parallelogram::p_ProgramIntersection;
bool Parallelogram::isSetuped = false;

void Parallelogram::setup( optix::Context& optixCtx)
{
	p_ProgramBoundingBox = optixCtx->createProgramFromPTXFile( my_ptxpath( "parallelogram.cu" ), "bounds" );
	p_ProgramIntersection = optixCtx->createProgramFromPTXFile( my_ptxpath( "parallelogram.cu" ), "intersect" );
	isSetuped = true;
}

Parallelogram::Parallelogram( optix::Context& optixCtx, const optix::float3& anchor,
															 const optix::float3& offset1,
															 const optix::float3& offset2):Shape()
{
	if (!isSetuped)
	{
		setup(optixCtx);
	}
	optix::Geometry parallelogramGeometry = optixCtx->createGeometry();
	parallelogramGeometry->setPrimitiveCount( 1u );
	parallelogramGeometry->setIntersectionProgram( p_ProgramIntersection );
	parallelogramGeometry->setBoundingBoxProgram( p_ProgramBoundingBox );

	optix::float3 normal = normalize( cross( offset1, offset2 ) );
	float d = dot( normal, anchor );
	optix::float4 plane = make_float4( normal, d );

	optix::float3 v1 = offset1 / dot( offset1, offset1 );
	optix::float3 v2 = offset2 / dot( offset2, offset2 );

	parallelogramGeometry["plane"]->setFloat( plane );
	parallelogramGeometry["anchor"]->setFloat( anchor );
	parallelogramGeometry["v1"]->setFloat( v1 );
	parallelogramGeometry["v2"]->setFloat( v2 );

	optix::GeometryInstance gi = optixCtx->createGeometryInstance();
	gi->setGeometry(parallelogramGeometry);
	setGeometryInstance( gi);
}
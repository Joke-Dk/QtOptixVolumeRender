#include "trianglemesh.h"

optix::Program TriangleMesh::_ProgramBoundingBox;
optix::Program TriangleMesh::_ProgramIntersection;
bool TriangleMesh::isSetuped = false;

void TriangleMesh::setup( optix::Context& optixCtx)
{
	if (!isSetuped)
	{
		_ProgramIntersection = optixCtx->createProgramFromPTXFile( my_ptxpath( "trianglemesh.cu" ), "mesh_intersect"  );
		_ProgramBoundingBox = optixCtx->createProgramFromPTXFile( my_ptxpath( "trianglemesh.cu" ), "mesh_bounds"  );
		isSetuped = true;
	}
}

TriangleMesh::TriangleMesh(optix::Context& optixCtx, const std::string& path, const optix::Matrix4x4 m0, const optix::Material& material0)
{
	init( optixCtx);
	ObjLoader objloader0( (path).c_str(), optixCtx, _gg, material0);
	objloader0.setIntersectProgram( _ProgramIntersection );
	objloader0.setBboxProgram( _ProgramBoundingBox );
	objloader0.load( m0 );
	_gi = _gg->getChild(0);
}

optix::GeometryInstance createObjloader( optix::Context& optixCtx, const std::string& path, const optix::Matrix4x4 m0, const optix::Material& material0)
{
	return TriangleMesh( optixCtx, path, m0, material0).getGeometryInstance();
}
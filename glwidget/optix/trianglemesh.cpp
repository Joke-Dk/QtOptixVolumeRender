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
	OptixMesh OptixMesh0(  optixCtx, _gg, material0);
	OptixMesh0.setDefaultIntersectionProgram(_ProgramIntersection);
	//OptixMesh0.setBboxProgram( _ProgramBoundingBox );
	OptixMesh0.setLoadingTransform(m0);
	OptixMesh0.loadBegin_Geometry(path);
	OptixMesh0.loadFinish_Materials();
	_gi = _gg->getChild(0);
}

optix::GeometryInstance createOptixMesh( optix::Context& optixCtx, const std::string& path, const optix::Matrix4x4 m0, const optix::Material& material0)
{
	return TriangleMesh( optixCtx, path, m0, material0).getGeometryInstance();
}
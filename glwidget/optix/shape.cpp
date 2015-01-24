#include "shape.h"

void Shape::init( optix::Context& optixCtx)
{

	setup(optixCtx);
	_geometry = optixCtx->createGeometry();
	_geometry->setPrimitiveCount( 1u );
	if ( isGroups())
	{
		_gg = optixCtx->createGeometryGroup();
	}
	_gi = optixCtx->createGeometryInstance();

}

void Shape::setGeometryInstance( )
{
	_gi->setGeometry(_geometry);
}

optix::GeometryInstance Shape::getGeometryInstance()
{
	return _gi;
}

optix::GeometryGroup Shape::getGeometryGroup()
{
	return _gg;
}


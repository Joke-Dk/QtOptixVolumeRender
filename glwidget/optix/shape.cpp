#include "shape.h"

void Shape::setGeometryInstance( optix::GeometryInstance gi)
{
	_gi = gi;
}

optix::GeometryInstance Shape::getGeometryInstance()
{
	return _gi;
}

